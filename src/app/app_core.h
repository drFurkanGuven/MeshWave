#pragma once

#include "../../lib/config/config_store.h"
#include "../../lib/common/types.h"
#include "../../lib/drivers/radio/i_radio_driver.h"
#include <memory>

namespace loralink::chat { class ChatService; }
namespace loralink::transport { class ReliableTransport; }
namespace loralink::radio { class IRadioDriver; }

namespace loralink::app {

struct AppOptions {
    loralink::DeviceRole role{loralink::DeviceRole::Terminal};
    loralink::DeviceId peer_id{loralink::kGatewayId};
    bool send_probes{false};
    uint32_t probe_interval_ms{5000};
};

bool start(std::unique_ptr<loralink::radio::IRadioDriver> radio, const AppOptions& opts);
void stop();

loralink::config::AppConfig& config();
loralink::chat::ChatService* chat();
loralink::transport::ReliableTransport* transport();
loralink::radio::IRadioDriver* radio();

} // namespace loralink::app
