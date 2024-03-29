//
// Created by Jan Heil on 12.08.2023.
//

#include <HTTPClient.h>
#include <ETH.h>
#include "Slave.h"
#include "ArduinoJson.h"
#include "PINOUT.h"
#include "Device.h"
#include "Adafruit_SSD1306.h"
#include "SimpleTimer.h"
#include "ESPNtpClient.h"

// Store Error Lock.
bool lockIO = false;

// Store Error Message.
String error_message = "";

// Store Display Message.
String display_message, display_title = "";

bool display_state = true;
bool display_button = false;

// Timer for DIM OLED:
SimpleTimer dimIO(5000);

// Timer for NTP Update.
SimpleTimer ntpupdateIO(30000);

// Timer for disabling OLED.
SimpleTimer disableIO(60000);

// Handle Shelly Check Timer.
//SimpleTimer shellyupdate(15000);

// Store OLED Instance.
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int disabledIO = -1;

int contrast = 128;

bool states[] = {
        false,
        false,
        false
};

String display_updates[] = {

};

// https://rickkas7.github.io/DisplayGenerator/index.html
const unsigned char logo[] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x1f, 0x0c, 0x3c, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xfc,
                                      0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x07, 0xdf,
                                      0xff, 0xfc, 0x00, 0x07, 0xdf, 0xff, 0xfc, 0x00, 0x07, 0xc0, 0x7f, 0x80, 0x00,
                                      0x07, 0xc0, 0x3f, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xff,
                                      0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xe0, 0x07,
                                      0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff,
                                      0xe0, 0x07, 0xc0, 0x3f, 0x1f, 0xe0, 0x07, 0xc0, 0x3f, 0x0f, 0xe0, 0x07, 0xc0,
                                      0x00, 0x0f, 0xe0, 0x07, 0xc0, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xe0,
                                      0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00,
                                      0x03, 0xc0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00,
                                      0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x07,
                                      0xe0, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00};
/**
 * @file
 * @brief Declaration of the devices variable.
 * @see pump0 => pump1 => Brunnen
 * @see pump1 => pump2 => Zisterne
 */
char *devices[]{
        "192.168.1.240",
        "192.168.1.241"
};


TaskHandle_t ntp_task;

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
void Slave::setSlave(int idIO, bool stateIO, int timeIO) {
    // todo: At OFF: check 1-2 Seconds later if Power Consumption = 0, else Shelly is defect... Trigger Alarm!
    // todo: At ON: check 1-2 Seconds later if Power Consumption > 0, else Pump Swimming Switch has no Water.
    if (stateIO) {
        if (!lockIO) {
            String contentIO;

            contentIO += R"({"id":"0", "toggle_after": )";
            contentIO += timeIO;
            contentIO += R"(, "on": true})";

            sendPost(idIO, "Switch.Set", contentIO);
        } else {
            Device::println("PUMP1 and PUMP2 Locked!");
        }
    } else {
        sendPost(idIO, "Switch.Set", R"({"id":"0", "on": false})");
    }

    // Set State Boolean.
    if (stateIO && !lockIO)
        states[idIO] = stateIO;

    /*if (!lockIO || !stateIO) {
        Serial.print("Changing PUMP");
        Serial.print(String(idIO));
        Serial.print(" to ");
        Serial.println(String(stateIO));
    }*/
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
    http.useHTTP10(true);
    http.begin(getURL(idIO, urlIO));

    // Begin POST Request.
    if (http.GET() != 200) {
        // Print Response.
        //Serial.println(http.getString());
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

    http.useHTTP10(true);

    // Begin HTTP Connection.
    http.begin(getURL(idIO, urlIO));

    // Begin POST Request.
    if (http.POST(dataIO) != 200) {
        // Print Response.
        //Serial.println(http.getString());
    }

    // Device::println(http.getString());

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

void Slave::setPump(bool stateIO, bool forceIO) {
    if (!stateIO) {
        digitalWrite(PUMP_3, HIGH);
    } else {
        if (!lockIO || forceIO) {
            digitalWrite(PUMP_3, LOW);
        }
    }

    states[2] = stateIO;
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
    // Setup IC2 Bus.
    Wire.begin(SDA, SCL);

    pinMode(UNLOCK_BUTTON, INPUT_PULLUP);
    pinMode(PUMP_3, OUTPUT);
    pinMode(ERROR_LAMP, OUTPUT);

    digitalWrite(PUMP_3, HIGH);
    digitalWrite(ERROR_LAMP, HIGH);

    // Display Setup.
    if (!oled_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        setError(true, "Display Error");
    } else {

        // Set Default Size and disable Wrap.
        oled_display.setTextSize(1.5);
        oled_display.setTextColor(WHITE);
        oled_display.ssd1306_command(SSD1306_SETPRECHARGE);
        oled_display.ssd1306_command(17);

        // On NTP Event.
        /*NTP.onNTPSyncEvent([](NTPSyncEvent_t errorIO) {
            if (errorIO) {
                Serial.print("Time Sync error: ");
                if (errorIO == noResponse)
                    Slave::infoDisplay("NTP", "CONNECTION ERROR");
                else if (errorIO == invalidAddress)
                    Slave::infoDisplay("NTP", "INVALID ADDRESS");
            } else {
                Serial.print("Got NTP time: ");
                Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
            }
        });*/
    }
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

void Slave::setError(bool stateIO, String codeIO, bool flashIO, String displayIO) {
    // Create new Task for Displaying Error Blinker.
    if (stateIO) {
        if (!lockIO || codeIO != error_message) {
            // Print Debug Message.
            error_message = codeIO;
            Device::print("Error: ");
            Device::print(String(stateIO));
            Device::print(" ");
            Device::println(codeIO);

            if (!lockIO && flashIO) {
                //xTaskCreate(runError, "error", 10000, NULL, 100, &error_task);
            }

            lockIO = true;

            // Enable Display before Printing Error Message.
            setDisplayActive();

            // Display Error Message on OLED Display.
            infoDisplay("Fehler", displayIO);
        }
    } else {
        if (lockIO) {
            lockIO = false;
            error_message = "";

            // Get Task by Handle.
            /*TaskHandle_t taskIO = xTaskGetHandle("error");

            if (taskIO != NULL)
                //vTaskDelete(error_task);

                // Disable Error Lamp...
                digitalWrite(ERROR_LAMP, HIGH);*/
        }
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
    while (true) {
        digitalWrite(ERROR_LAMP, LOW);
        delay(1500);
        digitalWrite(ERROR_LAMP, HIGH);
        delay(1500);
    }
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
    auto slaveIO = getSlave(idIO);

    if(slaveIO == nullptr)
        return 0.0f;

    if(!slaveIO["apower"].isNull())
        return 0.0f;

    if(!slaveIO["apower"].is<float>()) {
        // handle error
        return 0.0f;
    }


    // Warning
    CRASH

    //getSlave(i)
    return slaveIO["apower"].as<float>();
}

String Slave::getError() {
    return error_message;
}

bool Slave::getState(int idIO) {
    return states[idIO];
}

/*
void Slave::setDisplay(String messageIO) {
    // Avoid to much Calculation.
    if (messageIO != display_message) {
        oled_display.clearDisplay();
        oled_display.setCursor(0, 0);
        oled_display.printUTF8(const_cast<char *>(messageIO.c_str()));
        oled_display.display();

        display_message = messageIO;
    }
}
*/

void Slave::showBootscreen() {
    // Show initial display buffer contents on the screen --
    oled_display.clearDisplay();
    oled_display.drawBitmap(44, 5, logo, 40, 40, 1);
    oled_display.setTextWrap(0);
    oled_display.setCursor(34, 48);
    oled_display.println("Wasserwerk");
    oled_display.display();
    oled_display.setTextWrap(true);

    delay(2500);

    // Set Contrast.
    setContrast(128);
}

/**
 * Display Info Page on OLED.
 * @param titleIO
 * @param contentIO
 */
void Slave::infoDisplay(const char *titleIO, String contentIO, bool forceIO) {
    if (display_message != contentIO && display_title != titleIO) {
        if (display_state && (!display_button || forceIO)) {
            oled_display.clearDisplay();
            oled_display.setCursor(0, 0);
            oled_display.printlnUTF8(const_cast<char *>(titleIO));
            oled_display.drawLine(0, 15, 128, 15, 1);
            oled_display.printlnUTF8(const_cast<char *>(contentIO.c_str()));
            oled_display.display();

            display_message = contentIO;
            display_title = titleIO;

            // Reset Updates Array.
            std::fill_n(display_updates, sizeof(display_updates), 0);
            Wire.flush();
        }
    }
}

/**
 * \brief Update a line in the Slave class.
 *
 * This function updates a line in the Slave class by replacing its content and/or modifying its position.
 *
 * \param contentIO The content of the line to be updated.
 * \param xIO The new x-coordinate position of the line.
 * \param yIO The new y-coordinate position of the line.
 * \param forceIO Determines whether to force the update regardless of the line's current content and position.
 *              Set to true to force the update, false otherwise.
 *
 * \note The line to be updated must already exist in the Slave class. If forceIO is set to false, the update
 *       will only occur if the line's content or position has changed. If forceIO is set to true, the update
 *       will be performed regardless of the line's current state.
 *
 * \return void
 */

void Slave::updateLine(String contentIO, int xIO, int yIO, bool forceIO) {
    if (display_state && (!display_button || forceIO)) {
        if (display_updates[yIO - xIO] != contentIO) {
            // Override Line.
            oled_display.fillRect(xIO, yIO, 128, 14, BLACK);
            oled_display.setCursor(xIO, yIO);
            oled_display.display();

            // Print new Data.
            oled_display.printlnUTF8(const_cast<char *>(contentIO.c_str()));

            oled_display.display();
            display_updates[yIO - xIO] = contentIO;
            Wire.flush();
        }
    }
}

void Slave::loop() {
    // Dim Display if Time is exceeded.
    if (dimIO.isReady()) {
        setContrast(8);
        oled_display.display();
    }

    // Disable OLED.
    if (disableIO.isReady()) {
        display_state = false;
        //oled_display.clearDisplay();
        oled_display.ssd1306_command(SSD1306_DISPLAYOFF);
    }

    // Check if Pump is realy Active.
    /*if (shellyupdate.isReady()) {
        checkSlaveState(0);
        checkSlaveState(1);
    }*/

    // Read Unlock Button.
    display_button = !digitalRead(UNLOCK_BUTTON);

    // Unlock Error or Un-dim Display.
    if (display_button) {
        setDisplayActive();

        // Update Display Message.
        //infoDisplay("Button", "CLICKED", true);
        //updateLine(ETH.localIP().toString(), 0, 32, true);
    }

    if (ntpupdateIO.isReady()) {
        //updateLine(NTP.getTimeStr(), 0, 48, false);

        ntpupdateIO.reset();
    }
}

void Slave::setContrast(int valueIO) {
    if (valueIO != contrast) {
        oled_display.ssd1306_command(SSD1306_SETCONTRAST);
        oled_display.ssd1306_command(valueIO);

        contrast = valueIO;
    }
}

void Slave::setDisplayActive() {
    dimIO.reset();
    disableIO.reset();
    display_state = true;
    oled_display.ssd1306_command(SSD1306_DISPLAYON);
    setContrast(128);
}

void Slave::ntp() {
    NTP.setTimeZone(TZ_Europe_Berlin);
    NTP.begin();

    // Set Display Message.
    Slave::infoDisplay("NTP", "SYNCED");
}

void Slave::checkSlaveState(int idIO) {
    float slave = Slave::getPower(idIO);

    // Error when Shelly / Pump not pumps.
    if (states[idIO] && slave < MIN_SLAVE_POWER) {
        setError(true, "Slave is empty.", false, "Slave [E]");
    }

    // Error when Shelly Contactor not closing.
    if (!states[idIO] && slave > MAX_SLAVE_DISABLED) {
        setError(true, "Slave has glued Contacts.", false, "Slave [C]");
    }
}

/**
 * Set the disabled status for a specific slave.
 *
 * The disabled status indicates whether a slave is disabled or not.
 * A disabled slave will not perform any actions and its state will not be updated.
 *
 * @param slaveIO The ID of the slave for which to set the disabled status.
 */
void Slave::setDisabled(int slaveIO) {
    disabledIO = slaveIO;
}

/**
 * @brief Handles the disable status of a slave IO device.
 *
 * This function checks the power consumption of a slave IO device and sets the `disabledIO` variable if the power consumption is below 10.
 *
 * @param slaveIO The ID of the slave IO device.
 * @return None.
 */
void Slave::handleDisableStatus(int slaveIO) {
    // Get Power consumption of Slave.
    float powerIO = Slave::getPower(slaveIO);

    if(powerIO < 10) {
        disabledIO = slaveIO;
    }
}

/**
 * @brief Check if a specific slave is disabled.
 *
 * This function checks whether a specific slave, identified by `slaveIO`,
 * is disabled. It compares the value of `slaveIO` with the global variable
 * `disabledIO`. If `slaveIO` is equal to `disabledIO`, then the slave is
 * considered disabled and the function returns true. Otherwise, it returns false.
 *
 * @param slaveIO The ID of the slave to check.
 * @return true if the slave is disabled, false otherwise.
 */
bool Slave::isDisabled(int slaveIO) {
    return (slaveIO == disabledIO);
}




