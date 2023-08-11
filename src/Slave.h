//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_SLAVE_H
#define HAUSWASSERWERK_SLAVE_H


#include "ArduinoJson/Document/BasicJsonDocument.hpp"
#include "ArduinoJson/Document/DynamicJsonDocument.hpp"

class Slave {
public:
    static void setSlave(int idIO, bool stateIO);

private:
    static sendGet(int idIO, char *urlIO);

    static ArduinoJson::V6213PB2::BasicJsonDocument<ArduinoJson::V6213PB2::DefaultAllocator>
    sendPost(int idIO, char *urlIO, char *dataIO);

    static String getURL(int idIO, char *urlIO, int typeIO = 0);
};


#endif //HAUSWASSERWERK_SLAVE_H
