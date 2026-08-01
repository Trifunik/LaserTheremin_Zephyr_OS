#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor/vl53l0x.h>
#include "distance_sensor.h"

#define VOLUME_SENSOR_NODE DT_NODELABEL(volume_sensor)
#define LEFT_SENSOR_NODE DT_NODELABEL(left_sensor)
#define RIGHT_SENSOR_NODE DT_NODELABEL(right_sensor)


static const struct device *const volumeSensor = DEVICE_DT_GET(VOLUME_SENSOR_NODE);
static const struct device *const leftSensor = DEVICE_DT_GET(LEFT_SENSOR_NODE);
static const struct device *const rightSensor = DEVICE_DT_GET(RIGHT_SENSOR_NODE);



int main(void)
{
    struct sensor_value leftSensorValue;
    struct sensor_value rightSensorValue;
	struct sensor_value volumeSensorValue;
    int ret;


	ret = init_distance_sensor(leftSensor, rightSensor, volumeSensor);
	if (ret) {
		return 0;
	}

	k_sleep(K_MSEC(100));

	printf(" --- APP started ---\n");

	while (1) {

		ret = get_distance(leftSensor, &leftSensorValue);
		if(ret) {
			return 0;
		}
		printf("Left Sensor distance is %.3fm\n", sensor_value_to_double(&leftSensorValue));

		ret = get_distance(rightSensor, &rightSensorValue);
		if(ret) {
			return 0;
		}
		printf("Right Sensor distance is %.3fm\n", sensor_value_to_double(&rightSensorValue));
		ret = get_distance(volumeSensor, &volumeSensorValue);
		if(ret) {
			return 0;
		}
		printf("Volume Sensor distance is %.3fm\n", sensor_value_to_double(&volumeSensorValue));
   
		k_sleep(K_MSEC(1000));
	}
	
	return 0;
}