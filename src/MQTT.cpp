//
// Created by Jan Heil on 09.08.2023.
//

#include "MQTT.h"
#include "Slave.h"

static void *onTrigger(HANumeric numericIO, HANumber *numberIO) {
    Serial.println(numericIO.toFloat());
}


void MQTT::onSmart(bool state, HASwitch *sender) {
    Slave::setSlave(0, state);

    sender->setState(state);
}
