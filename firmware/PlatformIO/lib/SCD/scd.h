#ifndef scd_h
#define scd_h

// Includes
#include <Arduino.h>

/**
 * @brief SCD Library
 */
class Scd {
    public:
        Scd();
        void init(int I2C_SDA, int I2C_SCL);
        void readSensor(uint16_t &co2, float &temp, float &hum);
};

#endif