#ifndef AUDIO_SIGNAL
#define AUDIO_SIGNAL

#define SAMPLE_RATE      44100
#define NUM_CHANNELS     2
#define SAMPLE_BIT_WIDTH 16

#define FRAMES_PER_BLOCK 512
#define BLOCK_SIZE       (FRAMES_PER_BLOCK * NUM_CHANNELS * (SAMPLE_BIT_WIDTH / 8))
#define NUMBER_OF_BLOCKS 8

#define AMPLITUDE        32000 // Safe maximum range for int16_t (max 32767)
#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

int config_audio(struct i2s_config *config, const struct device *dev_i2s, struct k_mem_slab *mem_slab );
int setup_device(const struct device *dev_i2s, struct k_mem_slab *mem_slab, float *current_phase);
void fill_variable_frequency_buf(int16_t *tx_block, float volume, float frequency,  float *current_phase);

#endif // AUDIO