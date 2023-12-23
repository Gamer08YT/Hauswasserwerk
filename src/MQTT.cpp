//
// Created by Jan Heil on 09.08.2023.
//

#include "MQTT.h"
#include "Slave.h"
#include "Watcher.h"


void MQTT::onSmart(bool state, HASwitch *sender) {
    Serial.println("SMART VALUE CHANGED");

    sender->setState(state);
}

void MQTT::onNormal(HANumeric numberIO, HANumber *sender) {
    Watcher::setNormal(numberIO.toInt8());

    sender->setState(Watcher::getNormal());
}

void MQTT::onMax(HANumeric numberIO, HANumber *sender) {
    Watcher::setMax(numberIO.toInt8());

    sender->setState(Watcher::getMax());
}

void MQTT::onMin(HANumeric numberIO, HANumber *sender) {
    Watcher::setMin(numberIO.toInt8());

    sender->setState(Watcher::getMin());
}

void MQTT::minV(HANumeric numberIO, HANumber *senderIO) {
    Watcher::setMinV(numberIO.toFloat());

    senderIO->setState(numberIO);
}

void MQTT::maxV(HANumeric numberIO, HANumber *senderIO) {
    Watcher::setMaxV(numberIO.toFloat());

    senderIO->setState(numberIO);
}

void MQTT::onRatio(HANumeric numberIO, HANumber* sender) {
    Watcher::setRatio(numberIO.toInt8());

    sender->setState(numberIO);
}
