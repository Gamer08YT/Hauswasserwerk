//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_PINOUT_H
#define HAUSWASSERWERK_PINOUT_H

#define ERROR_LAMP 2
#define PUMP_3 15
#define LEVEL_SWITCH 35
#define LEVEL_FILL 36
#define LEVEL_ALARM 14
#define METER 39
#define UNLOCK_BUTTON 12
#define SDA 5
#define SCL 17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MIN_SLAVE_POWER 200
#define MAX_SLAVE_DISABLED 5

/**
 * Define EMA Filtering
 * 0.05 very good, very slow => 5% from new Value
 * 0.30 ok, ok => 30% from new Value
 * 0.50 very bad, very fast => 50% from new Value
 */
#define EMA_ALPHA 0.3f

#endif //HAUSWASSERWERK_PINOUT_H
