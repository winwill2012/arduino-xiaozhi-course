#include <Arduino.h>
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

#define MAX98357_I2S_NUM  I2S_NUM_0 // which_i2s_port_to_use
#define SAMPLE_RATE       16000  // audio_sampling_rate
#define MAX98357_DOUT     38  // max98357 pin, please_refer_to: https://www.yuque.com/welinklab/pbihut/sdnm396nt3rmcfne
#define MAX98357_LRC      40
#define MAX98357_BCLK     39

#define MICROPHONE_I2S_NUM             I2S_NUM_1
#define AUDIO_SAMPLE_RATE              16000
#define MICROPHONE_I2S_BCLK            42
#define MICROPHONE_I2S_LRC             2
#define MICROPHONE_I2S_DOUT            1

#define  READ_SAMPLE_COUNT 80000  // define_recording_length，a_total_of_80k_samples，for_16k_sampling_rate，just_record_5s

int16_t buffer[READ_SAMPLE_COUNT];
size_t bytesRead, bytesWritten;

RingbufHandle_t ringBuffer;

void playAudio(void* ptr);

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


    constexpr i2s_config_t mic_i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // the_left_and_right_channels_here_should_be_consistent_with_the_circuit
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false
    };
    constexpr i2s_pin_config_t mic_gpio_config = {
        .bck_io_num = MICROPHONE_I2S_BCLK,
        .ws_io_num = MICROPHONE_I2S_LRC,
        .data_out_num = -1,
        .data_in_num = MICROPHONE_I2S_DOUT
    };

    i2s_driver_install(MICROPHONE_I2S_NUM, &mic_i2s_config, 0, nullptr);
    i2s_set_pin(MICROPHONE_I2S_NUM, &mic_gpio_config);

    Serial.begin(9600);

    ringBuffer = xRingbufferCreate(160000 * sizeof(int16_t), RINGBUF_TYPE_BYTEBUF);
    if (ringBuffer == nullptr)
    {
        Serial.println("ringBuffer creation failed");
        ESP.restart();
    }
    // start_the_audio_playback_task
    xTaskCreate(playAudio, "playAudioTask", 64000, nullptr, 1, nullptr);
}

// get_recorded_audio_data_from_ringbuffer，perform_volume_gain，last_play
void playAudio(void* ptr)
{
    size_t readItemSize;
    while (true)
    {
        void* buffer = xRingbufferReceive(ringBuffer, &readItemSize, pdMS_TO_TICKS(1000));
        if (buffer != nullptr)
        {
            auto* audioData = static_cast<int16_t*>(buffer);
            for (int i = 0; i < readItemSize; i++)
            {
                // because_the_recording_volume_is_low(it_may_be_that_the_motherboard_recording_hole_is_too_small_it_will_affect_it)
                // so_gain_the_audio_and_limit_it_to_the_effective_range
                // note_that_when_int32_t_is_used_here_to_calculate，mainly_to_prevent_overflow
                auto value = static_cast<int32_t>(audioData[i] * 10.0);
                if (value > 32767) value = 32767;
                if (value < -32768) value = -32768;
                audioData[i] = static_cast<int16_t>(value);
            }
            // write_data_to_the_digital_amplifier_i2s_channel_through_i2s_write，perform_audio_playback
            i2s_write(MAX98357_I2S_NUM, audioData, bytesRead,
                      &bytesWritten, portMAX_DELAY);
            // after_processing，remember_to_return_the_buffer_memory
            vRingbufferReturnItem(ringBuffer, buffer);
        }
    }
    vTaskDelete(nullptr);
}

void loop()
{
    if (Serial.available()) // determine_whether_there_is_data_input_on_the_serial_port，there_are_input_inputs_to_start_recording
    {
        Serial.readStringUntil('\n');
        Serial.println("Recording...");
        // record_audio_from_i2s_channel_through_the_i2s_read_function
        // and_save_to_buffer_array, bytesRead is the final number of bytes read
        const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM, buffer,
                                       80000 * sizeof(int16_t),
                                       &bytesRead, portMAX_DELAY);
        if (err != ESP_OK)
        {
            Serial.println("I2S read failed");
        }
        else
        {
            // write_recording_data_to_ringbuffer
            xRingbufferSend(ringBuffer, buffer, bytesRead, portMAX_DELAY);
        }
    }
}
