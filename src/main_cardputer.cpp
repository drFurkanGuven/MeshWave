#include <Arduino.h>
#include "board.h"
#include <loralink/version.h>
#include "logger.h"
#include "app/cardputer_app.h"

using loralink::Logger;

void setup() {
    Serial.begin(115200);
    delay(500);
    Logger::info("MAIN", "LoRaLink %s v%s", BOARD_NAME, LORALINK_VERSION_STRING);

    if (loralink::app::startCardputer()) {
        Logger::info("MAIN", "LoRaLink Cardputer running");
    }
}

void loop() {
    loralink::app::cardputerLoop();
}
