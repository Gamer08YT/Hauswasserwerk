//
// Created by Jan Heil on 20.08.2023.
//

#ifndef HAUSWASSERWERK_FLUSHCOMMAND_H
#define HAUSWASSERWERK_FLUSHCOMMAND_H


#include <vector>
#include "Command.h"

class FlushCommand : public Command {
public:
    const char *invoke() override {
        return "flush";
    }

    const char *description() override {
        return "Enable given Pump.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_FLUSHCOMMAND_H
