//
// Created by Jan Heil on 03.07.2023.
//

#include "TestCommand.h"
#include "Device.h"
#include "Slave.h"
#include "PINOUT.h"

TaskHandle_t task;

void TestCommand::execute(std::vector<String> argsIO) {
    // Print Debug Message.
    Device::println("Testing Device Buzzer...");

    xTaskCreate(Slave::runError, "test", 1000, NULL, 10, &task);

    delay(3000);

    if (task != NULL)
        vTaskDelete(task);

    digitalWrite(ERROR_LAMP, HIGH);
}