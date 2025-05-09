//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_WATCHER_H
#define HAUSWASSERWERK_WATCHER_H

class Watcher
{
public:
    static void setup();

    static bool getLevelSwitch();

    static bool readLevelAlarm();

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
    static bool getIgnoreMoist();
    static void setIgnoreMoist(bool valueIO);

    static float getDistance();

    static void setMinV(float aFloat);

    static void setMaxV(float aFloat);

    static void setRatio(int8_t int8);

private:
    static void handleConditions();

    static void readCurrent();

    static void refill();

    static void stopRefill();
};


#endif //HAUSWASSERWERK_WATCHER_H
