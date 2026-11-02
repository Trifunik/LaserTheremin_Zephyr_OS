#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <math.h>

#include "audio_signal.h"


#define SAMPLE_RATE      44100
#define NUM_CHANNELS     2
#define SAMPLE_BIT_WIDTH 16

/* Determine frame sizes: 100 frames = 100 L/R pairs */
#define FRAMES_PER_BLOCK 100
#define BLOCK_SIZE       (FRAMES_PER_BLOCK * NUM_CHANNELS * (SAMPLE_BIT_WIDTH / 8)) // 400 bytes
#define NUMBER_OF_BLOCKS 8

#define AMPLITUDE        30000 // Safe maximum range for int16_t (max 32767)
#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

int config_audio(struct i2s_config *config, const struct device *dev_i2s, struct k_mem_slab *mem_slab )
{
    int ret = 0;

    if (!device_is_ready(dev_i2s)) {
        printk("I2S device not ready\n");
        return -EIO;
    }
    
    config->word_size = SAMPLE_BIT_WIDTH;
    config->channels = NUM_CHANNELS;
    config->format = I2S_FMT_DATA_FORMAT_I2S;
    config->options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
    config->frame_clk_freq = SAMPLE_RATE;
    config->mem_slab = mem_slab;
    config->block_size = BLOCK_SIZE;
    config->timeout = SYS_FOREVER_MS;

    ret = i2s_configure(dev_i2s, I2S_DIR_TX, config);
    if (ret < 0) {
        printk("Failed to configure I2S: %d\n", ret);
        return ret;
    }
    return ret;
}

void fill_variable_frequency_buf(int16_t *tx_block, float frequency,  float *current_phase)
{
    // How much the sine wave angle moves forward each frame
    float phase_increment = (2.0f * (float)M_PI * frequency) / (float)SAMPLE_RATE;

    for (int i = 0; i < FRAMES_PER_BLOCK; i++) {
        // Compute current sample value
        int16_t sample_val = (int16_t)(AMPLITUDE * sinf(*current_phase));

        tx_block[2 * i]     = sample_val; // Left Channel
        tx_block[2 * i + 1] = sample_val; // Right Channel

        // Step phase forward
        *current_phase += phase_increment;

        // Keep phase bounded within 0 to 2*PI to prevent precision drift
        if (*current_phase >= (2.0f * (float)M_PI)) {
            *current_phase -= (2.0f * (float)M_PI);
        }
    }
}

int setup_device(const struct device *dev_i2s, struct k_mem_slab *mem_slab, float *current_phase)
{
    printk("HIER 1 \n");

    int ret = 0;
    void *block1 = NULL;
    void *block2 = NULL;
    
    ret = k_mem_slab_alloc(mem_slab, &block1, K_NO_WAIT);
    if (ret < 0) return ret;
    
    ret = k_mem_slab_alloc(mem_slab, &block2, K_NO_WAIT);
    if (ret < 0) return ret;

    fill_variable_frequency_buf((int16_t *)block1, 440.0, current_phase );
    fill_variable_frequency_buf((int16_t *)block2, 440.0, current_phase );

    ret = i2s_write(dev_i2s, block1, BLOCK_SIZE);
    ret = i2s_write(dev_i2s, block2, BLOCK_SIZE);
    
    return ret;
}



