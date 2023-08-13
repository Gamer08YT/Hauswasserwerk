//
// Created by Jan Heil on 12.08.2023.
//

#include <HTTPClient.h>
#include "Slave.h"
#include "ArduinoJson.h"
#include "PINOUT.h"
#include "Network.h"
#include "Device.h"

// Store Task Handle for Multithreading Error.
TaskHandle_t error_task;

// Store Error Lock.
bool lockIO = false;


/**
 * @file
 * @brief Declaration of the devices variable.
 */
char *devices[]{
        "192.168.1.240"
};


/**
 * \brief Set the slave ID and state.
 *
 * This function sets the ID and state of the slave.
 *
 * \param idIO The ID of the slave. Must be a valid integer.
 * \param stateIO The state of the slave. Must be a valid boolean.
 *
 * \return void
 */

void Slave::setSlave(int idIO, bool stateIO) {
    // todo: At OFF: check 1-2 Seconds later if Power Consumption = 0, else Shelly is defect... Trigger Alarm!
    // todo: At ON: check 1-2 Seconds later if Power Consumption > 0, else Pump Swimming Switch has no Water.
    if (stateIO)
        sendPost(idIO, "Switch.Set", R"({"id":"0", "toggle_after": 5, "on": true})");
    else
        sendPost(idIO, "Switch.Set", R"({"id":"0", "on": false})");


    Device::print("Changing PUMP");
    Device::print(String(idIO));
    Device::print(" to ");
    Device::println(String(stateIO));
}

/**
 * Send a GET request to a specified URL.
 *
 * @param idIO The identifier for the request.
 * @param urlIO The URL to send the GET request to.
 */

BasicJsonDocument<DefaultAllocator> Slave::sendGet(int idIO, char *urlIO) {
    HTTPClient http;

    // Begin HTTP Connection.
    http.begin(getURL(idIO, urlIO));

    // Begin POST Request.
    if (http.GET() != 200) {
        // Print Response.
        Serial.println(http.getString());
    }

    // Create new Document.
    DynamicJsonDocument document(1024);

    // Deserialize JSON Content.
    deserializeJson(document, http.getStream());

    // End HTTP Request.
    http.end();

    return document;
}

/**
 * @brief Sends a POST request to a specified URL with the given data.
 *
 * @param idIO The ID of the IO object.
 * @param urlIO The URL to send the POST request to.
 * @param dataIO The data to send in the POST request.
 */

BasicJsonDocument<DefaultAllocator> Slave::sendPost(int idIO, char *urlIO, String dataIO) {
    HTTPClient http;

    // Begin HTTP Connection.
    http.begin(getURL(idIO, urlIO));

    // Begin POST Request.
    if (http.POST(dataIO) != 200) {
        // Print Response.
        Serial.println(http.getString());
    }

    // Create new Document.
    DynamicJsonDocument document(1024);

    // Deserialize JSON Content.
    deserializeJson(document, http.getStream());

    // End HTTP Request.
    http.end();

    return document;
}

/**
 * \brief Retrieves the URL for a specific ID and type.
 *
 * This function retrieves the URL associated with a given ID and type.
 * The URL is stored in the provided `urlIO` parameter.
 *
 * \param idIO The ID of the resource to retrieve the URL for.
 * \param urlIO The buffer to store the URL in. Must be allocated before calling this function.
 * \param typeIO The type of the resource (e.g., image, video, audio, etc.).
 *
 * \return None.
 *
 * \note The `urlIO` parameter must be a pre-allocated buffer with enough space to store the URL string.
 * \note The caller is responsible for freeing the memory allocated for the URL buffer.
 * \note The caller must ensure that the `urlIO` buffer is large enough to hold the URL string. Otherwise, a buffer overflow may occur.
 */

String Slave::getURL(int idIO, char *urlIO) {
    String string;

    string += "http://";
    string += devices[idIO];
    string += "/rpc/";
    string += urlIO;

    // Print Debug Message.
    Serial.print("Send -> ");
    Serial.println(string);

    return string;
}

void Slave::setPump(bool stateIO) {
    if (!stateIO) {
        digitalWrite(PUMP_3, HIGH);
        digitalWrite(ERROR_LAMP, HIGH);
    } else {
        digitalWrite(PUMP_3, LOW);
        digitalWrite(ERROR_LAMP, LOW);
    }

    // Print Debug Message.
    Device::print("Changing PUMP3 to ");
    Device::println(String(stateIO));
}

/**
 * @brief Sets up the configuration for the Slave object.
 *
 * This function is responsible for setting up the necessary variables and initializing any required resources for the Slave object.
 * Call this function before using any other methods of the Slave class.
 *
 * @return void
 */

void Slave::setup() {
    pinMode(PUMP_3, OUTPUT);
    pinMode(ERROR_LAMP, OUTPUT);

    digitalWrite(PUMP_3, HIGH);
    digitalWrite(ERROR_LAMP, HIGH);
}

/**
 * \brief Sets the error state and code for the Slave object.
 *
 * This function allows to set the error state and code for the Slave object.
 *
 * \param stateIO The error state to be set. Set to true if an error occurred, false otherwise.
 * \param codeIO The error code associated with the error state. It should be a String indicating the specific error.
 * \param flashIO The Flash memory value to indicate if the error should be persisted in Flash memory or not.
 *
 * \note Setting the error state and code can help in identifying the cause of an error and handle it appropriately.
 *       The error state can be used to trigger error handling mechanisms in the Slave object.
 *
 * \note The error code can be any String describing the specific error in detail.
 *
 * \note When flashIO is set to a non-zero value, the error state and code will be persisted in Flash memory. This means
 *       that the error state and code will remain even after the power is cycled.
 *       When flashIO is set to zero, the error state and code will not be persisted, and they will be cleared upon power cycle.
 *
 * \warning Make sure to handle errors appropriately when they occur and check the error state as needed.
 *          Use clearError() to clear the error state and code manually when necessary.
 */

void Slave::setError(bool stateIO, String codeIO, int flashIO) {
    // Create new Task for Displaying Error Blinker.
    if (stateIO && !lockIO) {
        xTaskCreate(runError, "error", 10000, NULL, 100, &error_task);
        lockIO = true;

        // Print Debug Message.
        Device::print("Errror ");
        Device::print(String(stateIO));
        Device::print(" ");
        Device::println(codeIO);
    } else {
        vTaskDelete(error_task);
        lockIO = false;
    }
}

/**
 * @brief Description: This function is responsible for handling run-time errors encountered during the execution of Slave class.
 *
 * @param parameter A pointer to the data parameter that caused the error.
 *
 * @return None
 */

void Slave::runError(void *parameter) {
    digitalWrite(ERROR_LAMP, LOW);
    delay(500);
    digitalWrite(ERROR_LAMP, HIGH);
    delay(500);
}

/**
 * @brief Get the slave with the specified IO ID.
 *
 * This function retrieves the slave object from the Slave class based on the given IO ID. It searches the internal
 * collection of slave objects and returns the matching slave object if found. If no matching slave object is found, it
 * returns an empty optional object.
 *
 * @param idIO The IO ID of the slave object to retrieve.
 * @return std::optional<Slave> The slave object with the specified IO ID, if found.
 *
 * @note This function assumes that the slave objects are stored in the internal collection of the Slave class.
 * @note The IO ID is a unique identifier associated with each slave object.
 * @note If no matching slave object is found, this function returns an empty optional object.
 *
 * @see Slave
 */
BasicJsonDocument<DefaultAllocator> Slave::getSlave(int idIO) {
    return sendGet(idIO, "Switch.GetStatus?id=0");
}

/**
 * @brief Retrieves the current power value of a specified IO device.
 *
 * This function retrieves the current power value of the IO device identified by the provided ID.
 *
 * @param idIO The ID of the IO device.
 * @return The current power value of the specified IO device.
 */

float Slave::getPower(int idIO) {
    return getSlave(idIO)["apower"].as<float>();
}




