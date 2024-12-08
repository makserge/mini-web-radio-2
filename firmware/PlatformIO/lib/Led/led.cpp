// Includes
#include <Arduino.h>
#include "led.h"

/**
 * @brief Construct a new Led:: Led object
 * 
 * @param LED_B 
 */
Led::Led(int LED_B)
{
    pinMode(LED_B,OUTPUT);
    digitalWrite(LED_B,HIGH);
    // Setup PWM
    ledcSetup(3, 12000, 8);
    // Attach PWN to LEDs
    ledcAttachPin(LED_B, 3);
};

/**
 * @brief Set LED to Colour
 * 
 * @param hexstring 
 */
void Led::setColour(String hexstring)
{
    long number = (long) strtol( &hexstring[0], NULL, 16);
    int B = number & 0xFF;
    ledcWrite(3, (255-B));
};

/**
 * @brief Set LED state
 * 
 * @param colour 
 * @param time 
 * @param repeat 
 */
void Led::setState(String colour, int time, int repeat)
{
    for(int i = repeat; i > 0; i--)
    {
      this->setColour(colour);
      delay(time);
      this->setColour("000000");
    }
}