#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/sensor/vl53l0x.h>

#include "distance_sensor.h"
#include "audio_signal.h"



#define STACK_SIZE 4096
#define PRIORITY   5

/* --- Audio definitions --- */
#define SAMPLE_RATE      44100
#define NUM_CHANNELS     2
#define SAMPLE_BIT_WIDTH 16

#define FRAMES_PER_BLOCK 100
#define BLOCK_SIZE       (FRAMES_PER_BLOCK * NUM_CHANNELS * (SAMPLE_BIT_WIDTH / 8)) // 400 bytes
#define NUMBER_OF_BLOCKS 10

/* --- Global variables for distance --- */
static const struct device *const left_sensor = DEVICE_DT_GET(DT_ALIAS(left_sensor));
static const struct device *const right_sensor = DEVICE_DT_GET(DT_ALIAS(right_sensor));
static const struct device *const volume_sensor = DEVICE_DT_GET(DT_ALIAS(volume_sensor));

struct sensor_value left_sensor_value;
struct sensor_value right_sensor_value;
struct sensor_value volume_sensor_value;

/* --- Global variables for audio --- */
/*
static float current_phase_1 = 0.0;
static float current_phase_2 = 0.0;

K_MEM_SLAB_DEFINE(i2s_mem_slab_1, BLOCK_SIZE, 6, 4);
K_MEM_SLAB_DEFINE(i2s_mem_slab_2, BLOCK_SIZE, 6, 4);

static const struct device *const dev_i2s_1 = DEVICE_DT_GET(DT_ALIAS(i2s_tx1));
static const struct device *const dev_i2s_2 = DEVICE_DT_GET(DT_ALIAS(i2s_tx2));
*/
double left_current_frequency = 440.0;
double right_current_frequency = 440.0;


double filter_frequency(double current_value, double new_value)
{ 	/* 10cm --> 440Hz */
	return (current_value + new_value*4400) / 2;
} 

void measurement(void)
{
	while (1) {

		get_distance(left_sensor, &left_sensor_value);
		left_current_frequency = filter_frequency(left_current_frequency, sensor_value_to_double(&left_sensor_value));
		
		get_distance(right_sensor, &right_sensor_value);
		right_current_frequency = filter_frequency(right_current_frequency, sensor_value_to_double(&right_sensor_value));
		
		get_distance(volume_sensor, &volume_sensor_value);

		k_sleep(K_MSEC(1000));
	}
}

K_THREAD_DEFINE(measurement_tid, STACK_SIZE, measurement, NULL, NULL, NULL,PRIORITY, 0, SYS_FOREVER_MS);


int main(void)
{
	int ret;
	
	struct i2s_config config_1;
    struct i2s_config config_2;
	 

	ret = init_distance_sensor(left_sensor, right_sensor, volume_sensor);
	if (ret) {
		return 0;
	}

	k_sleep(K_MSEC(1000));

	k_thread_start(measurement_tid);


	printf(" --- APP started ---\n");

	while (1) {


		printf("Left Sensor distance is %f\n", left_current_frequency);
		printf("Right Sensor distance is %f\n", right_current_frequency);
		printf("Volume Sensor distance is %d\n", volume_sensor_value.val2);
   
		k_sleep(K_MSEC(1000));
	}
	
	return 0;
}