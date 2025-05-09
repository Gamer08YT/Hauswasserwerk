//
// Created by Jan Heil on 09.08.2023.
//

#ifndef HAUSWASSERWERK_MQTT_H
#define HAUSWASSERWERK_MQTT_H


#include "device-types/HANumber.h"
#include "utils/HANumeric.h"
#include "device-types/HASwitch.h"
#include "device-types/HAButton.h"

class MQTT {

public:
    static void onMax(HANumeric, HANumber *);

    static void onNormal(HANumeric, HANumber *);

    static void onMin(HANumeric, HANumber *);

    static void minV(HANumeric, HANumber *);

    static void maxV(HANumeric, HANumber *);

    static void onRatio(HANumeric number, HANumber* sender);

    static void onBuzzer(HAButton *);

    static void onIgnore(bool state, HASwitch* sender);
};


#endif //HAUSWASSERWERK_MQTT_H
