#include "api_handlers.h"
#include "json_util.h"
#include <loralink/version.h>
#include <cstdio>

namespace loralink::webserver {

namespace {

const char* directionToken(loralink::MessageDirection dir) {
    return dir == loralink::MessageDirection::Outbound ? "out" : "in";
}

} // namespace

String ApiHandlers::statusJson(const ApiContext& ctx) {
    String j = "{";
    j += "\"linked\":";
    j += ctx.linked ? "true" : "false";
    j += ",\"rssi\":";
    j += ctx.rssi;
    j += ",\"snr\":";
    j += ctx.snr;
    j += ",\"uptime\":";
    j += ctx.uptime_ms / 1000U;
    j += ",\"lastPeerMs\":";
    j += ctx.last_peer_ms;
    if (ctx.device_name) {
        j += ",\"device\":";
        appendJsonEscaped(j, ctx.device_name);
    }
    j += ",\"version\":\"";
    j += LORALINK_VERSION_STRING;
    j += "\"";
    if (ctx.stats) {
        j += ",\"tx\":";
        j += ctx.stats->tx_count;
        j += ",\"rx\":";
        j += ctx.stats->rx_count;
        j += ",\"ack\":";
        j += ctx.stats->ack_count;
        j += ",\"pongs\":";
        j += ctx.stats->pongs_rx;
    }
    j += "}";
    return j;
}

String ApiHandlers::messagesJson(const chat::ConversationModel& model) {
    String j = "{\"messages\":[";
    for (size_t i = 0; i < model.count(); i++) {
        if (i) j += ',';
        const auto& msg = model.at(i);
        j += "{\"id\":";
        j += msg.id;
        j += ",\"ts\":";
        j += msg.timestamp;
        j += ",\"dir\":\"";
        j += directionToken(msg.direction);
        j += "\",\"from\":\"";
        char from_buf[8];
        snprintf(from_buf, sizeof(from_buf), "%04x", msg.source_id);
        j += from_buf;
        j += "\",\"text\":";
        appendJsonEscaped(j, msg.text);
        j += "}";
    }
    j += "]}";
    return j;
}

} // namespace loralink::webserver
