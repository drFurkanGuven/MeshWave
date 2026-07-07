#include "radio_self_test.h"
#include "../../common/logger.h"

namespace loralink::radio {

SelfTestResult runRadioSelfTest(IRadioDriver& radio) {
    using loralink::Logger;
    SelfTestResult r{};
    r.driver_created = true;
    r.driver_name = radio.driverName();

    if (!radio.initialize()) {
        Logger::error("RADIO", "%s initialize() failed", r.driver_name);
        return r;
    }
    r.initialized = true;
    Logger::info("RADIO", "%s ready, RSSI=%d", r.driver_name, radio.readRSSI());
    return r;
}

} // namespace loralink::radio
