/******************************************************
 * LED Billboard (ESP8266 D1 mini + MAX7219 FC-16)
 *
 * Minimal web UI for changing text, brightness, speed,
 * and scroll effect on a 4-module 32x8 LED matrix.
 ******************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>

// --------- MAX7219 / Parola configuration ---------
#define HARDWARE_TYPE  MD_MAX72XX::FC16_HW
#define MAX_DEVICES    4           // 4 x 8x8 = 32x8
// ESP8266 hardware SPI pins: MOSI=D7, SCK=D5
#define CS_PIN         D6          // Chip Select

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// --------- WiFi configuration ---------
// Leave these blank to use fallback AP mode.
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

const char* AP_SSID = "HD-Billboard";
const char* AP_PASS = "change-me"; // Must be at least 8 characters.

// --------- Web server ---------
ESP8266WebServer server(80);

// --------- Display parameters ---------
String g_text = "Hello from ESP8266!";
uint8_t g_brightness = 8;       // 0..15
uint8_t g_speed = 80;           // 10..200 (Parola speed)
textEffect_t g_effect = PA_SCROLL_LEFT;

// --------- Minimal web UI ---------
const char INDEX_MIN_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Billboard Control</title>
<style>
:root{--bg:#111;--fg:#fff;--muted:#bbb}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:16px/1.5 system-ui,Segoe UI,Roboto,Arial}
.wrap{max-width:720px;margin:0 auto;padding:18px}
h1{font-size:20px;margin:0 0 12px}
.row{display:flex;gap:8px;flex-wrap:wrap;margin:8px 0}
input[type=text]{flex:1;min-width:220px;padding:10px;border:1px solid #444;background:#000;color:#fff}
input[type=number]{width:120px;padding:10px;border:1px solid #444;background:#000;color:#fff}
select,button{padding:10px;border:1px solid #444;background:#222;color:#fff;cursor:pointer}
small{color:var(--muted)} .ok{color:#9f9}.err{color:#f99}
label{display:inline-block;min-width:130px}
</style></head><body>
<div class="wrap">
  <h1>LED Billboard</h1>
  <div class="row">
    <input id="text" type="text" placeholder="Display text" maxlength="100">
    <button onclick="send('/setText?text='+encodeURIComponent($('text').value))">Send text</button>
  </div>
  <div class="row">
    <label>Brightness 0-15</label>
    <input id="bri" type="number" min="0" max="15" value="8">
    <button onclick="send('/setBrightness?val='+$('bri').value)">Set</button>
  </div>
  <div class="row">
    <label>Speed 10-200</label>
    <input id="spd" type="number" min="10" max="200" value="80">
    <button onclick="send('/setSpeed?val='+$('spd').value)">Set</button>
  </div>
  <div class="row">
    <select id="eff">
      <option>PA_SCROLL_LEFT</option><option>PA_SCROLL_RIGHT</option>
      <option>PA_OPENING</option><option>PA_CLOSING</option>
      <option>PA_WIPE</option><option>PA_FADE</option>
      <option>PA_GROW_UP</option><option>PA_GROW_DOWN</option>
    </select>
    <button onclick="send('/setEffect?val='+$('eff').value)">Effect</button>
  </div>
  <div id="msg"><small>Ready.</small></div>
</div>
<script>
const $=id=>document.getElementById(id);
async function send(path){
  try{
    const r=await fetch(path,{cache:'no-store'});
    const t=await r.text();
    $('msg').innerHTML='<span class="'+(r.ok?'ok':'err')+'">'+t+'</span>';
  }catch(e){
    $('msg').innerHTML='<span class="err">Error: '+e+'</span>';
  }
}
</script>
</body></html>
)HTML";

textEffect_t parseEffect(const String& s)
{
  if (s == "PA_SCROLL_LEFT")  return PA_SCROLL_LEFT;
  if (s == "PA_SCROLL_RIGHT") return PA_SCROLL_RIGHT;
  if (s == "PA_OPENING")      return PA_OPENING;
  if (s == "PA_CLOSING")      return PA_CLOSING;
  if (s == "PA_WIPE")         return PA_WIPE;
  if (s == "PA_FADE")         return PA_FADE;
  if (s == "PA_GROW_UP")      return PA_GROW_UP;
  if (s == "PA_GROW_DOWN")    return PA_GROW_DOWN;
  return PA_SCROLL_LEFT;
}

void handleRoot()
{
  server.send_P(200, "text/html; charset=utf-8", INDEX_MIN_HTML);
}

void handleSetText()
{
  if (!server.hasArg("text")) { server.send(400, "text/plain", "ERR: text missing"); return; }
  String t = server.arg("text");
  if (t.length() > 100) { server.send(413, "text/plain", "ERR: max 100 chars"); return; }
  g_text = t;
  P.displayClear();
  P.displayReset();
  server.send(200, "text/plain", "OK");
}

void handleSetBrightness()
{
  if (!server.hasArg("val")) { server.send(400, "text/plain", "ERR: val missing"); return; }
  int v = server.arg("val").toInt();
  v = constrain(v, 0, 15);
  g_brightness = (uint8_t)v;
  P.setIntensity(g_brightness);
  server.send(200, "text/plain", "OK");
}

void handleSetSpeed()
{
  if (!server.hasArg("val")) { server.send(400, "text/plain", "ERR: val missing"); return; }
  int v = server.arg("val").toInt();
  v = constrain(v, 10, 200);
  g_speed = (uint8_t)v;
  P.setSpeed(g_speed);
  server.send(200, "text/plain", "OK");
}

void handleSetEffect()
{
  if (!server.hasArg("val")) { server.send(400, "text/plain", "ERR: val missing"); return; }
  g_effect = parseEffect(server.arg("val"));
  P.displayClear();
  P.setTextEffect(g_effect, g_effect);
  P.displayReset();
  server.send(200, "text/plain", "OK");
}

void startWebServer()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setText", HTTP_GET, handleSetText);
  server.on("/setBrightness", HTTP_GET, handleSetBrightness);
  server.on("/setSpeed", HTTP_GET, handleSetSpeed);
  server.on("/setEffect", HTTP_GET, handleSetEffect);
  server.onNotFound([](){ server.send(404, "text/plain", "Not Found"); });
  server.begin();
  Serial.println(F("HTTP server started"));
}

void startAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();
  Serial.print(F("AP started: ")); Serial.print(AP_SSID);
  Serial.print(F("  PASS: ")); Serial.println(AP_PASS);
  Serial.print(F("AP IP: ")); Serial.println(ip);
}

bool tryConnectSTA(const char* ssid, const char* pass, uint32_t timeoutMs = 12000)
{
  if (ssid == nullptr || ssid[0] == '\0') return false;
  Serial.printf("Connecting to WiFi \"%s\" ...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("Connected, IP: ")); Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println(F("WiFi connection failed."));
  return false;
}

void setup()
{
  Serial.begin(115200);
  delay(200);

  P.begin();
  P.setIntensity(g_brightness);
  P.setSpeed(g_speed);
  P.setPause(0);
  P.setTextAlignment(PA_LEFT);
  P.setTextEffect(g_effect, g_effect);
  P.displayText((char*)g_text.c_str(), PA_LEFT, g_speed, 0, g_effect, g_effect);

  if (!tryConnectSTA(WIFI_SSID, WIFI_PASSWORD)) {
    startAP();
  }

  startWebServer();

  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Open the displayed IP address in a browser."));
  } else {
    Serial.println(F("Connect to WiFi 'HD-Billboard' with password 'change-me'."));
    Serial.println(F("Then open http://192.168.4.1/ in a browser."));
  }
}

void loop()
{
  if (P.displayAnimate()) {
    P.displayText((char*)g_text.c_str(), PA_LEFT, g_speed, 0, g_effect, g_effect);
    P.displayReset();
  }

  server.handleClient();
}
