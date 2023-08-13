//
// Created by Jan Heil on 09.08.2023.
//

#include "MQTT.h"
#include "Slave.h"
#include "Watcher.h"


void MQTT::onSmart(bool state, HASwitch *sender) {
    Serial.println("SMART VALUE CHANGED");
    //Serial.println(Slave::getPower(0));
    //Slave::setSlave(0, state);
    Slave::setPump(state);

    sender->setState(state);
}

void MQTT::onNormal(HANumeric numberIO, HANumber *sender) {
    Watcher::setNormal(numberIO.toInt8());
}

void MQTT::onMax(HANumeric numberIO, HANumber *sender) {
    Watcher::setMax(numberIO.toInt8());
}

void MQTT::onMin(HANumeric numberIO, HANumber *sender) {
    Watcher::setMin(numberIO.toInt8());
}
