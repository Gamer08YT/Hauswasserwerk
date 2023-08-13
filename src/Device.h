//
// Created by Jan Heil on 13.08.2023.
//

#ifndef HAUSWASSERWERK_DEVICE_H
#define HAUSWASSERWERK_DEVICE_H


#include "ESPTelnetBase.h"
#include "ESPTelnet.h"
#include "EscapeCodes.h"
#include "vector"
#include "commands/Command.h"

class Device {
public:
    static void beginTelnet();

    static void println(String dataIO);

    static void print(String dataIO);

    static void handleTelnet();

    static void write(String dataIO);

    static void onConnect(String addressIO);

    static void onDisconnect(String addressIO);

    static void onInput(String dataIO);

    static ESPTelnet getTelnet();

    static EscapeCodes getANSI();

    static std::vector<Command *> getCommands();

    static void addCommand(Command *commandIO);

    static void addCommands();

    static std::vector<String> split(String &valueIO, const char *delimiterIO = "");

    static void handleCommand(String dataIO);

    static void print_device();
};


#endif //HAUSWASSERWERK_DEVICE_H
