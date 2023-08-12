//
// Created by Jan Heil on 09.08.2023.
//

#ifndef HAUSWASSERWERK_MQTT_H
#define HAUSWASSERWERK_MQTT_H


#include "device-types/HANumber.h"
#include "utils/HANumeric.h"
#include "device-types/HASwitch.h"

class MQTT {

public:
    static void onTrigger(HANumeric, HANumber *);

    static void onSmart(bool state, HASwitch *sender);
};


#endif //HAUSWASSERWERK_MQTT_H