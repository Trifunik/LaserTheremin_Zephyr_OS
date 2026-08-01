#ifndef DISTANCE_SENSOR
#define DISTANCE_SENSOR


int init_distance_sensor(const struct device *sensor1, const struct device *sensor2, const struct device *sensor3);
int get_distance(const struct device *sensor, struct sensor_value *sensorValue);

#endif // DISTANCE_SENSOR