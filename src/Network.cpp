//
// Created by Jan Heil on 12.08.2023.
//

#include <ETH.h>
#include "Network.h"
#include "Slave.h"

#define ETH_ADDR        1
#define ETH_POWER_PIN   16//-1 //16 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 16 //17
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT // ETH_CLOCK_GPIO0_INpio run --target clean

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
void Network::setupLAN() {
    // Begin ETH.
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); // Enable ETH

    // Set Hostname of Client.
    ETH.setHostname("Wasserwerk");

    // Wait for Ethernet Connection...
    while (!((uint32_t) ETH.localIP())) {};

    // Set Hostname of Client.
    ETH.setHostname("Wasserwerk");

    // Set Display Message.
    Slave::infoDisplay("Netzwerk", "VERBUNDEN");

    // Print Success Message.
    Serial.print("Successfully connected with Network IP-Address ");
    Serial.println(ETH.localIP());
}




