//
// Created by Jan Heil on 12.08.2023.
//

#include <HTTPClient.h>
#include "Slave.h"
#include <ArduinoJson.h>


WiFiClient client;

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
}

/**
 * Send a GET request to a specified URL.
 *
 * @param idIO The identifier for the request.
 * @param urlIO The URL to send the GET request to.
 */

int Slave::sendGet(int idIO, char *urlIO) {
    HTTPClient http;

    // Begin HTTP Connection.
    http.begin(client, getURL(idIO, urlIO, 0));

    // Begin POST Request.
    http.GET();

    // End HTTP Request.
    http.end();

    // Create new Document.
    DynamicJsonDocument document(2048);

    // Deserialize JSON Content.
    deserializeJson(document, http.getStream());
}

/**
 * @brief Sends a POST request to a given URL with provided data.
 *
 * @param idIO The ID of the post request.
 * @param urlIO The URL to send the post request to.
 * @param dataIO A character representing the data to be sent.
 */

BasicJsonDocument<DefaultAllocator> Slave::sendPost(int idIO, char *urlIO, char dataIO) {
    HTTPClient http;

    // Begin HTTP Connection.
    http.begin(client, getURL(idIO, urlIO, 1));

    // Begin POST Request.
    http.POST(dataIO);

    // End HTTP Request.
    http.end();

    // Create new Document.
    DynamicJsonDocument document(2048);

    // Deserialize JSON Content.
    deserializeJson(document, http.getStream());

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

String Slave::getURL(int idIO, char *urlIO, int typeIO) {
    String string;

    string += "http://";
    string += devices[idIO];
    string += "/rpc/HTTP.";

    if (typeIO == 0)
        string += "GET";
    else if (typeIO == 1)
        string += "POST";

    string += "?url=";
    string += "http://10.33.53.21/rpc/";
    string += urlIO;

    // Print Debug Message.
    Serial.print("Send -> ");
    Serial.println(string);

    return string;
}

