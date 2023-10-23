//
// Created by Jan Heil on 03.07.2023.
//

#ifndef HAUSWASSERWERK_INFOCOMMAND_H
#define HAUSWASSERWERK_INFOCOMMAND_H


#include <vector>
#include "Command.h"

class InfoCommand : public Command {
public:
    const char* invoke() override {
        return "info";
    }

    const char* description() override {
        return "Get Device Information.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_INFOCOMMAND_H
