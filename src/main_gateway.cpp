#include <Arduino.h>
#include "board.h"
#include <loralink/version.h>
#include "logger.h"
#include "app/gateway_app.h"

using loralink::Logger;

void setup() {
    Serial.begin(115200);
    delay(500);
    Logger::info("MAIN", "LoRaLink %s v%s", BOARD_NAME, LORALINK_VERSION_STRING);

    if (loralink::app::startGateway()) {
        Logger::info("MAIN", "LoRaLink Gateway running");
    }
}

void loop() {
    loralink::app::gatewayLoop();
}
