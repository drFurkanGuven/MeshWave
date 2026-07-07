#pragma once

/**
 * @file radio_self_test.h
 * @brief Quick radio driver validation at boot.
 */

#include "i_radio_driver.h"

namespace loralink::radio {

struct SelfTestResult {
    bool driver_created{false};
    bool initialized{false};
    const char* driver_name{nullptr};
};

SelfTestResult runRadioSelfTest(IRadioDriver& radio);

} // namespace loralink::radio
