//
// Created by Jan Heil on 03.07.2023.
//

#ifndef HAUSWASSERWERK_TESTCOMMAND_H
#define HAUSWASSERWERK_TESTCOMMAND_H


#include <vector>
#include "Command.h"

class TestCommand : public Command {
public:
    const char *invoke() override {
        return "test";
    }

    const char *description() override {
        return "Test Device Buzzer.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_TESTCOMMAND_H
