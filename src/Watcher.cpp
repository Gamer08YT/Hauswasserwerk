//
// Created by Jan Heil on 12.08.2023.
//

#include <esp32-hal-gpio.h>
#include <Arduino.h>
#include "Watcher.h"
#include "PINOUT.h"
#include "SimpleTimer.h"
#include "EmonLib.h"
#include "Slave.h"
#include "Device.h"


// Timer for Measuring Updates.
SimpleTimer timerIO(500);

// Store Energy Monitor Instance.
EnergyMonitor monitor;

// Store max, min and normal Water Level.
int level_min = 30;
int level_max = 80;
int level_normal = 50;

// Store IRMS.
float irmsIO = 0;

// Store last Update Time.
unsigned long updatedIO = 0;

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
    pinMode(METER, INPUT);
    pinMode(LEVEL_TRIGGER, OUTPUT);

    // Setup Monitor.
    monitor.current(METER, 25);
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

        // Calcualte max Percent.
        int percentIO = TANK_PERCENT * distanceIO;

        Device::println(String(percentIO));

        // Trigger Max Alarm.
        if (percentIO > level_max) {
            // Disable all Pumps.
            Slave::setError(true, "Tank is to full.");
        }

        // Throw Alarm because Tank is to empty.
        if (percentIO < level_min) {
            // Disable/Lock Pump 3.
            Slave::setError(true, "Tank is to empty.");
        }

        // Stop Pump1 or Pump2.
        if (percentIO > level_normal) {
            Slave::setPump(false);
        }

        // Pump new Water into the Tank.
        if (distanceIO < (percentIO - 10)) {
            Slave::setPump(true);
        }

        // Reset Timer (Endless Loop).
        timerIO.reset();
    }
}

/**
 * @brief Get the power of the Watcher object.
 *
 * This function returns the power of the Watcher object.
 *
 * @return The power of the Watcher object.
 */

float Watcher::getPower() {
    return (float) irmsIO * 230;
}

/**
 * @brief Handles the measurement data in the Watcher class.
 *
 * This function is responsible for processing the measurement data received in the Watcher class.
 * It performs the necessary calculations and updates the internal state of the Watcher object accordingly.
 *
 * @param measurementData The measurement data to be handled.
 * @return void
 */

void Watcher::handleMeasurement() {
    // Give Function 1000ms debounce time.
    if ((millis() - updatedIO) > 1000) {
        // Calc only IRMS in A.
        irmsIO = (monitor.calcIrms(1480) / 1000);

        // Update Timestamp.
        updatedIO = millis();
    }
}

/**
 * @brief Set the maximum value of the variable.
 *
 * This function sets the maximum value of the variable controlled by the Watcher class.
 * The maximum value determines the upper limit for the valueIO parameter.
 *
 * @param valueIO Integer value indicating the maximum value.
 */

void Watcher::setMax(int valueIO) {
    if (valueIO < 85) {
        level_max = valueIO;
    }
}

/**
 * \brief Sets the minimum value for the Watcher.
 *
 * This function sets the minimum value for the Watcher. The minimum value
 * specifies the lower limit for the input value. It ensures that the valueIO
 * cannot be set below this limit.
 *
 * \param valueIO The new minimum value to be set for the Watcher.
 */

void Watcher::setMin(int valueIO) {
    if (valueIO > 25) {
        level_min = valueIO;
    }
}

/**
 * \brief Sets the normal value for IO.
 *
 * This function is a member of the Watcher class and is used to set the normal value for Input/Output (IO).
 * The normal value represents the expected or reference value for the IO.
 *
 * \param valueIO The normal value to be set for IO.
 *
 * \return None.
 */

void Watcher::setNormal(int valueIO) {
    if (valueIO > level_min && valueIO < level_max) {
        level_normal = valueIO;
    }
}
