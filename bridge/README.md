# SmartPlantCare bridge

Lets the portfolio page (`https://aniqbinlutpi.my/projects/smart-plant`) read
and control the ESP32 from anywhere, without opening any port on the router.

```
Browser ──HTTPS──▶ Cloudflare ──tunnel──▶ home server :8787 (this bridge) ──LAN HTTP──▶ ESP32
```

- **Anyone** can read live status (`GET /status`).
- **Watering and settings** need `PLANT_PASSWORD` from `.env` (sent as `Authorization: Bearer …`).
- 5 wrong passwords from one IP → blocked for 15 minutes (uses Cloudflare's `CF-Connecting-IP`).
- Status is cached for 1.5s so many visitors don't overload the ESP32's single-threaded web server.
- If the ESP32 goes offline, the page shows the last reading plus "last seen".

The firmware still enforces its own limits: each burst is 1–10s (2.5s by default, adjustable from the
page) followed by a 15s cooldown. The 85% anti-rot lock only blocks **auto**-watering; a manual
"Water now" is only limited by the cooldown, so keep the password private.

Settings changed from the page (threshold, auto, burst length) reset to the `Config.h` defaults
when the ESP32 reboots.

## API

| Method | Path | Auth | Body |
| --- | --- | --- | --- |
| GET | `/status` | – | – |
| POST | `/auth` | ✓ | – (just checks the password) |
| POST | `/water` | ✓ | – |
| POST | `/settings` | ✓ | `{ "threshold"?: 5–95, "autoWatering"?: boolean, "pumpDurationMs"?: 1000–10000 }` |

Paths also work with an `/api/plant` prefix, in case you prefer to route it through nginx instead of a subdomain.

## Setup on the home server

Needs Node 20.12+. No `npm install` needed: it has zero dependencies.

1. **Give the ESP32 a fixed IP.** Add a DHCP reservation in the router for the ESP32's MAC.
   The IP is shown on the OLED / Serial Monitor at boot.

2. **Copy and configure:**
   ```bash
   sudo mkdir -p /opt/SmartPlantCare && sudo chown $USER /opt/SmartPlantCare
   cp -r bridge /opt/SmartPlantCare/
   cd /opt/SmartPlantCare/bridge
   cp .env.example .env
   chmod 600 .env
   nano .env   # set ESP32_URL and a strong PLANT_PASSWORD
   ```

3. **Quick test:**
   ```bash
   node server.mjs
   curl http://127.0.0.1:8787/status
   ```

4. **Run it as a service.** Edit `User=` and the paths in `deploy/plant-bridge.service`, then:
   ```bash
   sudo cp deploy/plant-bridge.service /etc/systemd/system/
   sudo systemctl daemon-reload
   sudo systemctl enable --now plant-bridge
   journalctl -u plant-bridge -f
   ```

5. **Expose it through your existing Cloudflare Tunnel.** Add the ingress rule from
   `deploy/cloudflared-config.example.yml` (above the catch-all `http_status:404`), then:
   ```bash
   cloudflared tunnel route dns <TUNNEL_NAME> plant-api.aniqbinlutpi.my
   sudo systemctl restart cloudflared
   curl https://plant-api.aniqbinlutpi.my/status
   ```
   If you manage the tunnel from the Cloudflare dashboard instead, add a **Public Hostname**
   `plant-api.aniqbinlutpi.my` → `http://127.0.0.1:8787`.

The portfolio calls `https://plant-api.aniqbinlutpi.my` by default. To use a different URL,
build the portfolio with `NEXT_PUBLIC_PLANT_API_URL=https://…` (this value is public, not a secret).

## Changing the password

Edit `PLANT_PASSWORD` in `.env`, then `sudo systemctl restart plant-bridge`.
Anyone who was unlocked gets locked out on their next action.
