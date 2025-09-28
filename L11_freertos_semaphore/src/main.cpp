#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

SemaphoreHandle_t semaphore;

void count(void* pvParameters)
{
    Serial.println("Start counting...");
    for (int i = 1; i <= 10; i++)
    {
        Serial.printf("Current report: %d\n", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    xSemaphoreGive(semaphore);  // The_signal_is_released_after_the_end_of_the_report, when_released_here, can you take it successfully in the setup function?
    vTaskDelete(nullptr);
}

void setup()
{
    Serial.begin(9600);
    semaphore = xSemaphoreCreateBinary();

    // create_a_counting_task，from_1_to_10_the_counting_is_completed
    xTaskCreate(count,
        "count",
        2048,
        nullptr,
        1,
        nullptr);

    // wait_for_semaphore_to_get（the_semaphore_will_be_released_only_if_the_previous_task_count_is_completed），if_you_cant_get_it_youll_wait
    xSemaphoreTake(semaphore, portMAX_DELAY);
    Serial.println("Acquisition of semaphore successfully");
}

void loop()
{
}
