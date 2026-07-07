#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <M5Cardputer.h>
#include "M5_LoRa_E220.h"

#include "../../shared/lora_config.h"
#include "../../shared/protocol.h"

// Cardputer Grove: TX=GPIO1, RX=GPIO2
#define LORA_UART_TX    1
#define LORA_UART_RX    2

static LoRa_E220 loraModule;
static LoRaConfigItem_t loraConfig;
static RecvFrame_t rxFrame;

static WebServer* webServer = nullptr;
static bool wifiApEnabled = false;

static String inputBuffer;
static String lastRx = "---";
static String statusLine = "Hazir";

static const size_t MAX_LINES = 8;
static String screenLines[MAX_LINES];
static size_t lineCount = 0;

static void addLine(const String& line) {
    if (lineCount < MAX_LINES) {
        screenLines[lineCount++] = line;
    } else {
        for (size_t i = 1; i < MAX_LINES; i++) screenLines[i - 1] = screenLines[i];
        screenLines[MAX_LINES - 1] = line;
    }
}

static void drawUi() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(BLACK);
    d.setTextColor(GREEN);
    d.setTextSize(1);
    d.setCursor(4, 4);
    d.println("Cardputer LoRa Bridge");

    d.setTextColor(CYAN);
    d.printf("WiFi: %s\n", wifiApEnabled ? CARDPUTER_AP_SSID : "Kapali (Fn+W)");
    d.setTextColor(YELLOW);
    d.printf("Durum: %s\n", statusLine.c_str());

    d.setTextColor(WHITE);
    for (size_t i = 0; i < lineCount; i++) {
        String s = screenLines[i];
        if (s.length() > 38) s = s.substring(0, 35) + "...";
        d.println(s);
    }

    d.setTextColor(ORANGE);
    d.setCursor(4, d.height() - 28);
    d.print("> ");
    d.print(inputBuffer);
    d.print("_");
    d.setCursor(4, d.height() - 12);
    d.setTextColor(DARKGREY);
    d.print("Enter=gonder Fn+W=WiFi Tab=temizle");
}

static bool configureE220() {
    loraModule.Init(&Serial2, CONFIG_MODE_BAUD, SERIAL_8N1, LORA_UART_RX, LORA_UART_TX);
    loraModule.SetDefaultConfigValue(loraConfig);

    loraConfig.own_address = E220_ADDRESS;
    loraConfig.baud_rate = BAUD_9600;
    loraConfig.uart_config = UART_8N1;
    loraConfig.air_data_rate = DATA_RATE_2_4Kbps;
    loraConfig.subpacket_size = SUBPACKET_200_BYTE;
    loraConfig.rssi_ambient_noise_flag = RSSI_AMBIENT_NOISE_DISABLE;
    loraConfig.transmitting_power = TX_POWER_22dBm;
    loraConfig.own_channel = E220_CHANNEL;
    loraConfig.rssi_byte_flag = RSSI_BYTE_DISABLE;
    loraConfig.transmission_method_type = UART_P2P_MODE;
    loraConfig.lbt_flag = LBT_DISABLE;
    loraConfig.wor_cycle = WOR_2000MS;
    loraConfig.encryption_key = 0x0000;
    loraConfig.target_address = 0x0000;
    loraConfig.target_channel = E220_CHANNEL;

    // DIP anahtarlari M0=1 M1=1 (config modu) olmali; basarisizsa atla
    if (loraModule.InitLoRaSetting(loraConfig) != 0) {
        Serial.println("E220 config atlandi (DIP M0/M1=11 mi?)");
        return false;
    }
    Serial.println("E220 yapilandirildi. DIP M0/M1=00 yapin.");
    return true;
}

static void sendLoRa(const char* text) {
    char packet[PROTO_MAX_LEN];
    if (!protocol_pack(packet, sizeof(packet), "CP", text)) return;

    if (loraModule.SendFrame(loraConfig, (uint8_t*)packet, strlen(packet)) == 0) {
        statusLine = "Gonderildi";
        addLine("Ben: " + String(text));
        Serial.printf("TX: %s", packet);
    } else {
        statusLine = "Gonderim hatasi";
    }
}

static void pollLoRaRx() {
    if (loraModule.RecieveFrame(&rxFrame) != 0) return;

    String payload;
    for (int i = 0; i < rxFrame.recv_data_len; i++) {
        payload += (char)rxFrame.recv_data[i];
    }

    char src[8], msg[PROTO_MAX_LEN];
    if (protocol_parse(payload.c_str(), payload.length(), src, sizeof(src), msg, sizeof(msg))) {
        lastRx = String(msg);
        addLine(String(src) + ": " + msg);
        statusLine = "Alindi";
        Serial.printf("RX [%s]: %s RSSI %d\n", src, msg, rxFrame.rssi);
    } else {
        addLine("?: " + payload);
    }
}

// --- WiFi AP + basit web arayuzu ---
static String webHistory[20];
static size_t webHistoryCount = 0;

static void webPush(const String& line) {
    if (webHistoryCount < 20) webHistory[webHistoryCount++] = line;
    else {
        for (size_t i = 1; i < 20; i++) webHistory[i - 1] = webHistory[i];
        webHistory[19] = line;
    }
}

static void handleWebRoot() {
    String html = R"raw(
<!DOCTYPE html><html lang="tr"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Cardputer LoRa</title>
<style>
body{font-family:system-ui;background:#111;color:#eee;margin:0;padding:16px}
#log{background:#222;border-radius:10px;padding:12px;height:50vh;overflow:auto}
form{display:flex;gap:8px;margin-top:12px}
input,button{padding:12px;border-radius:8px;border:none;font-size:1rem}
input{flex:1}button{background:#22c55e;color:#000;font-weight:700}
</style></head><body>
<h2>Cardputer LoRa</h2><div id="log"></div>
<form onsubmit="send(event)"><input id="m" placeholder="Mesaj" maxlength="160"><button>Gonder</button></form>
<script>
async function load(){const j=await(await fetch('/messages')).json();
document.getElementById('log').innerHTML=j.messages.map(x=>`<div>${x}</div>`).join('');}
async function send(e){e.preventDefault();const t=document.getElementById('m').value.trim();if(!t)return;
await fetch('/send',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({message:t})});
document.getElementById('m').value='';load();}
load();setInterval(load,2000);
</script></body></html>
)raw";
    webServer->send(200, "text/html", html);
}

static void handleWebMessages() {
    String json = "{\"messages\":[";
    for (size_t i = 0; i < webHistoryCount; i++) {
        if (i) json += ",";
        String esc = webHistory[i];
        esc.replace("\"", "\\\"");
        json += "\"" + esc + "\"";
    }
    json += "]}";
    webServer->send(200, "application/json", json);
}

static void handleWebSend() {
    String body = webServer->arg("plain");
    int q1 = body.indexOf('"', body.indexOf("message") + 8);
    int q2 = body.indexOf('"', q1 + 1);
    String msg = body.substring(q1 + 1, q2);
    msg.trim();
    if (msg.length()) {
        sendLoRa(msg.c_str());
        webPush("Ben: " + msg);
    }
    webServer->send(200, "application/json", "{\"ok\":true}");
}

static void startWifiAp() {
    if (wifiApEnabled) return;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(CARDPUTER_AP_SSID, WIFI_AP_PASSWORD);
    webServer = new WebServer(80);
    webServer->on("/", handleWebRoot);
    webServer->on("/messages", handleWebMessages);
    webServer->on("/send", HTTP_POST, handleWebSend);
    webServer->begin();
    wifiApEnabled = true;
    statusLine = "WiFi acik";
    addLine("AP: " + String(CARDPUTER_AP_SSID));
    Serial.printf("AP: %s IP %s\n", CARDPUTER_AP_SSID, WiFi.softAPIP().toString().c_str());
}

static void stopWifiAp() {
    if (!wifiApEnabled) return;
    webServer->stop();
    delete webServer;
    webServer = nullptr;
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiApEnabled = false;
    statusLine = "WiFi kapali";
}

static void toggleWifiAp() {
    if (wifiApEnabled) stopWifiAp();
    else startWifiAp();
    drawUi();
}

static void handleKeyboard() {
    if (!M5Cardputer.Keyboard.isChange()) return;
    auto status = M5Cardputer.Keyboard.keysState();

    // Fn+W ile WiFi toggle
    if (status.fn && status.word.find('w') != std::string::npos) {
        toggleWifiAp();
        return;
    }

    for (auto c : status.word) {
        if (c == '\b' || c == 127) {
            if (!inputBuffer.isEmpty()) inputBuffer.remove(inputBuffer.length() - 1);
        } else if (c == '\n' || c == '\r') {
            inputBuffer.trim();
            if (inputBuffer.length()) sendLoRa(inputBuffer.c_str());
            inputBuffer = "";
        } else if (c >= 32 && c < 127 && inputBuffer.length() < 160) {
            inputBuffer += c;
        }
    }

    if (status.enter) {
        inputBuffer.trim();
        if (inputBuffer.length()) sendLoRa(inputBuffer.c_str());
        inputBuffer = "";
    }

    if (status.tab) {
        inputBuffer = "";
    }

    drawUi();
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(80);

    Serial.begin(115200);
    delay(300);
    Serial.println("\n=== Cardputer LoRa Bridge ===");

    Serial2.begin(9600, SERIAL_8N1, LORA_UART_RX, LORA_UART_TX);
    configureE220();

    addLine("E220 kanal 0x17 / 433MHz");
    addLine("Fn+W = telefon WiFi");
    drawUi();
}

void loop() {
    M5Cardputer.update();
    handleKeyboard();
    pollLoRaRx();

    if (wifiApEnabled && webServer) {
        webServer->handleClient();
    }

    delay(5);
}
