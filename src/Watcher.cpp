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


TaskHandle_t measurements_task;
// Timer for Measuring Updates.
SimpleTimer timerIO(1000);

// Store Energy Monitor Instance.
EnergyMonitor monitor;

// Store max, min and normal Water Level.
int level_min = 30;
int level_max = 80;
int level_normal = 50;

// Store IRMS.
float irmsIO = 0;

float distanceIO = 0;
int percentIO = 0;

float voltage_max = 1.90;
float voltage_min = 0.90;

bool fillIO = false;


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
    pinMode(LEVEL_FILL, INPUT);
    pinMode(METER, INPUT);
    pinMode(LEVEL_ALARM, INPUT);

    // Setup Monitor.
    monitor.current(METER, 25);
}

/**
 * @brief Handles the conditions and controls the pumps based on the water level.
 *
 * This function is responsible for checking the water level conditions and controlling the pumps accordingly. It checks if the water level is within the acceptable range and takes appropriate
* actions. It also updates the display with the current fill state and updates the fill level.
 */
void Watcher::handleConditions() {
    // Check if Level is not Above Max value.
    // If True, disallow Pump1 and Pump2.
    if (percentIO < level_max && percentIO >= 0) {

        // Check if Level is not Below Min Value.
        // If True, disallow Pump3.
        if (percentIO > level_min) {

            // Reset Error Message.
            Slave::setError(false);

            if (fillIO) {
                if (percentIO >= (level_max - 5)) {
                    // Update Display Fill-state.
                    Slave::infoDisplay("Füllstand:", "BEREIT");

                    // Stop Pump 1 - 2;
                    stopRefill();

                    // Set State of Process.
                    fillIO = false;
                } else {
                    // Start Pump 1 - 2;
                    refill();
                }
            } else if (percentIO < level_normal) {
                // Update Display Fill-state.
                Slave::infoDisplay("Füllstand:", "FÜLLEN...");

                // Set State of Process.
                fillIO = true;

                // Start Pump 1 - 2;
                refill();
            }

            // Allow Pump3 to Pump.
            Slave::setPump(true);
        } else {
            // Disallow Pump3 to Pump.
            Slave::setPump(false);

            Slave::setError(true, "Füllstand zu niedrig.", false, "Füllstand < min");
        }
    } else {
        if (percentIO >= 0) {
            Slave::setError(true, "Füllstand zu hoch.", true, "Füllstand > max");
            Slave::setPump(true, true);
        } else {
            Slave::setError(true, "Ultraschall Fehler.", true, "Ultraschall [X]");
            Slave::setPump(false);
        }

        stopRefill();
    }

    String fill_line;
    fill_line += percentIO;
    fill_line += "%";

    // Update Fill Level.
    Slave::updateLine(fill_line, 0, 32);
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
    // Read Ultrasonic Sensor.
    if (timerIO.isReady()) {
        // Read Ultrasonic Sensor.
        readCurrent();

        // Handle Conditions.
        handleConditions();

        // Handle Measurements.
        handleMeasurement();

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
    // Calc only IRMS in A.
    irmsIO = (monitor.calcIrms(1480) / 1000);
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
    if (valueIO <= 85) {
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
    if (valueIO >= 25) {
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
* @brief Reads the current analog input value and calculates the distance and percentage based on the measured value.
*
* This function reads 10 analog input values from the LEVEL_FILL pin to calculate an average. It then converts
* the average from millivolts to volts and stores it in the distanceIO variable.
* The rangeIO is calculated based on the maximum and minimum voltage values.
* The positionIO is calculated as the difference between distanceIO and voltage_min.
* Finally, the percentIO variable is updated with the percentage value based on the positionIO and rangeIO.
*/
void Watcher::readCurrent() {
    float averageIO = 0;

    // Read 10 Values to calculate Average.
    for (float counter = 0; counter < 5; counter++) {
        averageIO = averageIO + analogReadMilliVolts(LEVEL_FILL);
    }

    // Calculate the Distance (mV to V).
    distanceIO = ((averageIO / 5) / 1000.0);

    // Calculate Range.
    double rangeIO = voltage_max - voltage_min;

    // Calculate Position with Offset.
    double positionIO = distanceIO - voltage_min;

    // Calculate max Percent.
    percentIO = (positionIO / rangeIO) * 100;
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
 * @brief Refills the water tank by activating Pump1 or Pump2.
 *
 * This function activates either Pump1 or Pump2 to refill the water tank.
 */
void Watcher::refill() {
    // Activate Pump1 or Pump2.
    Slave::setSlave(0, true);
    Slave::setSlave(1, true);
}

float Watcher::getDistance() {
    return distanceIO;
}

void Watcher::stopRefill() {
    // Disable Pump1 or Pump2.
    Slave::setSlave(0, false);
    Slave::setSlave(1, false);
}

void Watcher::setMinV(float voltageIO) {
    voltage_min = voltageIO;
}

void Watcher::setMaxV(float voltageIO) {
    voltage_max = voltageIO;
}

/*void Watcher::setupTask() {
    // Create new Task for Measurements.
    xTaskCreate(runMeasurements, "measurements", 10000, NULL, 50, &measurements_task);
}*/
