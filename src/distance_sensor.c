#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/vl53l0x.h>
#include "distance_sensor.h"

int init_distance_sensor(const struct device *sensor1, const struct device *sensor2, const struct device *sensor3)
{
	int ret = 0;

	if (!device_is_ready(sensor1)) {
	    printk("Sensor 1: device not ready.\n");
	    return ret;
	}
	
	ret = sensor_sample_fetch(sensor1);
	if (ret) {
		printk("fetch sensor 1 failed ret %d\n", ret);
		return ret;
	}

	k_sleep(K_MSEC(1000));

    if (!device_is_ready(sensor2)) {
	    printk("Sensor 2: device not ready.\n");
	    return ret;
	}

	k_sleep(K_MSEC(1000));

    ret = sensor_sample_fetch(sensor2);
	if (ret) {
		printk("fetch sensor 2 failed ret %d\n", ret);
		return ret;
	}

    if (!device_is_ready(sensor3)) {
	    printk("Sensor 3: device not ready.\n");
	    return ret;
	}
    ret = sensor_sample_fetch(sensor3);
	if (ret) {
		printk("fetch sensor 3 failed ret %d\n", ret);
		return ret;
	}

	return ret;
}

int get_distance(const struct device *sensor, struct sensor_value *sensorValue)
{
	int ret = 0;
	ret = sensor_sample_fetch(sensor);
	if (ret) {
		printk("sensor_sample_fetch failed ret %d\n", ret);
		return ret;
	}

	ret = sensor_channel_get(sensor, SENSOR_CHAN_DISTANCE, sensorValue);
	return ret;
}