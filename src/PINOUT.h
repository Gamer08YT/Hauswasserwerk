//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_PINOUT_H
#define HAUSWASSERWERK_PINOUT_H

#define ERROR_LAMP 14
#define PUMP_3 15
#define LEVEL_SWITCH 12
#define LEVEL_ECHO 36
#define LEVEL_TRIGGER 4
#define METER 39
#define WATER 35
// Tank Height + Sensor Offset of 20 cm = 54cm + 20cm.
#define TANK_HEIGHT 74
#define UNLOCK_BUTTON 2
#define SDA 5
#define SCL 17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define USONIC_DIV 58.0
#define DEBOUNCE_TIME  50
#define MIN_SLAVE_POWER 200
#define MAX_SLAVE_DISABLED 5

#endif //HAUSWASSERWERK_PINOUT_H
