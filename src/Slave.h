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

    static void setError(bool stateIO, String codeIO = "Unknown Error", int flashIO = 500);

    // Give Pump 3 Power for 30 Seconds otherwise throw Error Alarm.
    static void setPump(bool stateIO);

private:
    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendGet(int idIO, char *urlIO);

    static ArduinoJson::BasicJsonDocument<ArduinoJson::DefaultAllocator> sendPost(int idIO, char *urlIO, String dataIO);

    static String getURL(int idIO, char *urlIO);

    static void runError(void *parameter);
};


#endif //HAUSWASSERWERK_SLAVE_H
