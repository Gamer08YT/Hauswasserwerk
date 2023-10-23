#define ARDUINOHA_DEBUG

#include <Arduino.h>
#include <ArduinoHA.h>
#include <MQTT.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ETH.h>
#include "Watcher.h"
#include "Network.h"
#include "Device.h"
#include "Slave.h"
#include "SimpleTimer.h"

//#include "ArduinoOTA.h"
//#include "InternalStorage.h"

// Store MAC Address of Device.
byte macIO[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};


// http://tomeko.net/online_tools/file_to_hex.php?lang=en
// https://gzip.swimburger.net/
const unsigned char indexIO[] = {
        0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x8D, 0x53, 0xDB, 0x6A, 0xDC, 0x30,
        0x10, 0xFD, 0x95, 0xA9, 0x9F, 0x5A, 0xA8, 0xEC, 0x6C, 0x36, 0x29, 0xA5, 0x78, 0x4D, 0xC9, 0x05,
        0x42, 0x2F, 0xB4, 0xD0, 0x40, 0xE9, 0xA3, 0x6C, 0xCD, 0xAE, 0x95, 0xD5, 0x0D, 0x69, 0xBC, 0x5E,
        0xFF, 0x58, 0x9F, 0xFA, 0x96, 0x1F, 0xEB, 0x68, 0xBD, 0x49, 0x08, 0xB4, 0xA5, 0x0F, 0xB6, 0xA5,
        0x99, 0xA3, 0xD1, 0x99, 0x73, 0xC6, 0xF5, 0x8B, 0xAB, 0x2F, 0x97, 0xB7, 0x3F, 0xBE, 0x5E, 0xC3,
        0xCD, 0xED, 0xE7, 0x4F, 0x4D, 0xDD, 0x93, 0x35, 0xA0, 0x24, 0x49, 0xD1, 0x26, 0x41, 0x3D, 0x5A,
        0x5C, 0x15, 0x4A, 0xC6, 0x6D, 0xC1, 0x29, 0x94, 0xAA, 0xA9, 0x2D, 0x92, 0x84, 0xAE, 0x97, 0x31,
        0x21, 0xAD, 0x8A, 0x81, 0xD6, 0xE2, 0x6D, 0x71, 0x8C, 0x3A, 0x99, 0xD1, 0x3B, 0x8D, 0x63, 0xF0,
        0x91, 0x0A, 0xE8, 0xBC, 0x23, 0x74, 0x8C, 0x1A, 0xB5, 0xA2, 0x7E, 0xA5, 0x70, 0xA7, 0x3B, 0x14,
        0x87, 0xCD, 0x6B, 0xD0, 0x4E, 0x93, 0x96, 0x46, 0xA4, 0x4E, 0x1A, 0x5C, 0x2D, 0xB8, 0x06, 0x69,
        0x32, 0xD8, 0x5C, 0x4C, 0x84, 0xDF, 0x65, 0x4A, 0x18, 0x47, 0x8C, 0xDB, 0xBA, 0x9A, 0xA3, 0xB5,
        0xD1, 0x6E, 0x0B, 0x11, 0xCD, 0xAA, 0x48, 0x34, 0x19, 0x4C, 0x3D, 0x22, 0xDF, 0xD0, 0x47, 0x5C,
        0xAF, 0x8A, 0x9E, 0x28, 0xBC, 0xAB, 0xAA, 0x4E, 0xB9, 0xF2, 0x2E, 0x29, 0x34, 0x7A, 0x17, 0x4B,
        0x87, 0x54, 0xB9, 0x60, 0xAB, 0xD6, 0x7B, 0x4A, 0x14, 0x65, 0x78, 0x7F, 0x5E, 0x2E, 0xCB, 0x45,
        0xA5, 0x74, 0xA2, 0xAA, 0x4B, 0xE9, 0x29, 0x51, 0x5A, 0xED, 0x4A, 0x8E, 0x30, 0x83, 0x6A, 0x6E,
        0xB1, 0xF5, 0x6A, 0x6A, 0x6A, 0xA5, 0x77, 0xD0, 0x19, 0x66, 0xB2, 0x2A, 0x72, 0x23, 0x52, 0x3B,
        0x8C, 0x60, 0x27, 0x71, 0x5E, 0x3C, 0xCB, 0xB5, 0x1B, 0x91, 0xF1, 0xD0, 0xFA, 0xA8, 0x18, 0x30,
        0x7F, 0x84, 0x92, 0xCC, 0x50, 0x41, 0x10, 0xE7, 0x10, 0x7C, 0xE2, 0x4E, 0xBD, 0x13, 0xCC, 0x5E,
        0x92, 0xDE, 0x21, 0x44, 0x3F, 0x38, 0x85, 0x8A, 0x73, 0x84, 0x7B, 0x12, 0x1D, 0x6B, 0xC4, 0x27,
        0x0F, 0x6B, 0x3B, 0x10, 0xAA, 0xAC, 0xF5, 0xE2, 0xA1, 0xFE, 0x21, 0x9C, 0x6F, 0x10, 0x68, 0x43,
        0x2F, 0x93, 0x66, 0xA2, 0xB3, 0x3E, 0x75, 0xDB, 0xCC, 0x12, 0xB5, 0x99, 0xF9, 0xA2, 0xA9, 0xC3,
        0x13, 0x5F, 0x23, 0xCC, 0x46, 0xBC, 0x01, 0xDB, 0x8A, 0x33, 0xB0, 0x7B, 0x21, 0x07, 0xF2, 0x45,
        0x73, 0x25, 0x13, 0x3C, 0x49, 0x0B, 0xAC, 0x04, 0xB4, 0x48, 0x51, 0x63, 0x9B, 0x5A, 0x8C, 0xA8,
        0x09, 0x98, 0x17, 0x28, 0x8D, 0x70, 0xAD, 0x5D, 0x22, 0x34, 0x66, 0x70, 0x1B, 0x74, 0xB0, 0xBD,
        0xFF, 0xE9, 0x1C, 0x7F, 0xEF, 0x7F, 0x31, 0x0C, 0x14, 0xAF, 0x2E, 0x06, 0x22, 0xEF, 0x18, 0xCE,
        0xFE, 0x02, 0x6A, 0x46, 0x25, 0xEC, 0x79, 0xC9, 0x75, 0x39, 0x5D, 0x42, 0xBE, 0xE9, 0xDB, 0xC4,
        0x25, 0x2C, 0x8C, 0x03, 0x87, 0x60, 0xC7, 0xE8, 0x0F, 0xD2, 0xC1, 0x0D, 0x6A, 0x03, 0x2F, 0xC7,
        0x71, 0x2C, 0x5B, 0xF6, 0x59, 0x24, 0xF2, 0x11, 0x4B, 0x85, 0xAF, 0x80, 0x35, 0x18, 0x75, 0xB7,
        0x45, 0x33, 0x93, 0x08, 0xD1, 0x6F, 0xA2, 0xB4, 0x56, 0x63, 0xA4, 0xB2, 0xAE, 0x42, 0x53, 0xCB,
        0xA3, 0xD9, 0x65, 0xC5, 0x6E, 0xAC, 0xF5, 0xA6, 0x78, 0x34, 0x80, 0x1C, 0xF0, 0x23, 0x42, 0xD4,
        0x56, 0xC6, 0x29, 0xF7, 0xCC, 0xA2, 0xEF, 0xD9, 0x27, 0x9A, 0x02, 0xCF, 0x63, 0x7B, 0xE0, 0x5A,
        0x34, 0x1F, 0x0F, 0xC7, 0x86, 0x28, 0xB3, 0x19, 0x75, 0x25, 0x59, 0x34, 0xB6, 0xF1, 0xE1, 0xFD,
        0x27, 0xB7, 0xD9, 0x87, 0x35, 0x4F, 0x09, 0xF7, 0x7C, 0xCC, 0x48, 0xA3, 0x37, 0x4E, 0x68, 0x6E,
        0x2B, 0x3D, 0xB8, 0xA6, 0xC4, 0xDA, 0xE0, 0xFE, 0xC1, 0x76, 0xF2, 0x01, 0xF2, 0x5E, 0x8C, 0x3C,
        0x58, 0x70, 0x37, 0x24, 0xD2, 0xEB, 0x49, 0x1C, 0xFF, 0x03, 0xC1, 0x62, 0x8F, 0xC8, 0x2A, 0xF1,
        0x10, 0x9D, 0x41, 0x98, 0xC4, 0xF2, 0xF9, 0x24, 0xFD, 0xBD, 0x7A, 0xF6, 0xD3, 0x2A, 0x71, 0x56,
        0x64, 0x19, 0x8E, 0x68, 0x6E, 0x73, 0x99, 0x7B, 0xE5, 0xF8, 0x09, 0x3C, 0x0D, 0x49, 0x42, 0xBE,
        0x4D, 0x65, 0x1D, 0x4C, 0x2F, 0x16, 0x60, 0x51, 0x9C, 0xCE, 0x59, 0xC5, 0x89, 0xB9, 0x77, 0xE1,
        0xBC, 0xC3, 0x62, 0x56, 0xB3, 0xCA, 0x53, 0xCF, 0x52, 0xA4, 0xC0, 0xE6, 0xFC, 0x67, 0xE5, 0xA2,
        0xB9, 0xF4, 0x61, 0x8A, 0x7A, 0xD3, 0xF3, 0xFC, 0x4C, 0x70, 0x7A, 0x72, 0xBA, 0xFC, 0xA7, 0xB7,
        0x75, 0x95, 0xAB, 0x3F, 0xCA, 0x3D, 0x4B, 0x7A, 0xDC, 0xFE, 0x06, 0x9F, 0x10, 0x07, 0x0B, 0x7F,
        0x04, 0x00, 0x00
};

// Store WebServer Instance.
AsyncWebServer server(80);

// Timer for HA Updates.
SimpleTimer updateIO(2500);

// Define Home Assistant Credentials.
HADevice device;

WiFiClient client;

// Define Home Assistant MQTT Instance.
HAMqtt mqtt(client, device, 14);

// Store Refill Trigger.
HANumber minIO("water_min", HABaseDeviceType::PrecisionP0);
HANumber maxIO("water_max", HABaseDeviceType::PrecisionP0);
HANumber normalIO("water_normal", HABaseDeviceType::PrecisionP0);

// Store States of Pumps.
HABinarySensor pump1("water_pump1");
HABinarySensor pump2("water_pump2");
HABinarySensor pump3("water_pump3");
HABinarySensor maxSwitch("water_max");

// Store Mixing Values.
HANumber mix1("water_mix1");
HANumber mix2("water_mix2");

// Store Error Message.
HASensor errorIO("water_error");

// Store Smart Mode (Pump when Cistern has too much water or from Well when not enough water in Cistern).
HASwitch smart("water_smart");

// Store Buffer Tank Level.
HASensorNumber buffer("water_buffer", HABaseDeviceType::PrecisionP0);

// Store Buffer Level Distance.
HASensorNumber distance("water_distance", HABaseDeviceType::PrecisionP0);

// Store Power Usage of Pump3.
HASensorNumber power("water_load", HABaseDeviceType::PrecisionP2);


void setupOTA();

void handleOTA();

void handleWeb();

void setupHA();

void setupMQTT();

void setupHTTP();

void setupThreads();

/**
 * @brief Initializes the variables and resources required for the setup.
 *
 * This function should be called once at the beginning of the program execution
 * to initialize the necessary variables and resources.
 *
 * @details
 * This function sets up any global variables and resources that are needed
 * before the main code execution starts.
 *
 * Example usage:
 * @code
 * setup();
 * @endcode
 *
 * @note
 * It is recommended to call this function from the main() function.
 */

void setup() {
    // Begin Serial Console.
    Serial.begin(115200);

    // Print Serial Start.
    Serial.println("Starting.");

    // Setup Pins.
    Watcher::setup();
    Slave::setup();

    // Display Bootscreen.
    Slave::showBootscreen();

    // Print Debug Message.
    Serial.println("Starting Hauswasserwerk...");

    // Set Display Message.
    Slave::infoDisplay("System", "BOOTING");

    // Setup LAN Connection.
    Network::setupLAN();

    // Setup Arduino OTA.
    setupOTA();

    // Setup HTTP Server.
    setupHTTP();

    // Setup NTP.
    Slave::ntp();

    // Register all Commands.
    //Device::addCommands();

    // Setup Telnet Server.
    //Device::beginTelnet();

    // Wait for a short Period.
    delay(250);

    // Setup HomeAssistant.
    setupHA();

    // Wait for a short Period.
    delay(500);

    // Setup MQTT.
    setupMQTT();

    // Setup NTP Task.
    //Slave::setupNTPTask();
}

void setupHTTP() {
    // Handle Index Query.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", indexIO, sizeof(indexIO));
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    // Begin WebServer.
    server.begin();
}

/**
 * @brief This function is called when a successful connection is established.
 *
 * This function is invoked when a connection to a remote device is successfully established.
 * It can be used to perform any necessary actions or setup required after a successful connection.
 *
 * @note This function should be implemented by the user.
 *
 * @param void No parameters are required for this function.
 *
 * @return void This function does not return any value.
 *
 * @see onDisconnected()
 */

void onConnected() {
    Serial.println("Connected to MQTT Server.");

    // Set Info Display.
    Slave::infoDisplay("MQTT", "VERBUNDEN");
}

/**
 * @brief Handles the received message from the specified topic.
 *
 * This function is called whenever a message is received on the specified topic.
 * It takes the received topic, payload, and payload length as input parameters.
 *
 * @param topic The topic on which the message was received.
 * @param payload The payload of the received message.
 * @param length The length of the payload in bytes.
 */

void onMessage(const char *topic, const uint8_t *payload, uint16_t length) {
    Serial.println("Received ");
    Serial.print(topic);
    Serial.println(" - ");
    //(payload);
}

/**
 * @brief Sets up the MQTT connection with the MQTT broker.
 *
 * This function initializes the necessary resources and establishes a
 * connection with the MQTT broker. It performs the configuration settings
 * required for a successful MQTT connection.
 *
 * @note The function assumes that the necessary parameters like broker URL,
 * credentials, client ID, etc. are already set before calling this function.
 * Ensure that the required parameters are correctly set prior to calling
 * this function.
 *
 * @see setBrokerURL()
 * @see setCredentials()
 * @see setClientID()
 * @see setKeepAlive()
 *
 * @return void
 */

void setupMQTT() {
    // Add MQTT Listener.
    mqtt.onMessage(onMessage);
    mqtt.onConnected(onConnected);

    // Connect to HomeAssistant.
    mqtt.begin("homeassistant.local", "wasserwerk", "wasserwerk");

    // Print Debug Message.
    Serial.println("Connecting to HomeAssistant MQTT Server.");
}


/**
 * @brief Initial setup for Home Automation
 *
 * This function sets up the necessary variables and configurations for Home Automation.
 * It initializes the required devices, sensors, and controllers.
 * After calling this function, the home automation system is ready to be used.
 *
 * @details
 * The setupHA() function performs the following tasks:
 * 1. Initializes the communication protocols and configurations for controlling devices.
 * 2. Configures and connects to sensors for collecting data.
 * 3. Initializes and configures the Home Automation Controller.
 * 4. Sets up the necessary event handlers for monitoring and responding to events.
 *
 * @note
 * This function should be called once at the beginning of the Home Automation system.
 * After calling this function, the main loop of the Home Automation system should be implemented.
 * Make sure to configure and handle events as required for the specific Home Automation use case.
 *
 * @see loopHA() for the main loop of the Home Automation system.
 * @see handleEvent() for handling specific events in the Home Automation system.
 */

void setupHA() {
    // Set ID of Device.
    device.setUniqueId(macIO, sizeof(macIO));

    // Prepare for Home Assistant.
    device.setName("Wasserwerk");
    device.setSoftwareVersion("1.0.0");
    device.setModel("ESP32");
    device.setManufacturer("Jan Heil (www.byte-store.de)");
    device.enableSharedAvailability();
    device.enableLastWill();

    // Prepare Min Trigger.
    // Define min Level to refill.
    minIO.setMax(85);
    minIO.setMin(25);
    minIO.setName("Minimal");
    minIO.setUnitOfMeasurement("%");
    minIO.setMode(HANumber::ModeBox);
    minIO.setRetain(true);
    maxIO.onCommand(MQTT::onMin);

    // Prepare Normal Trigger.
    // Define normal Level to refill (when it's Rain and Cistern is getting to full Tank is getting filled to Max Value).
    normalIO.setMax(85);
    normalIO.setMin(25);
    normalIO.setName("Normal");
    normalIO.setUnitOfMeasurement("%");
    normalIO.setMode(HANumber::ModeBox);
    normalIO.setRetain(true);
    maxIO.onCommand(MQTT::onNormal);

    // Prepare Max Trigger.
    // Max Value for example in Emergency Situation.
    maxIO.setMax(85);
    maxIO.setMin(25);
    maxIO.setName("Maximal");
    maxIO.setUnitOfMeasurement("%");
    maxIO.setMode(HANumber::ModeBox);
    maxIO.setRetain(true);
    maxIO.onCommand(MQTT::onMax);

    // Prepare Pump 1
    pump1.setName("Brunnen");
    pump1.setCurrentState(false);
    pump1.setIcon("mdi:pump");

    // Prepare Pump 2
    pump2.setName("Zisterne");
    pump2.setCurrentState(false);
    pump2.setIcon("mdi:pump");

    // Prepare Pump 3
    pump3.setName("Druckspeicher");
    pump3.setCurrentState(false);
    pump3.setIcon("mdi:water-pump");

    // Prepare Max Level Switch.
    maxSwitch.setName("Überlauf");
    maxSwitch.setCurrentState(false);

    // Prepare Mix 1
    mix1.setName("Misch. Brunnen");
    mix1.setMax(100);
    mix1.setMin(10);
    mix1.setMode(HANumber::ModeBox);
    mix1.setUnitOfMeasurement("%");
    mix1.setRetain(true);

    // Prepare Mix 2
    mix2.setName("Misch. Zisterne");
    mix2.setMax(100);
    mix2.setMax(10);
    mix2.setMode(HANumber::ModeBox);
    mix2.setUnitOfMeasurement("%");
    mix2.setRetain(true);

    // Prepare Smart Mode
    smart.setIcon("mdi:eye-check");
    smart.setName("Smart Modus");
    smart.setRetain(true);
    smart.onCommand(MQTT::onSmart);

    // Prepare Buffer Tank.
    buffer.setName("Füllstand Puffer");
    buffer.setIcon("mdi:duck");
    buffer.setUnitOfMeasurement("%");

    // Prepare TL136 Value.
    distance.setName("TL136");
    distance.setUnitOfMeasurement("V");

    // Prepare Power.
    power.setName("Druckspeicher Power");
    power.setUnitOfMeasurement("W");
    power.setDeviceClass("power");
    power.setIcon("mdi:lightbulb");

    // Prepare Error Message
    errorIO.setName("Störungsmeldung");
    errorIO.setIcon("mdi:alert");

    // Print Debug Message.
    Serial.println("Finished registration in HomeAssistant.");
}

/**
 * @brief Sets up OTA (Over-The-Air) update functionality
 *
 * This function initializes the OTA update functionality, which allows the firmware
 * of the device to be updated remotely over a network connection. It sets up
 * the required OTA callbacks, configures the OTA parameters, and starts the OTA
 * update service.
 *
 * Usage: Call this function in the setup() function of your Arduino sketch to
 * enable OTA updates for your device.
 *
 * OTA provides a convenient way to remotely update firmware, ensuring the latest
 * version of the code is running on the device. It eliminates the need for manual
 * firmware updates via USB or other means, simplifying the process for both
 * developers and end users.
 *
 * @note This function should only be called once in the setup() function.
 *
 * @return void
 */
void setupOTA() {
    // Begin OTA Server with Internal Storage.
    AsyncElegantOTA.begin(&server/*, "ByteWasserwerk", "ByteWasserwerk"*/);
}

/**
 * @brief Executes a loop continuously until exit condition is met.
 *
 * The loop() function is a placeholder to demonstrate the structure of a loop execution.
 * It continues to execute until an exit condition is met. You can define various conditions
 * and actions inside the loop to perform repetitive tasks.
 *
 * @details
 * This is a generic loop function that can be used as a starting point in your code.
 * It does not provide any specific functionality, but it can be customized to suit your
 * program requirements. Please refer to the examples and documentation to learn how
 * to use this function effectively in your program.
 *
 * @warning
 * Be cautious while implementing infinite loops and ensure there is a mechanism to break
 * the loop when necessary. Otherwise, it may result in an infinite execution and may lead
 * to system locks or freezes.
 *
 * @param none
 * @retval none
 *
 * @see exitCondition()
 * @see performAction()
 *
 */

void loop() {
    // Handle MQTT Stream.
    mqtt.loop();

    // Loop Watcher Thread.
    Watcher::loop();

    // Handle Telnet Stream.
    //Device::handleTelnet();

    // Handle Slave Loop.
    Slave::loop();

    // Check if Timer is endend.
    if (updateIO.isReady()) {
        // Set Power Usage.
        power.setValue(Watcher::getPower());

        // Set Level Height.{
        buffer.setValue(Watcher::getLevelDistance());
        distance.setValue(Watcher::getDistance());

        // Set Error Message.
        errorIO.setValue(Slave::getError().c_str());

        pump1.setState(Slave::getState(0));
        pump2.setState(Slave::getState(1));
        pump3.setState(Slave::getState(2));

        maxSwitch.setCurrentState(Watcher::getLevelSwitch());

        // Reset Timer State.
        updateIO.reset();
    }

    // Check Ethernet Connection.
    if (!ETH.linkUp()) {
        Slave::setError(true, "Loosed Connection with Ethernet", false, "ETHERNET [X]");
    }
}

