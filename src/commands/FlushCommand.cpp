//
// Created by Jan Heil on 20.08.2023.
//

#include "FlushCommand.h"
#include "Device.h"
#include "Slave.h"

void FlushCommand::execute(std::vector<String> argsIO) {
    if (!argsIO.empty()) {
        int timeIO = 5;

        if (argsIO.size() > 1)
            timeIO = argsIO[1].toInt();

        if (argsIO[0] == "1" || argsIO[0] == "2") {
            Slave::setSlave((argsIO[0].toInt() - 1), true, timeIO);
            delay(timeIO * 1000);
            Slave::setSlave(argsIO[0].toInt(), false);
        } else if (argsIO[0] == "3") {
            Slave::setPump(true);
            delay(timeIO * 1000);
            Slave::setPump(false);
        } else {
            // Print Debug Message.
            Device::println("Given Pump not exits.");
        }
    } else
        // Print Debug Message.
        Device::println("Please use /flush <Pump> <Time>");
}
