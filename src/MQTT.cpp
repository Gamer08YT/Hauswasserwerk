//
// Created by Jan Heil on 09.08.2023.
//

#include "MQTT.h"
#include "Slave.h"
#include "Watcher.h"

// Task handle for controlling.
TaskHandle_t testTaskHandle = NULL;

void MQTT::onSmart(bool state, HASwitch* sender)
{
    Serial.println("SMART VALUE CHANGED");

    sender->setState(state);
}

void MQTT::onNormal(HANumeric numberIO, HANumber* sender)
{
    Watcher::setNormal(numberIO.toInt8());

    sender->setState(Watcher::getNormal());
}

void MQTT::onMax(HANumeric numberIO, HANumber* sender)
{
    Watcher::setMax(numberIO.toInt8());

    sender->setState(Watcher::getMax());
}

void MQTT::onMin(HANumeric numberIO, HANumber* sender)
{
    Watcher::setMin(numberIO.toInt8());

    sender->setState(Watcher::getMin());
}

void MQTT::minV(HANumeric numberIO, HANumber* senderIO)
{
    Watcher::setMinV(numberIO.toFloat());

    senderIO->setState(numberIO);
}

void MQTT::maxV(HANumeric numberIO, HANumber* senderIO)
{
    Watcher::setMaxV(numberIO.toFloat());

    senderIO->setState(numberIO);
}

void MQTT::onRatio(HANumeric numberIO, HANumber* sender)
{
    Watcher::setRatio(numberIO.toInt8());

    sender->setState(numberIO);
}

void testTask(void* pvParameters)
{
    (void)pvParameters; // Prevent unused variable warning

    // Enable Buzzer.
    Slave::setErrorState(true);

    // Wait for 2 seconds without blocking
    vTaskDelay(pdMS_TO_TICKS(2000)); // 2000 milliseconds delay

    // Disable Buzzer.
    Slave::setErrorState(false);

    // Optionally, delete the task if no longer needed
    vTaskDelete(NULL);
}

void MQTT::onBuzzer(HAButton* senderIO)
{
    // Create the GPIO task
    xTaskCreate(
        testTask, // Task function
        "tT", // Name of the task (for debugging purposes)
        128, // Stack size (in words, not bytes)
        NULL, // Task input parameter
        1, // Priority of the task
        &testTaskHandle // Task handle
    );
}

/**
 * Sets the ignore moisture state for the MQTT device.
 *
 * @param state The new ignore moisture state.
 * @param sender The sender of the event.
 */
void MQTT::onIgnore(bool state, HASwitch* sender)
{
    Watcher::setIgnoreMoist(state);
}
