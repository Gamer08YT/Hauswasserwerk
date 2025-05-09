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

// Store Alarm States.
bool alarm_level = false;
bool alarm_moist = false;
bool ignoreMoist = false;

// Store IRMS.
float irmsIO = 0;

float distanceIO = 0;
int percentIO = 0;

float voltage_max = 1.90;
float voltage_min = 0.90;

bool fillIO = false;


int mixIO = 50;

int averageIO = 30;

int8_t ratioIO = 15;

/**
 * @brief Getter method for level switch of the Watcher class.
 *
 * This method returns the level switch of the Watcher class.
 * The level switch determines at which level the Watcher should take action.
 *
 * @return The current level switch value.
 */

bool Watcher::getLevelSwitch()
{
    alarm_level = digitalRead(LEVEL_SWITCH);

    return alarm_level;
}

/**
 * @defgroup Watcher Watcher
 * @brief Class responsible for setting up the Watcher module.
 *
 * The Watcher class is responsible for setting up the Watcher module. It
 * provides functions to initialize and configure the necessary components to
 * enable the monitoring of a specific system or application.
 */

void Watcher::setup()
{
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
void Watcher::handleConditions()
{
    // Check if Level is not Above Max value.
    // If True, disallow Pump1 and Pump2.
    if (percentIO < level_max && percentIO >= 0)
    {
        // Check if Level is not Below Min Value.
        // If True, disallow Pump3.
        if (percentIO > level_min)
        {
            // Reset Error Message.
            Slave::setError(false);

            if (fillIO)
            {
                if (percentIO >= (level_max - 5))
                {
                    // Update Display Fill-state.
                    Slave::infoDisplay("Füllstand:", "BEREIT");

                    // Stop Pump 1 - 2;
                    stopRefill();

                    // Set State of Process.
                    fillIO = false;
                }
                else
                {
                    // Start Pump 1 - 2;
                    refill();
                }
            }
            else if (percentIO < level_normal)
            {
                // Update Display Fill-state.
                Slave::infoDisplay("Füllstand:", "FÜLLEN...");

                // Set State of Process.
                fillIO = true;

                // Start Pump 1 - 2;
                refill();
            }

            // add moist check here
            bool stateIO = true;

            if (alarm_moist && !ignoreMoist)
                stateIO = false;

            // Allow Pump3 to Pump.
            Slave::setPump(stateIO);
        }
        else
        {
            // Disallow Pump3 to Pump.
            Slave::setPump(false);

            Slave::setError(true, "Füllstand zu niedrig.", false, "Füllstand < min");
        }
    }
    else
    {
        if (percentIO >= 0)
        {
            Slave::setError(true, "Füllstand zu hoch.", true, "Füllstand > max");
            Slave::setPump(true, true);
        }
        else
        {
            Slave::setError(true, "Sensor", true, "Ultraschall [X]");
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

void Watcher::loop()
{
    // Read Ultrasonic Sensor.
    if (timerIO.isReady())
    {
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

float Watcher::getPower()
{
    return (float)irmsIO * 230;
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

void Watcher::handleMeasurement()
{
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

void Watcher::setMax(int valueIO)
{
    if (valueIO <= 85)
    {
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

void Watcher::setMin(int valueIO)
{
    if (valueIO >= 25)
    {
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

void Watcher::setNormal(int valueIO)
{
    if (valueIO > level_min && valueIO < level_max)
    {
        level_normal = valueIO;
    }
}

/**
 * @brief Get the maximum level value.
 *
 * This function returns the value of the maximum level. The maximum level represents the highest level that can be measured.
 *
 * @return The maximum level value.
 */
int Watcher::getMax()
{
    return level_max;
}

/**
 * @brief Retrieves the minimum water level value.
 *
 * This function returns the stored minimum water level value.
 *
 * @return The minimum water level value.
 */
int Watcher::getMin()
{
    return level_min;
}

/**
 * @brief Get the normal level.
 *
 * This function returns the normal level of the watcher.
 *
 * @return The normal level.
 */
int Watcher::getNormal()
{
    return level_normal;
}


/**
 * @brief Getter method for the ignoreMoist property of the Watcher class.
 *
 * This method returns the value of the ignoreMoist property of the Watcher class.
 * The ignoreMoist property determines whether the Watcher should ignore moisture readings.
 * If ignoreMoist is set to true, the Watcher will not take action based on moisture levels.
 * If ignoreMoist is set to false, the Watcher will take action based on moisture levels.
 *
 * @return The current value of the ignoreMoist property.
 */
bool Watcher::getIgnoreMoist()
{
    return ignoreMoist;
}

/**
 * @brief Setter method for ignoreMoist property of the Watcher class.
 *
 * This method allows to set the value of the ignoreMoist property.
 * If ignoreMoist is set to true, the Watcher class will ignore moisture levels.
 *
 * @param valueIO The new value for the ignoreMoist property.
 */
void Watcher::setIgnoreMoist(bool valueIO)
{
    ignoreMoist = valueIO;
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
void Watcher::readCurrent()
{
    float averageIO = 0;

    // Read 10 Values to calculate Average.
    for (float counter = 0; counter < 5; counter++)
    {
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

float Watcher::getLevelDistance()
{
    return percentIO;
}

/**
 * @brief Refills the water tank by activating Pump1 or Pump2.
 *
 * This function activates either Pump1 or Pump2 to refill the water tank.
 */
void Watcher::refill()
{
    // Get Current Position and Check if Position is in Ratio Range.

    // Percent of current position eq. 50%
    // Pump0 eq 50% - 65%
    // Pump1 eq. 65% - 80%

    // add moist check here
    if (alarm_moist && !ignoreMoist)
    {
        // Fast disable Slaves.
        Slave::setSlave(0, false);
        Slave::setSlave(1, false);
    }
    else
    {
        if (percentIO <= (getMin() + ratioIO))
        {
            // Activate Pump1
            Slave::setSlave(0, true);
        }
        else
        {
            // Activate Pump2
            Slave::setSlave(1, true);
        }
    }
}

/**
 * @brief Returns the current distance value.
 *
 * This function returns the current distance value that is stored in the `distanceIO` variable.
 * The distance value is a floating point number.
 *
 * @return The current distance value.
 */
float Watcher::getDistance()
{
    return distanceIO;
}

/**
 * @brief Stops the refill process by disabling Pump1 and Pump2.
 *
 * This function sets the state of both Pump1 and Pump2 to false,
 * effectively turning them off and stopping the refill process.
 *
 * @note This function internally calls the static function `setSlave` to
 * set the state of the slaves.
 *
 * @see setSlave
 */
void Watcher::stopRefill()
{
    // Disable Pump1 or Pump2.
    Slave::setSlave(0, false);
    Slave::setSlave(1, false);
}

/**
 * @brief Sets the minimum voltage input value.
 *
 * This function sets the value of voltage_min, which represents the minimum voltage input.
 *
 * @param voltageIO The minimum voltage input value to be set.
 */
void Watcher::setMinV(float voltageIO)
{
    voltage_min = voltageIO;
}

/**
 * @brief Sets the maximum voltage value.
 *
 * This function sets the maximum voltage value that can be measured by the Watcher class.
 *
 * @param voltageIO The maximum voltage value to be set.
 */
void Watcher::setMaxV(float voltageIO)
{
    voltage_max = voltageIO;
}

/*void Watcher::setupTask() {
    // Create new Task for Measurements.
    xTaskCreate(runMeasurements, "measurements", 10000, NULL, 50, &measurements_task);
}*/

/**
 * @brief Sets the ratio input and calculates average and mix ratios.
 * @param int8 The ratio input value.
 */
void Watcher::setRatio(int8_t int8)
{
    ratioIO = int8;

    // Max eq. 80%
    // Min eq. 50%
    // Average = Fill Area eq. 30%
    averageIO = getMax() - getMin();

    // Calculate Fill Ratio.
    // Ratio eq. 50%
    // Mix = (average / 100) * ratioIO eq. (30% / 100) * 50 = 15%
    mixIO = (averageIO / 100) * ratioIO;

    // Calculate Ratio for Pump 1
    //pump1 = averageIO - mixIO;
}

/**
 * @brief Reads the level alarm status.
 *
 * This function reads the level alarm status by reading the digital pin LEVEL_ALARM.
 *
 * @return true if the level alarm is raised, false otherwise.
 */
bool Watcher::readLevelAlarm()
{
    alarm_moist = digitalRead(LEVEL_ALARM);

    return alarm_moist;
}
