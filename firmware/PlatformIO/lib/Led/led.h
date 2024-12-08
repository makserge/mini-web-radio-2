#ifndef led_h
#define led_h

// Includes
#include <Arduino.h>

/**
 * @brief LED Library
 */
class Led {
    public:
        Led(int LED_B);
        void setColour(String hexstring);
        void setState(String colour, int time, int repeat);
};

#endif