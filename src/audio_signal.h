#ifndef AUDIO_SIGNAL
#define AUDIO_SIGNAL

int config_audio(struct i2s_config *config, const struct device *dev_i2s, struct k_mem_slab *mem_slab );
int setup_device(const struct device *dev_i2s, struct k_mem_slab *mem_slab, float *current_phase);

void fill_variable_frequency_buf(int16_t *tx_block, float frequency,  float *current_phase);

#endif // AUDIO