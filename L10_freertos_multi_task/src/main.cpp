#include <Arduino.h>

TaskHandle_t printMessageTask1;
TaskHandle_t printMessageTask2;
TaskHandle_t printMessageTask3;
TaskHandle_t printMessageTask4;

void printMessage(void* ptr)
{
    Serial.println(static_cast<char*>(ptr));
    while (true)
    {
        Serial.printf("Current task parameters: %s, run_the_cpu: %d\n", static_cast<char*>(ptr), xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    // this_is_the_first_permanent_task，so_the_line_of_code_will_not_be_run，if_its_a_task_that_will_end，
    // it_needs_to_be_at_the_end_of_the_task，delete_the_current_task，the_parameter_is_null_to_delete_yourself
    vTaskDelete(nullptr);
}

void setup()
{
    Serial.begin(9600);
    vTaskDelay(pdMS_TO_TICKS(3000));
    xTaskCreate(printMessage, // task_function_pointer
                "printMessage1", // task_name
                2048, // task_stack_size，simple_task_stack_can_be_set_smaller，complex_tasks_need_to_be_set_to_get_the_stack，otherwise_it_will_easily_overflow
                const_cast<char*>("This is the first task parameter"), // what_parameters_need_to_be_passed_into_the_task_processing_function
                1, // task_priority，optional_range_is_0-24，the_bigger_the_number，the_higher_the_priority
                &printMessageTask1); // task_handle, if_there_is_a_control_requirement_for_this_task_in_the_future, this_parameter_can_be_set, TaskHandle_t type

    xTaskCreate(printMessage, // task_function_pointer
                "printMessage2", // task_name
                4096, // task_stack_size，simple_task_stack_can_be_set_smaller，complex_tasks_need_to_be_set_to_get_the_stack，otherwise_it_will_easily_overflow
                const_cast<char*>("This is the second task parameter"), // what_parameters_need_to_be_passed_into_the_task_processing_function
                2, // task_priority，optional_range_is_0-24，the_bigger_the_number，the_higher_the_priority
                &printMessageTask2); // task_handle, if_there_is_a_control_requirement_for_this_task_in_the_future, this_parameter_can_be_set, TaskHandle_t type

    vTaskDelay(pdMS_TO_TICKS(5000));
    // After 5s, delete_the_above_two_tasks
    vTaskDelete(printMessageTask1);
    vTaskDelete(printMessageTask2);

    xTaskCreatePinnedToCore(printMessage,
                            "printMessage3",
                            2048,
                            const_cast<char*>("The third task parameter"),
                            1,
                            &printMessageTask3,
                            0);
    xTaskCreatePinnedToCore(printMessage,
                            "printMessage4",
                            2048,
                            const_cast<char*>("The 4th task parameter"),
                            2,
                            &printMessageTask4,
                            1);
    vTaskDelay(pdMS_TO_TICKS(5000));
    // After 5s, delete_the_above_two_tasks
    vTaskDelete(printMessageTask3);
    vTaskDelete(printMessageTask4);
}

void loop()
{
}
