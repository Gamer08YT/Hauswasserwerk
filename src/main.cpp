#define ARDUINOHA_DEBUG

#include <Arduino.h>
#include <ArduinoHA.h>
#include <MQTT.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ETH.h>
#include "SPI.h"

//#include "ArduinoOTA.h"
//#include "InternalStorage.h"

// Store MAC Address of Device.
byte macIO[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Store WebServer Instance.
AsyncWebServer server(80);

// Store Ethernet Client Instance.
WiFiClient client;

#define ETH_ADDR        1
#define ETH_POWER_PIN   16//-1 //16 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 16 //17
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT // ETH_CLOCK_GPIO0_IN

// Define Home Assistant Credentials.
HADevice device;

// Define Home Assistant MQTT Instance.
HAMqtt mqtt(client, device, 24);

// Store Refill Trigger.
HANumber minIO("water_min", HABaseDeviceType::PrecisionP0);
HANumber maxIO("water_max", HABaseDeviceType::PrecisionP0);
HANumber normalIO("water_normal", HABaseDeviceType::PrecisionP0);

// Store States of Pumps.
HABinarySensor pump1("water_pump1");
HABinarySensor pump2("water_pump2");
HABinarySensor pump3("water_pump3");

// Store Mixing Values.
HANumber mix1("water_mix1");
HANumber mix2("water_mix2");

// Store Error Message.
HASensor errorIO("water_error");

// Store Smart Mode (Pump when Cistern has too much water or from Well when not enough water in Cistern).
HASwitch smart("water_smart");

// Store Buffer Tank Level.
HASensorNumber buffer("water_buffer", HABaseDeviceType::PrecisionP0);

// Store Power Usage of Pump3.
HASensorNumber power("water_load", HABaseDeviceType::PrecisionP2);

void setupLAN();

void setupOTA();

void handleOTA();

void handleWeb();

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

    // Print Debug Message.
    Serial.println("Starting Hauswasserwerk...");

    // Setup LAN Connection.
    setupLAN();

    // Setup Arduino OTA.
    setupOTA();

    // Setup HTTP Server.
    setupHTTP();

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
    // Handle Index Query.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hi! This is a sample response.");
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
    minIO.setMax(100);
    minIO.setMin(10);
    minIO.setName("Minimal");
    minIO.setUnitOfMeasurement("%");
    minIO.setMode(HANumber::ModeBox);

    // Prepare Normal Trigger.
    // Define normal Level to refill (when it's Rain and Cistern is getting to full Tank is getting filled to Max Value).
    normalIO.setMax(100);
    normalIO.setMin(10);
    normalIO.setName("Normal");
    normalIO.setUnitOfMeasurement("%");
    normalIO.setMode(HANumber::ModeBox);

    // Prepare Max Trigger.
    // Max Value for example in Emergency Situation.
    maxIO.setMax(100);
    maxIO.setMin(10);
    maxIO.setName("Minimal");
    maxIO.setUnitOfMeasurement("%");
    maxIO.setMode(HANumber::ModeBox);

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

    // Prepare Mix 1
    mix1.setName("Misch. Brunnen");
    mix1.setMax(100);
    mix1.setMin(10);
    mix1.setMode(HANumber::ModeBox);
    mix1.setUnitOfMeasurement("%");

    // Prepare Mix 2
    mix2.setName("Misch. Zisterne");
    mix2.setMax(100);
    mix2.setMax(10);
    mix2.setMode(HANumber::ModeBox);
    mix2.setUnitOfMeasurement("%");

    // Prepare Smart Mode
    smart.setIcon("mdi:eye-check");
    smart.setName("Smart Modus");
    smart.setRetain(true);
    smart.onCommand(MQTT.onSmart);

    // Prepare Buffer Tank.
    buffer.setMax(100);
    buffer.setMin(0);
    buffer.setName("Füllstand Puffer");
    buffer.setIcon("mdi:duck");

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
 * \brief Sets up the local area network (LAN) configuration.
 *
 * This function initializes the necessary variables and settings for
 * configuring and establishing a local area network (LAN).
 *
 * \note This function assumes that the necessary hardware and software
 * requirements for LAN setup are already in place.
 *
 * \note The specific implementation of this function may vary depending
 * on the platform and network architecture being used.
 *
 * \return No return value.
 */
void setupLAN() {
    // Begin ETH.
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); // Enable ETH

    // Set Hostname of Client.
    ETH.setHostname("Wasserwerk");

    // Wait for Ethernet Connection...
    while (!((uint32_t) ETH.localIP())) {};

    // Print Success Message.
    Serial.print("Successfully connected with Network IP-Address ");
    Serial.println(ETH.localIP());
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
}

