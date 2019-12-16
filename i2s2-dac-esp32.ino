#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include <math.h>

//#define FRAME_SIZE      4
#define FRAME_SIZE      8
#define SAMPLE_RATE     (44100)
#define I2S_NUM         I2S_NUM_0
#define WAVE_FREQ_HZ    (100)

const double twoPi = 2.0 * PI;

double sin_phase = 0.0;
double sin_phase_step = 200.0 / SAMPLE_RATE * twoPi;

double sin2_phase = 0.0;
double sin2_phase_step = 200.0 / SAMPLE_RATE * twoPi;

static void output_audio()
{
    static double frequency = 100.0;
    sin_phase_step = frequency / SAMPLE_RATE * twoPi;
    frequency += 0.01;
    
    uint8_t samples_data[FRAME_SIZE][2][2];

    for (int i = 0; i < FRAME_SIZE; ++i)
    {
    	// channel 1
    	
        double sin1_float = sin(sin_phase);

        sin_phase = sin_phase + sin_phase_step;
        if (sin_phase >= twoPi)
          sin_phase -= twoPi;

        sin1_float *= 1 << (8 - 1);

        // channel 2

        double sin2_float = sin(sin2_phase);

        sin2_phase = sin2_phase + sin2_phase_step;
        if (sin2_phase >= twoPi)
          sin2_phase -= twoPi;

        sin2_float *= 1 << (8 - 1);

        // output

        samples_data[i][0][0] = 0;
        samples_data[i][0][1] = (uint8_t)(int8_t)sin1_float + 128;
        samples_data[i][1][0] = 0;
        samples_data[i][1][1] = (uint8_t)(int8_t)sin2_float + 128;
    }

    size_t i2s_bytes_write = 0;
    
    i2s_write(
      I2S_NUM,
      samples_data,
      FRAME_SIZE * sizeof(int16_t) * 2,
      &i2s_bytes_write,
      portMAX_DELAY);
}

void setup()
{
    i2s_config_t i2s_config;
    i2s_config.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
    i2s_config.sample_rate = SAMPLE_RATE;
    i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    i2s_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB);
    i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    i2s_config.dma_buf_count = 2;
    i2s_config.dma_buf_len = FRAME_SIZE * sizeof(int16_t) * 2;
    i2s_config.use_apll = false;
    i2s_config.tx_desc_auto_clear = true;
    i2s_config.fixed_mclk = 0;

    
    derer/audio_renderer.c
    esp_chip_info_t out_info;
    esp_chip_info(&out_info);
    if(out_info.revision > 0)
    {
        //i2s_config.use_apll = true;
        ESP_LOGI(TAG, "chip revision %d, enabling APLL for I2S", out_info.revision);
    }
    
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, NULL);
    
    i2s_set_sample_rates(I2S_NUM, SAMPLE_RATE);
}

void loop()
{
    for (;;)
    {
        output_audio();
    }
}

