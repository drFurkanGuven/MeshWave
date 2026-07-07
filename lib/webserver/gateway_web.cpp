#include "gateway_web.h"
#include "api_handlers.h"
#include "json_util.h"
#include "../common/logger.h"
#include <loralink/version.h>

#if HAS_WEB_SERVER

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <cstring>

namespace loralink::webserver {

using loralink::Logger;
using loralink::kCardputerId;
using loralink::MessageDirection;

namespace {

GatewayWeb* s_instance{nullptr};
AsyncWebServer* s_server{nullptr};
AsyncWebSocket* s_ws{nullptr};

const char* directionToken(MessageDirection dir) {
    return dir == MessageDirection::Outbound ? "out" : "in";
}

} // namespace

String GatewayWeb::messageToJson(const chat::ChatMessage& msg) const {
    String j = "{";
    j += "\"id\":";
    j += msg.id;
    j += ",\"ts\":";
    j += msg.timestamp;
    j += ",\"dir\":\"";
    j += directionToken(msg.direction);
    j += "\",\"text\":";
    appendJsonEscaped(j, msg.text);
    j += "}";
    return j;
}

String GatewayWeb::buildMessagesJson() const {
    String j = "{\"messages\":[";
    if (m_ctx.chat) {
        const auto& model = m_ctx.chat->conversation();
        for (size_t i = 0; i < model.count(); i++) {
            if (i) j += ',';
            j += messageToJson(model.at(i));
        }
    }
    j += "]}";
    return j;
}

String GatewayWeb::buildHistoryEnvelope() const {
    String j = "{\"type\":\"history\",\"messages\":[";
    if (m_ctx.chat) {
        const auto& model = m_ctx.chat->conversation();
        for (size_t i = 0; i < model.count(); i++) {
            if (i) j += ',';
            j += messageToJson(model.at(i));
        }
    }
    j += "]}";
    return j;
}

String GatewayWeb::buildStatusJson(uint32_t now_ms) const {
    ApiContext ctx{};
    ctx.conversation = m_ctx.chat ? &m_ctx.chat->conversation() : nullptr;
    ctx.stats = m_ctx.transport ? &m_ctx.transport->stats() : nullptr;
    ctx.rssi = m_ctx.radio ? m_ctx.radio->readRSSI() : 0;
    ctx.linked = ctx.stats ? ctx.stats->link_up : false;
    ctx.uptime_ms = now_ms;
    if (ctx.stats) {
        ctx.snr = ctx.stats->last_snr;
        ctx.last_peer_ms = ctx.stats->last_peer_ms;
    }
    if (m_ctx.config) {
        ctx.device_name = m_ctx.config->device_name;
    }
    return ApiHandlers::statusJson(ctx);
}

void GatewayWeb::onChatMessage(const chat::ChatMessage& msg) {
    if (!s_ws) return;
    String payload = "{\"type\":\"message\",";
    payload += messageToJson(msg).substring(1);
    s_ws->textAll(payload);
}

void GatewayWeb::broadcastStatus(uint32_t now_ms) {
    if (!s_ws || s_ws->count() == 0) return;

    const auto* stats = m_ctx.transport ? &m_ctx.transport->stats() : nullptr;
    const int16_t rssi = m_ctx.radio ? m_ctx.radio->readRSSI() : 0;
    const bool linked = stats ? stats->link_up : false;

    String payload = "{\"type\":\"status\",\"linked\":";
    payload += linked ? "true" : "false";
    payload += ",\"rssi\":";
    payload += rssi;
    payload += ",\"snr\":";
    payload += stats ? stats->last_snr : 0;
    payload += ",\"uptime\":";
    payload += now_ms / 1000U;
    payload += ",\"lastPeerMs\":";
    payload += stats ? stats->last_peer_ms : 0;
    if (stats) {
        payload += ",\"tx\":";
        payload += stats->tx_count;
        payload += ",\"rx\":";
        payload += stats->rx_count;
    }
    payload += "}";
    s_ws->textAll(payload);
}

bool GatewayWeb::begin(const GatewayWebContext& ctx, uint16_t port) {
    if (m_running) return true;
    if (!ctx.chat || !ctx.transport) {
        Logger::error("WEB", "missing chat/transport context");
        return false;
    }

    m_ctx = ctx;
    s_instance = this;

    static AsyncWebServer server(port);
    static AsyncWebSocket ws("/ws");
    s_server = &server;
    s_ws = &ws;

    if (!LittleFS.begin(false)) {
        Logger::warning("WEB", "LittleFS not mounted — web assets may be missing");
    }

    ws.onEvent([](AsyncWebSocket* webSocket, AsyncWebSocketClient* client,
                  AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (!s_instance || !s_ws) return;

        switch (type) {
            case WS_EVT_CONNECT: {
                Logger::info("WS", "client #%u connected (%u total)",
                             client->id(), webSocket->count());
                client->text(s_instance->buildHistoryEnvelope());
                s_instance->broadcastStatus(millis());
                break;
            }
            case WS_EVT_DISCONNECT:
                Logger::info("WS", "client #%u disconnected", client->id());
                break;
            case WS_EVT_DATA: {
                auto* info = reinterpret_cast<AwsFrameInfo*>(arg);
                if (!info || !info->final || info->index != 0 || info->len != len ||
                    info->opcode != WS_TEXT) {
                    break;
                }
                if (len >= 512) break;

                char buf[512];
                std::memcpy(buf, data, len);
                buf[len] = '\0';

                char text[500];
                if (!extractJsonString(buf, "text", text, sizeof(text))) break;
                if (!text[0]) break;

                if (s_instance->m_ctx.chat) {
                    s_instance->m_ctx.chat->sendText(text, kCardputerId);
                    Logger::info("WS", "phone -> LoRa: %s", text);
                }
                break;
            }
            default:
                break;
        }
    });

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        req->send(200, "application/json", buildStatusJson(millis()));
    });

    server.on("/api/messages", HTTP_GET, [this](AsyncWebServerRequest* req) {
        req->send(200, "application/json", buildMessagesJson());
    });

    server.onNotFound([](AsyncWebServerRequest* req) {
        if (req->method() == HTTP_OPTIONS) {
            req->send(200);
            return;
        }
        req->send(404, "text/plain", "Not found");
    });

    server.addHandler(&ws);
    server.begin();

    m_ctx.chat->setMessageListener([this](const chat::ChatMessage& msg) {
        onChatMessage(msg);
    });

    m_running = true;
    m_last_status_ms = 0;
    Logger::info("WEB", "HTTP+WS on port %u (LittleFS /)", port);
    return true;
}

void GatewayWeb::poll(uint32_t now_ms) {
    if (!m_running || !s_ws) return;

    s_ws->cleanupClients();

    if (now_ms - m_last_status_ms >= 3000) {
        broadcastStatus(now_ms);
        m_last_status_ms = now_ms;
    }
}

void GatewayWeb::stop() {
    if (!m_running) return;
    if (m_ctx.chat) m_ctx.chat->setMessageListener(nullptr);
    if (s_server) s_server->end();
    m_running = false;
    s_instance = nullptr;
    s_server = nullptr;
    s_ws = nullptr;
}

} // namespace loralink::webserver

#else

namespace loralink::webserver {

bool GatewayWeb::begin(const GatewayWebContext&, uint16_t) { return false; }
void GatewayWeb::poll(uint32_t) {}
void GatewayWeb::stop() {}

} // namespace loralink::webserver

#endif
