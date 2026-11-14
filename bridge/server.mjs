// SmartPlantCare bridge
// Runs on the home server (same LAN as the ESP32) and is exposed to the
// internet through a Cloudflare Tunnel. Visitors can read live status;
// watering and settings require PLANT_PASSWORD from .env.
//
// Zero dependencies. Requires Node 20.12+ (process.loadEnvFile).

import http from "node:http";
import { createHash, timingSafeEqual } from "node:crypto";
import { existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import path from "node:path";

const here = path.dirname(fileURLToPath(import.meta.url));
const envFile = path.join(here, ".env");
if (existsSync(envFile)) process.loadEnvFile(envFile);

const config = {
  host: process.env.HOST || "127.0.0.1",
  port: Number(process.env.PORT || 8787),
  esp32Url: (process.env.ESP32_URL || "").replace(/\/+$/, ""),
  password: process.env.PLANT_PASSWORD || "",
  allowedOrigins: (process.env.ALLOWED_ORIGINS || "")
    .split(",")
    .map((o) => o.trim())
    .filter(Boolean),
  esp32TimeoutMs: Number(process.env.ESP32_TIMEOUT_MS || 3000),
  statusCacheMs: Number(process.env.STATUS_CACHE_MS || 1500),
};

if (!config.esp32Url) {
  console.error("[bridge] ESP32_URL is not set (e.g. http://192.168.1.50). Check .env");
  process.exit(1);
}
if (config.password.length < 8) {
  console.error("[bridge] PLANT_PASSWORD must be at least 8 characters. Check .env");
  process.exit(1);
}

// ---------------------------------------------------------------------------
// Auth + brute-force protection
// ---------------------------------------------------------------------------

const passwordDigest = createHash("sha256").update(config.password).digest();

function passwordMatches(candidate) {
  const digest = createHash("sha256").update(String(candidate)).digest();
  return timingSafeEqual(digest, passwordDigest);
}

const MAX_FAILURES = 5;
const FAILURE_WINDOW_MS = 15 * 60 * 1000;
const failures = new Map(); // ip -> { count, resetAt }

function clientIp(req) {
  // Cloudflare Tunnel sets CF-Connecting-IP to the real visitor address.
  return (
    req.headers["cf-connecting-ip"] ||
    String(req.headers["x-forwarded-for"] || "").split(",")[0].trim() ||
    req.socket.remoteAddress ||
    "unknown"
  );
}

function isBlocked(ip) {
  const entry = failures.get(ip);
  if (!entry) return false;
  if (Date.now() > entry.resetAt) {
    failures.delete(ip);
    return false;
  }
  return entry.count >= MAX_FAILURES;
}

function recordFailure(ip) {
  const now = Date.now();
  const entry = failures.get(ip);
  if (!entry || now > entry.resetAt) {
    failures.set(ip, { count: 1, resetAt: now + FAILURE_WINDOW_MS });
  } else {
    entry.count += 1;
  }
}

/** Returns null when authorised, otherwise [status, body]. */
function checkAuth(req) {
  const ip = clientIp(req);
  if (isBlocked(ip)) {
    return [429, { error: "Too many wrong attempts. Try again later." }];
  }
  const header = String(req.headers.authorization || "");
  const token = header.startsWith("Bearer ") ? header.slice(7) : "";
  if (!token || !passwordMatches(token)) {
    recordFailure(ip);
    return [401, { error: "Wrong password." }];
  }
  failures.delete(ip);
  return null;
}

// ---------------------------------------------------------------------------
// ESP32 access
// ---------------------------------------------------------------------------

async function esp32(pathname, init = {}) {
  const res = await fetch(`${config.esp32Url}${pathname}`, {
    ...init,
    signal: AbortSignal.timeout(config.esp32TimeoutMs),
  });
  let body = null;
  try {
    body = await res.json();
  } catch {
    body = null;
  }
  return { status: res.status, body };
}

// The ESP32 WebServer is single-threaded, so many visitors polling at once
// share one cached reading instead of each hitting the board.
let lastStatus = null; // { online, data, fetchedAt }
let lastData = null; // last good reading, kept while the ESP32 is offline
let lastOnlineAt = null;
let inflight = null;

async function getStatus() {
  if (lastStatus && Date.now() - lastStatus.fetchedAt < config.statusCacheMs) {
    return lastStatus;
  }
  if (!inflight) {
    inflight = (async () => {
      try {
        const { status, body } = await esp32("/api/status");
        if (status !== 200 || !body) throw new Error(`ESP32 responded ${status}`);
        lastOnlineAt = Date.now();
        lastData = body;
        lastStatus = { online: true, data: body, fetchedAt: Date.now() };
      } catch (err) {
        lastStatus = {
          online: false,
          data: lastData,
          fetchedAt: Date.now(),
          error: err instanceof Error ? err.message : String(err),
        };
      } finally {
        inflight = null;
      }
      return lastStatus;
    })();
  }
  return inflight;
}

function invalidateStatus() {
  lastStatus = null;
}

// ---------------------------------------------------------------------------
// HTTP plumbing
// ---------------------------------------------------------------------------

function applyCors(req, res) {
  const origin = req.headers.origin;
  if (origin && config.allowedOrigins.includes(origin)) {
    res.setHeader("Access-Control-Allow-Origin", origin);
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
    res.setHeader("Access-Control-Max-Age", "600");
  }
  res.setHeader("Vary", "Origin");
}

function send(res, status, body) {
  res.writeHead(status, {
    "Content-Type": "application/json; charset=utf-8",
    "Cache-Control": "no-store",
    "X-Content-Type-Options": "nosniff",
  });
  res.end(JSON.stringify(body));
}

function readJson(req, limit = 1024) {
  return new Promise((resolve, reject) => {
    let size = 0;
    const chunks = [];
    req.on("data", (chunk) => {
      size += chunk.length;
      if (size > limit) {
        reject(new Error("Body too large"));
        req.destroy();
        return;
      }
      chunks.push(chunk);
    });
    req.on("end", () => {
      if (!chunks.length) return resolve({});
      try {
        resolve(JSON.parse(Buffer.concat(chunks).toString("utf8")));
      } catch {
        reject(new Error("Invalid JSON"));
      }
    });
    req.on("error", reject);
  });
}

function routeOf(url) {
  // Works both on its own subdomain (/status) and behind a path prefix
  // such as nginx `location /api/plant/` (/api/plant/status).
  const { pathname } = new URL(url, "http://bridge");
  return pathname.replace(/^\/api\/plant(?=\/|$)/, "").replace(/\/+$/, "") || "/";
}

// ---------------------------------------------------------------------------
// Routes
// ---------------------------------------------------------------------------

async function handle(req, res) {
  applyCors(req, res);

  if (req.method === "OPTIONS") {
    res.writeHead(204);
    res.end();
    return;
  }

  const route = routeOf(req.url || "/");

  if (req.method === "GET" && (route === "/" || route === "/health")) {
    return send(res, 200, { ok: true });
  }

  if (req.method === "GET" && route === "/status") {
    const status = await getStatus();
    return send(res, 200, {
      online: status.online,
      lastSeen: lastOnlineAt ? new Date(lastOnlineAt).toISOString() : null,
      data: status.data,
    });
  }

  if (req.method === "POST" && route === "/auth") {
    const denied = checkAuth(req);
    if (denied) return send(res, ...denied);
    return send(res, 200, { ok: true });
  }

  if (req.method === "POST" && route === "/water") {
    const denied = checkAuth(req);
    if (denied) return send(res, ...denied);
    try {
      const { status, body } = await esp32("/api/water", { method: "POST" });
      invalidateStatus();
      return send(res, status, body ?? { success: status === 200 });
    } catch {
      return send(res, 502, { success: false, message: "Plant controller is offline." });
    }
  }

  if (req.method === "POST" && route === "/settings") {
    const denied = checkAuth(req);
    if (denied) return send(res, ...denied);

    let input;
    try {
      input = await readJson(req);
    } catch (err) {
      return send(res, 400, { error: err.message });
    }
    if (!input || typeof input !== "object" || Array.isArray(input)) {
      return send(res, 400, { error: "Expected a JSON object" });
    }

    const params = new URLSearchParams();
    if (input.threshold !== undefined) {
      const t = Number(input.threshold);
      if (!Number.isInteger(t) || t < 5 || t > 95) {
        return send(res, 400, { error: "threshold must be an integer between 5 and 95" });
      }
      params.set("threshold", String(t));
    }
    if (input.autoWatering !== undefined) {
      if (typeof input.autoWatering !== "boolean") {
        return send(res, 400, { error: "autoWatering must be a boolean" });
      }
      params.set("auto", input.autoWatering ? "1" : "0");
    }
    if (input.pumpDurationMs !== undefined) {
      // Must match PumpController::MIN_RUN_MS / MAX_RUN_MS in the firmware.
      const ms = Number(input.pumpDurationMs);
      if (!Number.isInteger(ms) || ms < 1000 || ms > 10000) {
        return send(res, 400, { error: "pumpDurationMs must be an integer between 1000 and 10000" });
      }
      params.set("duration", String(ms));
    }
    if (![...params.keys()].length) {
      return send(res, 400, { error: "Nothing to update" });
    }

    try {
      const { status, body } = await esp32(`/api/settings?${params}`, { method: "POST" });
      invalidateStatus();
      return send(res, status, body ?? { success: status === 200 });
    } catch {
      return send(res, 502, { success: false, message: "Plant controller is offline." });
    }
  }

  return send(res, 404, { error: "Not found" });
}

const server = http.createServer((req, res) => {
  handle(req, res).catch((err) => {
    console.error("[bridge] unhandled", err);
    if (!res.headersSent) send(res, 500, { error: "Internal error" });
  });
});

server.listen(config.port, config.host, () => {
  console.log(`[bridge] listening on http://${config.host}:${config.port} -> ${config.esp32Url}`);
  console.log(`[bridge] allowed origins: ${config.allowedOrigins.join(", ") || "(none)"}`);
});
