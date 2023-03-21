#ifndef SENSOR_TYPES_HPP
#define SENSOR_TYPES_HPP

#include <iostream>
#include <array>
#include <cstring>

using sensor_payload_t = std::array<uint8_t, sizeof(float) + sizeof(int32_t)>;

struct DataFrame {
    float temperature;
    int32_t humidity;

    sensor_payload_t hex() {
        sensor_payload_t ret{};
        memcpy(ret.data(), &temperature, sizeof(float));
        memcpy(ret.data() + sizeof(float), &humidity, sizeof(int32_t));
        return ret;
    }
};

#endif // !SENSOR_TYPES_HPP