//
// Created by Jan Heil on 12.08.2023.
//

#include <esp32-hal-gpio.h>
#include <Arduino.h>
#include "Watcher.h"
#include "PINOUT.h"
#include "SimpleTimer.h"


// Timer for Measuring Updates.
SimpleTimer timerIO(500);

float distanceIO, durationIO = -1;

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
    pinMode(LEVEL_ECHO, INPUT);
    pinMode(LEVEL_TRIGGER, OUTPUT);
}

/**
 * \fn int Watcher::getLevelDistance()
 * \brief Retrieves the level distance of the Watcher.
 *
 * This function returns the level distance of the Watcher. The level distance is a measurement indicating the horizontal distance between the current level and the target level.
 *
 * \return The level distance of the Watcher as an integer.
 */

float Watcher::getLevelDistance() {
    return distanceIO;
}

void Watcher::loop() {
    if (timerIO.isReady()) {
        // Generate Trigger Signal.
        digitalWrite(LEVEL_TRIGGER, HIGH);
        delayMicroseconds(10);
        digitalWrite(LEVEL_TRIGGER, LOW);

        // Measure Duration between ECHO Pin.
        durationIO = pulseIn(LEVEL_ECHO, HIGH);

        // Calculate the Distance.
        distanceIO = 0.017 * durationIO;

        // Reset Timer (Endless Loop).
        timerIO.reset();
    }
}
