#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/sensor/vl53l0x.h>

#include "distance_sensor.h"
#include "audio_signal.h"


#define MAX_DISTANCE_M 1.0f
#define STACK_SIZE 4096
#define PRIORITY   5


/* --- Global variables for distance --- */
static const struct device *const left_sensor = DEVICE_DT_GET(DT_ALIAS(left_sensor));
static const struct device *const right_sensor = DEVICE_DT_GET(DT_ALIAS(right_sensor));
static const struct device *const volume_sensor = DEVICE_DT_GET(DT_ALIAS(volume_sensor));

struct sensor_value left_sensor_value;
struct sensor_value right_sensor_value;
struct sensor_value volume_sensor_value;

/* --- Global variables for audio --- */

static float left_current_phase = 0.0;
static float right_current_phase = 0.0;

K_MEM_SLAB_DEFINE(i2s_mem_slab_1, BLOCK_SIZE, 6, 4);
K_MEM_SLAB_DEFINE(i2s_mem_slab_2, BLOCK_SIZE, 6, 4);

static const struct device *const dev_i2s_1 = DEVICE_DT_GET(DT_ALIAS(i2s_tx1));
static const struct device *const dev_i2s_2 = DEVICE_DT_GET(DT_ALIAS(i2s_tx2));

float left_current_frequency = 440.0;
float right_current_frequency = 440.0;


float filter_frequency(float current_value, float new_value)
{
    if (new_value > MAX_DISTANCE_M) {
        new_value = MAX_DISTANCE_M;
    }
    return (current_value + new_value * 4400.0f) / 2.0f;
}

void measurement(void)
{
	while (1) {

		get_distance(left_sensor, &left_sensor_value);
		left_current_frequency = sensor_value_to_float(&left_sensor_value);
		//left_current_frequency = filter_frequency(left_current_frequency, sensor_value_to_float(&left_sensor_value));
		
		get_distance(right_sensor, &right_sensor_value);
		right_current_frequency = sensor_value_to_float(&right_sensor_value);
		//right_current_frequency = filter_frequency(right_current_frequency, sensor_value_to_float(&right_sensor_value));
		
		get_distance(volume_sensor, &volume_sensor_value);

		//k_sleep(K_MSEC(10));
	}
}

K_THREAD_DEFINE(measurement_tid, STACK_SIZE, measurement, NULL, NULL, NULL,PRIORITY, 0, SYS_FOREVER_MS);


int main(void)
{
	int ret;
	
	struct i2s_config config_1;
    struct i2s_config config_2;
	 

	/* --- Init. all components ---*/
	ret = init_distance_sensor(left_sensor, right_sensor, volume_sensor);
	if (ret) {
		return 0;
	}
	k_sleep(K_MSEC(1000));

	ret = config_audio(&config_1, dev_i2s_1, &i2s_mem_slab_1 );
    if (ret < 0) {
        printk("Failed to configure I2S: %d\n", ret);
        return ret;
    }

    ret = config_audio(&config_2, dev_i2s_2, &i2s_mem_slab_2 );
    if (ret < 0) {
        printk("Failed to configure I2S: %d\n", ret);
        return ret;
    }

	/* --- Setup and preload audio signal --- */
    ret = setup_device(dev_i2s_1, &i2s_mem_slab_1, &left_current_phase);
    if (ret < 0) {
        printk("Failed to trigger I2S_1 start: %d\n", ret);
        return ret;
    }

    ret = setup_device(dev_i2s_2, &i2s_mem_slab_2, &right_current_phase);
    if (ret < 0) {
        printk("Failed to trigger I2S_2 start: %d\n", ret);
        return ret;
    }

	/* --- Start all components ---*/
	k_thread_start(measurement_tid);

    ret = i2s_trigger(dev_i2s_1, I2S_DIR_TX, I2S_TRIGGER_START);
    if (ret < 0) {
        printk("Failed to trigger I2S_1 start: %d\n", ret);
        return ret;
    }

	ret = i2s_trigger(dev_i2s_2, I2S_DIR_TX, I2S_TRIGGER_START);
    if (ret < 0) {
        printk("Failed to trigger I2S_2 start: %d\n", ret);
        return ret;
    }


	while (1) {

		void *left_next_block = NULL;
        void *right_next_block = NULL;
        
        if (k_mem_slab_alloc(&i2s_mem_slab_1, &left_next_block, K_MSEC(10)) == 0) {
            
            fill_variable_frequency_buf((int16_t *)left_next_block, left_current_frequency, &left_current_phase);

            ret = i2s_write(dev_i2s_1, left_next_block, BLOCK_SIZE);
            if (ret < 0) {
                printk("I2S write failed left_next_block: %d\n", ret);
                k_mem_slab_free(&i2s_mem_slab_1, left_next_block);
            }           
        }

        if (k_mem_slab_alloc(&i2s_mem_slab_2, &right_next_block, K_MSEC(10)) == 0) {

            fill_variable_frequency_buf((int16_t *)right_next_block, right_current_frequency, &right_current_phase);

            ret = i2s_write(dev_i2s_2, right_next_block, BLOCK_SIZE);
            if (ret < 0) {
                printk("I2S write failed right_next_block: %d\n", ret);
                k_mem_slab_free(&i2s_mem_slab_2, right_next_block);
            }
        }



		/*
		printf("Left Sensor distance is %f\n", left_current_frequency);
		printf("Right Sensor distance is %f\n", right_current_frequency);
		printf("Volume Sensor distance is %d\n", volume_sensor_value.val2);
   		*/
	}
	
	return 0;
}