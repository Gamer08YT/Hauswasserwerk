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

bool level_state;

// Store IRMS.
float irmsIO = 0;

// Store last Update Time.
unsigned long updatedIO = 0;

float distanceIO, durationIO;
float percentIO = 0;

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
    pinMode(UNLOCK_BUTTON, INPUT);
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
 * 100% - 80% = 20%
 */

float Watcher::getLevelDistance() {
    return percentIO;
}

/**
 * \brief Executes the loop code.
 *
 * This function is responsible for executing the main loop code of the `Watcher` class.
 * It continuously monitors and performs certain actions based on certain conditions.
 *
 * \return None.
 */

void Watcher::loop() {
    // Read Unlock Button.
    /*bool stateIO = digitalRead(UNLOCK_BUTTON);

    // Unlock Error if State is true.
    if (stateIO) {
        Slave::setError(false);

        // Print Debug Message.
        Device::println("Unlocked Error Lock");
    }*/

    if (timerIO.isReady()) {
        // Generate Trigger Signal.
        digitalWrite(LEVEL_TRIGGER, HIGH);
        delayMicroseconds(10);
        digitalWrite(LEVEL_TRIGGER, LOW);

        // Measure Duration between ECHO Pin.
        durationIO = pulseIn(LEVEL_ECHO, HIGH);

        // Calculate the Distance.
        distanceIO = (0.017 * durationIO);

        // Calcualte max Percent.
        percentIO = (100 - (distanceIO * 100) / TANK_HEIGHT);

        // Handle Conditions.
        handleConditions();

        // Handle Level Switch.
        handleLevelSwitch();

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

        Device::println("Set Max Level Value to ");
        Device::println(String(valueIO));
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

        Device::println("Set Min Level Value to ");
        Device::println(String(valueIO));
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

        Device::println("Set Normal Level Value to ");
        Device::println(String(valueIO));
    }
}

int Watcher::getMax() {
    return level_max;
}

int Watcher::getMin() {
    return level_min;
}

int Watcher::getNormal() {
    return level_normal;
}

/**
 * @brief Handles the conditions in the Watcher class.
 *
 * This method is responsible for handling the conditions in the Watcher class. It processes
 * the conditions and takes appropriate action based on the result. The conditions are checked
 * against the defined criteria and necessary steps are performed accordingly.
 *
 * @param void
 *
 * @return void
 */

bool setSlave1Status = false;
bool setSlave2Status = false;
bool setPumpStatus = false;

void Watcher::handleConditions() {
    if (percentIO >= 80) {
        // Wenn der Füllstand 80 erreicht oder übersteigt, Setzen Sie setSlave1 und setSlave2 auf false.
        if (setSlave1Status || setSlave2Status) { // Nur Status setzen, wenn notwendig
            Slave::setSlave(0, false);
            Slave::setSlave(1, false);

            setSlave1Status = false;
            setSlave2Status = false;
        }
    } else {
        if (!setSlave1Status || !setSlave2Status) { // Nur Status setzen, wenn notwendig
            Slave::setSlave(0, true);
            Slave::setSlave(1, true);

            setSlave1Status = true;
            setSlave2Status = true;
        }
    }

    if (percentIO > 30) {
        // Wenn der Füllstand über 30 liegt, Setze setPump auf true.
        if (!setPumpStatus) { // Nur Status setzen, wenn notwendig
            Slave::setPump(true);

            setPumpStatus = true;
        }
    } else {
        if (setPumpStatus) { // Nur Status setzen, wenn notwendig
            // Wenn der Füllstand unter 30 sinkt, setze setPump auf false.
            Slave::setPump(false);

            setPumpStatus = false;
        }
    }
    if (percentIO < 50) {
        // Wenn der Füllstand unter 50 sinkt, setze setSlave1 und setSlave2 auf true.
        Slave::setSlave(0, true);
        Slave::setSlave(1, true);
    }
}

void Watcher::handleLevelSwitch() {
    level_state = digitalRead(LEVEL_SWITCH);

    if (!level_state) {
        // Disable all Pump3.
        if (setPumpStatus) { // Nur Status setzen, wenn notwendig
            Slave::setPump(false);

            setPumpStatus = false;
        }

        Slave::setError(true, "Tank is to empty (LEVEL SWITCH).");
    } else {
        if (!setPumpStatus && (percentIO > 30)) {  // Prüfen, ob die Pumpe wieder eingeschaltet werden kann
            Slave::setPump(true);

            setPumpStatus = true;

            Slave::setError(false);
        }
    }
}
