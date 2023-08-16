//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_SLAVE_H
#define HAUSWASSERWERK_SLAVE_H


#include "ArduinoJson.h"

class Slave {
public:
    static void setup();

    static void setSlave(int idIO, bool stateIO);

    static BasicJsonDocument<DefaultAllocator> getSlave(int idIO);

    static float getPower(int idIO);

    static void setError(bool stateIO, String codeIO = "Unknown Error", bool flashIO = false, String displayIO = "UNKNOWN");

    // Give Pump 3 Power for 30 Seconds otherwise throw Error Alarm.
    static void setPump(bool stateIO);

    static String getError();

    static bool getState(int idIO);

    static void setDisplay(String messageIO);

    static void showBootscreen();

    static void infoDisplay(const char *titleIO, String contentIO);

    static void updateLine(String contentIO, int xIO, int yIO);

    static void loop();

private:
    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendGet(int idIO, char *urlIO);

    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendPost(int idIO, char *urlIO, String dataIO);

    static String getURL(int idIO, char *urlIO);

    static void runError(void *parameter);

    static void setContrast(int valueIO);
};


#endif //HAUSWASSERWERK_SLAVE_H
