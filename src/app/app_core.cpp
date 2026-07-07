/**
 * @file app_core.cpp
 * @brief Boots config, radio, transport, chat, storage, background tasks.
 */
#include "app_core.h"
#include "../../lib/chat/chat_service.h"
#include "../../lib/storage/message_store.h"
#include "../../lib/transport/reliable_transport.h"
#include "../../lib/transport/link_probe.h"
#include "../../lib/protocol/packet_builder.h"
#include "../../lib/common/logger.h"
#include "../../lib/common/queues.h"
#include <Arduino.h>
#include <memory>

namespace loralink::app {

using loralink::Logger;
using loralink::QueueHandle;
using namespace loralink::config;
using namespace loralink::storage;
using namespace loralink::transport;
using namespace loralink::chat;
using namespace loralink::protocol;
using namespace loralink::radio;

namespace {

std::unique_ptr<IRadioDriver> g_radio;
std::unique_ptr<MessageStore> g_store;
std::unique_ptr<ReliableTransport> g_transport;
std::unique_ptr<LinkProbe> g_probe;
std::unique_ptr<ChatService> g_chat;
std::unique_ptr<QueueHandle<Packet>> g_inbound;
ConfigStore g_config_store;
AppConfig g_config{};
TaskHandle_t g_protocol_task{nullptr};

RadioConfig toRadioConfig(const AppConfig& cfg) {
    RadioConfig rc{};
    rc.frequency_hz = cfg.frequency_hz;
    rc.spreading_factor = cfg.spreading_factor;
    rc.bandwidth_hz = cfg.bandwidth_hz;
    rc.coding_rate = cfg.coding_rate;
    rc.sync_word = cfg.sync_word;
    rc.tx_power_dbm = cfg.tx_power_dbm;
    rc.e220_channel = cfg.e220_channel;
    rc.preamble_length = 8;
    return rc;
}

void protocolTask(void*) {
    Logger::info("TASK", "protocol+chat task started");
    while (true) {
        const uint32_t now = millis();
        if (g_transport) g_transport->poll(now);
        if (g_probe) g_probe->poll(now);
        if (g_chat) g_chat->poll();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

} // namespace

bool start(std::unique_ptr<IRadioDriver> radio, const AppOptions& opts) {
    if (!radio) return false;
    g_radio = std::move(radio);

    g_config_store.loadDefaults(g_config, opts.role);
    if (!g_config_store.load(g_config)) {
        g_config_store.save(g_config);
        Logger::info("APP", "first boot — defaults saved to NVS");
    }

    g_radio->setConfig(toRadioConfig(g_config));
    // Radio initialized inside transport->begin()

    RetryPolicy policy;
    policy.base_timeout_ms = g_config.ack_timeout_ms;
    policy.max_retries = g_config.max_retries;

    g_transport = std::make_unique<ReliableTransport>(*g_radio, policy);
    g_transport->setLocalId(g_config.device_id);

    g_inbound = std::make_unique<QueueHandle<Packet>>(16);
    g_transport->setInboundQueue(g_inbound.get());

    if (!g_transport->begin()) {
        Logger::error("APP", "transport begin failed");
        stop();
        return false;
    }

    g_store = std::make_unique<MessageStore>();
    g_chat = std::make_unique<ChatService>(*g_transport, *g_store, g_config.device_id);
    g_chat->setInboundQueue(g_inbound.get());
    if (!g_chat->begin()) {
        Logger::error("APP", "chat/store begin failed");
        stop();
        return false;
    }

    if (opts.send_probes) {
        g_probe = std::make_unique<LinkProbe>(*g_transport, opts.peer_id);
        g_probe->setIntervalMs(opts.probe_interval_ms);
    }

    Packet hello = makeHello(g_config.device_id, opts.peer_id, 1);
    g_transport->send(hello, false);

    xTaskCreatePinnedToCore(protocolTask, "protocol", 10240, nullptr, 4,
                            &g_protocol_task, 1);

    Logger::info("APP", "%s ready id=%04X peer=%04X",
                 g_config.device_name, g_config.device_id, opts.peer_id);
    return true;
}

void stop() {
    if (g_protocol_task) {
        vTaskDelete(g_protocol_task);
        g_protocol_task = nullptr;
    }
    g_probe.reset();
    g_chat.reset();
    g_store.reset();
    g_inbound.reset();
    g_transport.reset();
    g_radio.reset();
}

AppConfig& config() { return g_config; }
ChatService* chat() { return g_chat.get(); }
ReliableTransport* transport() { return g_transport.get(); }
IRadioDriver* radio() { return g_radio.get(); }

} // namespace loralink::app
