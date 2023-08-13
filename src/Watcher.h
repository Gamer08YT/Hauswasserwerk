//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_WATCHER_H
#define HAUSWASSERWERK_WATCHER_H


#include "utils/HANumeric.h"

class Watcher {
public:
    static void setup();
    static bool getLevelSwitch();
    static float getLevelDistance();
    static void handleMeasurement();
    static float getPower();
    static void loop();

    static void setNormal(int numeric);

    static void setMax(int numeric);

    static void setMin(int numeric);

    static int getMax();

    static int getMin();

    static int getNormal();
};


#endif //HAUSWASSERWERK_WATCHER_H
