//
// Created by Jan Heil on 20.08.2023.
//

#ifndef HAUSWASSERWERK_UNLOCKCOMMAND_H
#define HAUSWASSERWERK_UNLOCKCOMMAND_H


#include <vector>
#include "Command.h"

class UnlockCommand : public Command {
public:
    const char *invoke() override {
        return "unlock";
    }

    const char *description() override {
        return "Unlock Error State.";
    }

    void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_UNLOCKCOMMAND_H
