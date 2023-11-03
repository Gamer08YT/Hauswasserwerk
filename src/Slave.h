//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_SLAVE_H
#define HAUSWASSERWERK_SLAVE_H


#include "ArduinoJson.h"

class Slave {
public:
    static void setup();

    static void setSlave(int idIO, bool stateIO, int timeIO = 5);

    static BasicJsonDocument<DefaultAllocator> getSlave(int idIO);

    static float getPower(int idIO);

    static void setError(bool stateIO, String codeIO = "Unknown Error", bool flashIO = false, String displayIO = "UNKNOWN");

    // Give Pump 3 Power for 30 Seconds otherwise throw Error Alarm.
    static void setPump(bool stateIO, bool forceIO = false);

    static String getError();

    static bool getState(int idIO);

    static void setDisplay(String messageIO);

    static void showBootscreen();

    static void infoDisplay(const char *titleIO, String contentIO, bool forceIO = false);

    static void updateLine(String contentIO, int xIO, int yIO, bool forceIO = false);

    static void loop();

    static void ntp();

    static void runError(void *parameter);

private:
    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendGet(int idIO, char *urlIO);

    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendPost(int idIO, char *urlIO, String dataIO);

    static String getURL(int idIO, char *urlIO);

    static void setContrast(int valueIO);

    static void setDisplayActive();

    static void checkSlaveState(int idIO);
};


#endif //HAUSWASSERWERK_SLAVE_H
