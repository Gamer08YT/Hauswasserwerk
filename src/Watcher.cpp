//
// Created by Jan Heil on 12.08.2023.
//

#include <esp32-hal-gpio.h>
#include "Watcher.h"
#include "PINOUT.h"

/**
 * @brief Getter method for level switch of the Watcher class.
 *
 * This method returns the level switch of the Watcher class.
 * The level switch determines at which level the Watcher should take action.
 *
 * @return The current level switch value.
 */

bool Watcher::getLevelSwitch() {
    return digitalRead(LEVEL_SWITCH);
}

/**
 * @defgroup Watcher Watcher
 * @brief Class responsible for setting up the Watcher module.
 *
 * The Watcher class is responsible for setting up the Watcher module. It
 * provides functions to initialize and configure the necessary components to
 * enable the monitoring of a specific system or application.
 */

void Watcher::setup() {
    pinMode(LEVEL_SWITCH, INPUT);
}
