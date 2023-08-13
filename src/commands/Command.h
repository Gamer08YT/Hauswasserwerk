//
// Created by Jan Heil on 03.07.2023.
//

#ifndef HAUSWASSERWERK_COMMAND_H
#define HAUSWASSERWERK_COMMAND_H


#include <WString.h>

class Command {
public:
    virtual const char *invoke() {
        return "empty";
    }

    virtual const char *description() {
        return "Empty Description.";
    }

    virtual void execute(std::vector<String> argsIO);
};


#endif //HAUSWASSERWERK_COMMAND_H
