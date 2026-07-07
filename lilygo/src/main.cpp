#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "../../shared/lora_config.h"
#include "../../shared/protocol.h"

// LilyGo T3 LoRa32 V1.6.1 — SX1278 pinleri
#define LORA_CS     18
#define LORA_RST    23
#define LORA_DIO0   26
#define LORA_SCK    5
#define LORA_MISO   19
#define LORA_MOSI   27
#define OLED_SDA    21
#define OLED_SCL    22
#define BOARD_LED   25

static const size_t MAX_HISTORY = 12;
static String history[MAX_HISTORY];
static size_t historyCount = 0;

static WebServer server(80);
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

static String lastRx = "";
static bool loraOk = false;

static void pushHistory(const String& line) {
    if (historyCount < MAX_HISTORY) {
        history[historyCount++] = line;
    } else {
        for (size_t i = 1; i < MAX_HISTORY; i++) history[i - 1] = history[i];
        history[MAX_HISTORY - 1] = line;
    }
}

static void drawOled() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "T3 LoRa32 v1.6.1");
    u8g2.drawStr(0, 24, WiFi.softAPIP().toString().c_str());
    u8g2.drawStr(0, 38, loraOk ? "LoRa: OK" : "LoRa: HATA");
    String status = "RX: " + lastRx.substring(0, 18);
    u8g2.drawStr(0, 52, status.c_str());
    u8g2.sendBuffer();
}

static bool initLoRa() {
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

    if (!LoRa.begin(LORA_FREQ_HZ)) {
        return false;
    }

    LoRa.setSpreadingFactor(LORA_SPREADING);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH_HZ);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
    LoRa.setTxPower(LORA_TX_POWER_DBM);
    LoRa.enableCrc();
    return true;
}

static void sendLoRaMessage(const char* text) {
    char packet[PROTO_MAX_LEN];
    if (!protocol_pack(packet, sizeof(packet), "LG", text)) return;

    LoRa.beginPacket();
    LoRa.print(packet);
    LoRa.endPacket();

    pushHistory("Ben: " + String(text));
    digitalWrite(BOARD_LED, HIGH);
    delay(50);
    digitalWrite(BOARD_LED, LOW);
    Serial.printf("TX LoRa: %s", packet);
}

static void handleRoot() {
    String html = R"raw(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>T3 LoRa</title>
<style>
*{box-sizing:border-box;font-family:system-ui,sans-serif}
body{margin:0;background:#0f172a;color:#e2e8f0;min-height:100vh}
.wrap{max-width:520px;margin:0 auto;padding:16px}
h1{font-size:1.2rem;margin:0 0 12px}
#log{background:#1e293b;border-radius:12px;padding:12px;height:55vh;overflow-y:auto;font-size:.95rem}
.msg{margin:6px 0;padding:8px 10px;border-radius:8px;background:#334155}
.msg.rx{background:#14532d}
.msg.tx{background:#1e3a5f;text-align:right}
form{display:flex;gap:8px;margin-top:12px}
input,button{font-size:1rem;padding:12px;border-radius:10px;border:none}
input{flex:1;background:#1e293b;color:#fff}
button{background:#3b82f6;color:#fff;font-weight:600}
small{display:block;margin-top:8px;color:#94a3b8}
</style>
</head>
<body>
<div class="wrap">
<h1>T3 LoRa32 Köprüsü</h1>
<div id="log"></div>
<form onsubmit="sendMsg(event)">
<input id="msg" placeholder="Mesaj yaz..." maxlength="160" autocomplete="off">
<button type="submit">Gönder</button>
</form>
<small>WiFi: )raw";
    html += LILYGO_AP_SSID;
    html += R"raw( | LoRa 433 MHz</small>
</div>
<script>
async function refresh(){
  const r=await fetch('/messages');
  const j=await r.json();
  const log=document.getElementById('log');
  log.innerHTML=j.messages.map(m=>`<div class="msg ${m.dir}">${m.text}</div>`).join('');
  log.scrollTop=log.scrollHeight;
}
async function sendMsg(e){
  e.preventDefault();
  const i=document.getElementById('msg');
  const t=i.value.trim();
  if(!t)return;
  await fetch('/send',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({message:t})});
  i.value='';
  refresh();
}
refresh();
setInterval(refresh,2000);
</script>
</body>
</html>
)raw";
    server.send(200, "text/html", html);
}

static void handleMessages() {
    String json = "{\"messages\":[";
    for (size_t i = 0; i < historyCount; i++) {
        if (i) json += ",";
        bool isTx = history[i].startsWith("Ben:");
        json += "{\"dir\":\"" + String(isTx ? "tx" : "rx") + "\",\"text\":\"";
        String esc = history[i];
        esc.replace("\"", "\\\"");
        json += esc + "\"}";
    }
    json += "]}";
    server.send(200, "application/json", json);
}

static void handleSend() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }
    String body = server.arg("plain");
    int start = body.indexOf("\"message\"");
    if (start < 0) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }
    int q1 = body.indexOf('"', body.indexOf(':', start) + 1);
    int q2 = body.indexOf('"', q1 + 1);
    String msg = body.substring(q1 + 1, q2);
    msg.trim();
    if (msg.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }
    sendLoRaMessage(msg.c_str());
    server.send(200, "application/json", "{\"ok\":true}");
}

static void setupWifiAp() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(LILYGO_AP_SSID, WIFI_AP_PASSWORD);
    Serial.printf("AP: %s  IP: %s\n", LILYGO_AP_SSID, WiFi.softAPIP().toString().c_str());
}

static void pollLoRaRx() {
    int packetSize = LoRa.parsePacket();
    if (packetSize == 0) return;

    String payload;
    while (LoRa.available()) {
        payload += (char)LoRa.read();
    }

    char src[8], msg[PROTO_MAX_LEN];
    if (protocol_parse(payload.c_str(), payload.length(), src, sizeof(src), msg, sizeof(msg))) {
        lastRx = String(msg);
        pushHistory(String(src) + ": " + msg);
        Serial.printf("RX [%s]: %s (RSSI %d)\n", src, msg, LoRa.packetRssi());
    } else {
        Serial.printf("RX ham: %s\n", payload.c_str());
        pushHistory("?: " + payload);
    }
    drawOled();
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== LilyGo T3 LoRa32 V1.6.1 Bridge ===");

    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    u8g2.setI2CAddress(0x3C << 1);
    u8g2.begin();
    u8g2.setContrast(200);

    loraOk = initLoRa();
    if (!loraOk) {
        Serial.println("LoRa baslatilamadi! 433 MHz SX1278 modul ve anten kontrol edin.");
    } else {
        Serial.println("LoRa hazir.");
    }

    setupWifiAp();

    server.on("/", handleRoot);
    server.on("/messages", handleMessages);
    server.on("/send", HTTP_POST, handleSend);
    server.begin();

    pushHistory("Sistem basladi");
    drawOled();
}

void loop() {
    server.handleClient();
    pollLoRaRx();

    static uint32_t lastDraw = 0;
    if (millis() - lastDraw > 3000) {
        drawOled();
        lastDraw = millis();
    }
}
