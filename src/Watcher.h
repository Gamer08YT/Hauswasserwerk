//
// Created by Jan Heil on 12.08.2023.
//

#ifndef HAUSWASSERWERK_WATCHER_H
#define HAUSWASSERWERK_WATCHER_H


class Watcher {
public:
    static void setup();
    static bool getLevelSwitch();
    static float getLevelDistance();
    static void loop();
};


#endif //HAUSWASSERWERK_WATCHER_H
