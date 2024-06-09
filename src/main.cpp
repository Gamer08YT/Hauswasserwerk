#define ARDUINOHA_DEBUG

#include <Arduino.h>
#include <ArduinoHA.h>
#include <MQTT.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
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

// Store WebServer Instance.
AsyncWebServer server(80);

// Timer for HA Updates.
SimpleTimer updateIO(1500);

// Define Home Assistant Credentials.
HADevice device;

WiFiClient client;

// Define Home Assistant MQTT Instance.
HAMqtt mqtt(client, device, 16);

// Store Refill Trigger.
HANumber minIO("water_min", HABaseDeviceType::PrecisionP0);
HANumber maxIO("water_max", HABaseDeviceType::PrecisionP0);
HANumber normalIO("water_normal", HABaseDeviceType::PrecisionP0);

// Store States of Pumps.
HABinarySensor pump1("water_pump1");
HABinarySensor pump2("water_pump2");
HABinarySensor pump3("water_pump3");
HABinarySensor maxSwitch("water_max");
HABinarySensor moistSwitch("water_moist");

// Store Mixing Values.
HANumber mix1("water_mix1");

// Store Voltage Calibration.
HANumber minVoltage("water_minV", HABaseDeviceType::PrecisionP2);
HANumber maxVoltage("water_maxV", HABaseDeviceType::PrecisionP2);


// Store Error Message.
HASensor errorIO("water_error");

// Store Smart Mode (Pump when Cistern has too much water or from Well when not enough water in Cistern).
HASwitch smart("water_smart");

// Store Buffer Tank Level.
HASensorNumber buffer("water_buffer", HABaseDeviceType::PrecisionP0);

// Store Buffer Level Distance.
HASensorNumber voltage("water_voltage", HABaseDeviceType::PrecisionP2);

// Store Power Usage of Pump3.
HASensorNumber power("water_load", HABaseDeviceType::PrecisionP2);


void setupOTA();

void setupHA();

void setupMQTT();

void setupHTTP();

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

    // Create new Slave Object.

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

    // Wait for a short Period.
    delay(250);

    // Setup HomeAssistant.
    setupHA();

    // Wait for a short Period.
    delay(500);

    // Setup MQTT.
    setupMQTT();

}

void setupHTTP() {
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
    mqtt.begin("homeassistant.local", "wasserwerk1", "wasserwerk");

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
    device.setSoftwareVersion("1.1.5");
    device.setModel("ESP32");
    device.setManufacturer("Jan Heil (www.byte-store.de)");
    device.enableSharedAvailability();
    device.enableLastWill();

    // Prepare Min Trigger.
    // Define min Level to refill.
    minIO.setMax(50);
    minIO.setMin(25);
    minIO.setName("Minimal");
    minIO.onCommand(MQTT::onMin);
    minIO.setUnitOfMeasurement("%");
    minIO.setMode(HANumber::ModeBox);
    minIO.setRetain(true);

    // Prepare Normal Trigger.
    // Define normal Level to refill (when it's Rain and Cistern is getting to full Tank is getting filled to Max Value).
    normalIO.setMax(75);
    normalIO.setMin(25);
    normalIO.setName("Normal");
    normalIO.onCommand(MQTT::onNormal);
    normalIO.setUnitOfMeasurement("%");
    normalIO.setMode(HANumber::ModeBox);
    normalIO.setRetain(true);

    // Prepare Max Trigger.
    // Max Value for example in Emergency Situation.
    maxIO.setMax(85);
    maxIO.setMin(25);
    maxIO.setName("Maximal");
    maxIO.onCommand(MQTT::onMax);
    maxIO.setUnitOfMeasurement("%");
    maxIO.setMode(HANumber::ModeBox);
    maxIO.setRetain(true);

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

    // Prepare Moist Switch.
    moistSwitch.setName("Rohrbruch");
    moistSwitch.setCurrentState(false);

    // Prepare Mix 1
    mix1.setName("Misch. Brunnen / Zisterne");
    mix1.setMax(100);
    mix1.setMin(10);
    mix1.setMode(HANumber::ModeSlider);
    mix1.setUnitOfMeasurement("%");
    mix1.setRetain(true);
    mix1.onCommand(MQTT::onRatio);

    // Prepare Fields for Voltage Calibrations.
    minVoltage.setRetain(true);
    minVoltage.onCommand(MQTT::minV);
    minVoltage.setUnitOfMeasurement("V");
    minVoltage.setName("Min Voltage");
    minVoltage.setMode(HANumber::ModeBox);

    maxVoltage.setRetain(true);
    maxVoltage.onCommand(MQTT::maxV);
    maxVoltage.setName("Max Voltage");
    maxVoltage.setUnitOfMeasurement("V");
    maxVoltage.setMode(HANumber::ModeBox);


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
    voltage.setName("TL136");
    voltage.setUnitOfMeasurement("V");

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

    // Set Display Message.
    Slave::infoDisplay("Server", "VERBUNDEN");
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
    ElegantOTA.begin(&server);

    // Set Display Message.
    Slave::infoDisplay("OTA", "ONLINE");
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

    // Handle Slave Loop.
    Slave::loop();

    // Check if Timer is endend.
    if (updateIO.isReady()) {
        // Set Power Usage.
        power.setValue(Watcher::getPower());

        // Set Level Height.{
        buffer.setValue(Watcher::getLevelDistance());
        voltage.setValue(Watcher::getDistance());

        // Set Error Message.
        errorIO.setValue(Slave::getError().c_str());

        pump1.setState(Slave::getState(0));
        pump2.setState(Slave::getState(1));
        pump3.setState(Slave::getState(2));

        // Read Values of Digital Inputs.
        maxSwitch.setCurrentState(Watcher::getLevelSwitch());
        moistSwitch.setCurrentState(Watcher::readLevelAlarm());

        // Reset Timer State.
        updateIO.reset();
    }

    // Check Ethernet Connection.
    if (!ETH.linkUp()) {
        Slave::setError(true, "Loosed Connection with Ethernet", false, "ETHERNET [X]");
    }
}

