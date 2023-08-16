//
// Created by Jan Heil on 12.08.2023.
//

#include <HTTPClient.h>
#include "Slave.h"
#include "ArduinoJson.h"
#include "PINOUT.h"
#include "Network.h"
#include "Device.h"
#include "Adafruit_SSD1306.h"
#include "SimpleTimer.h"

// Store Task Handle for Multithreading Error.
TaskHandle_t error_task;

// Store Error Lock.
bool lockIO = false;

// Store Error Message.
String error_message = "";

// Store Display Message.
String display_message, display_title = "";

bool display_state = true;

// Timer for DIM OLED:
SimpleTimer dimIO(5000);

// Timer for disablind OLED.
SimpleTimer disableIO(15000);

// Store OLED Instance.
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int contrast = 128;

bool states[] = {
        false,
        false,
        false
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
    if (stateIO) {
        if (!lockIO) {
            //sendPost(idIO, "Switch.Set", R"({"id":"0", "toggle_after": 5, "on": true})");
        } /*else {
            Device::println("PUMP1 and PUMP2 Locked!");
        }*/
    } else {
        //sendPost(idIO, "Switch.Set", R"({"id":"0", "on": false})");
    }

    // Set State Boolean.
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
    } else {
        if (!lockIO) {
            digitalWrite(PUMP_3, LOW);
        } /*else
            Device::println("PUMP3 Locked!");*/
    }

    states[2] = stateIO;

    /*if (!lockIO || !stateIO) {
        // Print Debug Message.
        Serial.print("Changing PUMP3 to ");
        Serial.println(String(stateIO));
    }*/
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
    }

    // Set Default Size and disable Wrap.
    oled_display.setTextSize(1.5);
    oled_display.setTextColor(WHITE);
    oled_display.ssd1306_command(SSD1306_SETPRECHARGE);
    oled_display.ssd1306_command(17);
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
                xTaskCreate(runError, "error", 10000, NULL, 100, &error_task);
            }

            lockIO = true;

            // Display Error Message on OLED Display.
            infoDisplay("Fehler", displayIO);
        }
    } else {
        if (lockIO) {
            lockIO = false;
            error_message = "";

            // Get Task by Handle.
            TaskHandle_t taskIO = xTaskGetHandle("error");

            if (taskIO != NULL)
                vTaskDelete(error_task);

            // Disable Error Lamp...
            digitalWrite(ERROR_LAMP, HIGH);
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
    return getSlave(idIO)["apower"].as<float>();
}

String Slave::getError() {
    return error_message;
}

bool Slave::getState(int idIO) {
    return states[idIO];
}

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
void Slave::infoDisplay(const char *titleIO, String contentIO) {
    if (display_message != contentIO && display_title != titleIO) {
        if (display_state) {
            oled_display.clearDisplay();
            oled_display.setCursor(0, 0);
            oled_display.printlnUTF8(const_cast<char *>(titleIO));
            oled_display.drawLine(0, 15, 128, 15, 1);
            oled_display.printlnUTF8(const_cast<char *>(contentIO.c_str()));
            oled_display.display();
            delay(250);

            display_message = contentIO;
            display_title = titleIO;
        }
    }
}

void Slave::updateLine(String contentIO, int xIO, int yIO) {
    if (display_state) {
        // Override Line.
        oled_display.fillRect(xIO, yIO, 128, 12, BLACK);
        oled_display.setCursor(xIO, yIO);

        // Print new Data.
        oled_display.printlnUTF8(const_cast<char *>(contentIO.c_str()));

        oled_display.display();

        delay(250);
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
        oled_display.clearDisplay();
        oled_display.ssd1306_command(SSD1306_DISPLAYOFF);
    }

    // Read Unlock Button.
    bool stateIO = digitalRead(UNLOCK_BUTTON);

    // Unlock Error or Un-dim Display.
    if (!stateIO) {
        dimIO.reset();
        disableIO.reset();
        display_state = true;
        oled_display.ssd1306_command(SSD1306_DISPLAYON);
        setContrast(128);

        // Update Display Message.
        infoDisplay("Button", "CLICKED");
    }
}

void Slave::setContrast(int valueIO) {
    if (valueIO != contrast) {
        oled_display.ssd1306_command(SSD1306_SETCONTRAST);
        oled_display.ssd1306_command(valueIO);

        contrast = valueIO;
    }
}





