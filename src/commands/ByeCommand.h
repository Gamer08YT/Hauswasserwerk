//
// Created by Jan Heil on 03.07.2023.
//

#ifndef HAUSWASSERWERK_BYECOMMAND_H
#define HAUSWASSERWERK_BYECOMMAND_H


#include <vector>
#include "Command.h"

class ByeCommand : public Command {
public:
    const char *invoke() override {
        return "bye";
    }

    const char *description() override {
        return "Close Telnet Session.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_BYECOMMAND_H
