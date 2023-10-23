//
// Created by Jan Heil on 20.08.2023.
//

#include "UnlockCommand.h"
#include "Device.h"
#include "Slave.h"

void UnlockCommand::execute(std::vector<String> argsIO) {
    Slave::setError(false);
    Device::println("Unlocked Error State.");
}
