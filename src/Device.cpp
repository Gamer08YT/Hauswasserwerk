//
// Created by Jan Heil on 13.08.2023.
//

#include <Arduino.h>
#include <vector>
#include "Device.h"


/**
* Println Message to Serial and Telnet.
* @param dataIO
*/
void Device::println(String dataIO) {
    Serial.println(dataIO);
}

/**
 * Print Message to Serial and Telnet.
 * @param dataIO
 */
void Device::print(String dataIO) {
    Serial.print(dataIO);
}

/**
 * Write Message to Serial and Telnet.
 * @param dataIO
 */

void Device::write(String dataIO) {
    Serial.write(dataIO.toInt());
}

