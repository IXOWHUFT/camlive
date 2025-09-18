ESP32-CAM + HX711 Auto Photo Project (with Dashboard & Live)
===============================================================

New features added:
- ESP notifies server with weight and stream URL after each upload.
- Dashboard shows weight next to each photo and a Live Camera panel (iframe) if ESP exposes a stream.
- Server keeps a data.json log (latest entries).

Files in project:
- esp32_cam_hx711_auto.ino     -> ESP sketch (uploads photo, notifies server with weight & stream)
- upload.php                   -> receives binary image, saves to uploads/, returns JSON {url,filename}
- notify.php                   -> receives JSON metadata from ESP and stores in data.json
- list_uploads.php             -> returns merged list of uploads including weight/stream info
- dashboard.html               -> web dashboard with gallery, weight and live video panel
- style.css                    -> dashboard styles
- README.txt                   -> this file
- uploads/                     -> folder for images (must be writable by webserver)
- data.json                    -> created/updated by notify.php

How it works (ESP):
1. ESP captures photo and POSTs raw JPEG binary to upload.php.
2. upload.php returns JSON with public URL and filename.
3. ESP then POSTs JSON to notify.php with fields {filename, url, weight, stream, timestamp}.
   - stream is like "http://192.168.1.123:81/stream" (ESP camera server URL).
   - timestamp is epoch seconds (use time()).

How to enable live stream on ESP:
- The sketch includes `startCameraServer()` (standard ESP32-CAM example) which serves:
   - stream at http://<esp_ip>/stream
   - stills via other endpoints
- Ensure ESP is accessible from the machine viewing the dashboard (same LAN or port-forwarding).

Security notes:
- Currently endpoints accept unauthenticated uploads. Add simple token checks or API key in headers/params.
- If exposing stream publicly, secure with firewall/VPN.

Deployment:
1. Upload all files to your PHP-capable server (e.g., /var/www/html/esp_project/).
2. Ensure uploads/ is writable (chown www-data:www-data && chmod 775 uploads).
3. Ensure data.json is writable (create an empty data.json with "[]" or let notify.php create it).
4. Configure ESP: set SERVER_UPLOAD_URL to https://yourdomain.com/esp_project/upload.php and SERVER_NOTIFY_URL to https://yourdomain.com/esp_project/notify.php. ESP will determine its own stream URL and include it in notify.

Notes:
- If ESP can't POST to notify.php due to CORS or TLS issues, you can have your server poll or a cron to match images and metadata.
- For public use consider token-based auth or IP whitelisting.

--- END ---
