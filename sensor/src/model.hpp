#ifndef SENSOR_MODEL_HPP
#define SENSOR_MODEL_HPP

#include "types.hpp"
#include <ctime>

struct SensorDynamicsConfig {
    DataFrame start;
    float dTemp;
    int dHum;
};

struct SensorModel {
    SensorDynamicsConfig dynamics_config;
    DataFrame current_data;

    SensorModel(SensorDynamicsConfig):
    current_data{dynamics_config.start} {

    }

    void step() {
        srand(time(0));
        current_data.temperature += (rand()%2 * 0.5 - 1)*dynamics_config.dTemp*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
        current_data.humidity += (rand()%2 * 0.5 - 1)*dynamics_config.dTemp*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    }
};

SensorModel DEFAULT_MODEL = {{{23.5, 60}, 0.1, 1}};
SensorModel HIGH_MODEL = {{26.4, 67}, 0.2, 2};
SensorModel LOW_MODEL = {{20.3, 52}, 0.2, 1};

#endif // !SENSOR_MODEL_HPP