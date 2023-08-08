//
// Created by Jan Heil on 03.07.2023.
//

#ifndef HAUSWASSERWERK_RESETCOMMAND_H
#define HAUSWASSERWERK_RESETCOMMAND_H


#include <vector>
#include "Command.h"

class ResetCommand : public Command {
public:
    const char *invoke() override {
        return "reset";
    }

    const char *description() override {
        return "Reset ESP Device.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_RESETCOMMAND_H
