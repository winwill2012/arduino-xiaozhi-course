#include <Arduino.h>
#include "driver/i2s.h"
#include "SPIFFS.h"

#define MAX98357_I2S_NUM  I2S_NUM_0 // which_i2s_port_to_use
#define SAMPLE_RATE       16000  // audio_sampling_rate
#define MAX98357_DOUT     38  // max98357 pin, please_refer_to: https://www.yuque.com/welinklab/pbihut/sdnm396nt3rmcfne
#define MAX98357_LRC      40
#define MAX98357_BCLK     39

File audioFile;
constexpr size_t CHUNK_SIZE = 256;
uint8_t buffer[CHUNK_SIZE];
size_t bytesWritten, bytesRead;

void setup()
{
    // build_i2s_configuration_structure
    constexpr i2s_config_t max98357_i2s_config = {
        // I2S sends data to digital amplifier, so_its_tx_mode
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16-bit width
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // left_channel_playback
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // standard_i2s_protocol
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // interrupt_priority，if_the_realtime_requirements_are_high，priority_can_be_raised
        .dma_buf_count = 4, // Number of DMA buffers
        .dma_buf_len = 1024, // the_number_of_audio_samples_that_can_be_saved_in_each_buffer，if_the_value_is_too_large，there_is_a_delay_in_playing_audio，if_the_value_is_too_small，may_cause_audio_playback_to_stutter
        .tx_desc_auto_clear = true // automatically_clean_up_dma_descriptors_after_data_transmission_is_completed，simple_and_convenient，and_it_can_prevent_memory_leaks_or_dma_buffer_overflow
    };

    // define_max98357_related_pins
    constexpr i2s_pin_config_t max98357_gpio_config = {
        .bck_io_num = MAX98357_BCLK,
        .ws_io_num = MAX98357_LRC,
        .data_out_num = MAX98357_DOUT,
        .data_in_num = -1
    };

    // start_the_i2s_driver
    i2s_driver_install(MAX98357_I2S_NUM, &max98357_i2s_config, 0, nullptr);
    // make_relevant_configurations_effective
    i2s_set_pin(MAX98357_I2S_NUM, &max98357_gpio_config);

    // start_the_serial_port
    Serial.begin(9600);

    if (!SPIFFS.begin(true)) // mount_the_file_system
    {
        Serial.println("SPIFFS Mount Failed");
        ESP.restart();
    }
    audioFile = SPIFFS.open("/test.pcm"); // open_a_file_in_the_file_system（be_sure_to_upload_first）
    if (!audioFile)
    {
        Serial.println("File open failed");
        ESP.restart();
    }

    while (audioFile.available()) // looping_the_audio_file_contents
    {
        // for_every_fragment_i_read，write_directly_to_the_i2s_channel_through_the_i2s_write_function，finally_it_plays_out_through_the_digital_amplifier_module
        bytesRead = audioFile.read(buffer, CHUNK_SIZE);
        const esp_err_t err = i2s_write(MAX98357_I2S_NUM, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
        if (err != ESP_OK || bytesWritten != bytesRead)
        {
            Serial.println("i2s_write failed");
        }
    }
}

void loop()
{
}
