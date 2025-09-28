#include <Arduino.h>
#include <event_groups.h>

// define_some_event_bits_during_washing_machine_washing
#define DOOR_CLOSED   (1 << 0)  // indicates_that_the_washing_machine_door_is_closed
#define WATER_READY   (1 << 1)  // indicates_whether_the_water_level_meets_the_conditions
#define POWER_ON      (1 << 2)  // indicates_that_the_power_supply_is_turned_on

// create_event_group_handle
EventGroupHandle_t xWashingEventGroup;

// sensor_task_to_detect_whether_the_hatch_door_is_closed
void vDoorSensorTask(void *pvParameters);
// sensor_task_to_detect_whether_the_water_level_reaches_a_predetermined_position
void vWaterSensorTask(void *pvParameters);
// task_to_detect_whether_the_power_supply_is_turned_on
void vPowerMonitorTask(void *pvParameters);
// the_laundry_task_officially_begins
void vWashingMachineTask(void *pvParameters);

void setup() {
  Serial.begin(9600);
  vTaskDelay(pdMS_TO_TICKS(3000));

  // create_event_group
  xWashingEventGroup = xEventGroupCreate();

  // create_individual_subtasks
  xTaskCreate(vDoorSensorTask, "Door", 1024, nullptr, 1, nullptr);
  xTaskCreate(vWaterSensorTask, "Water", 1024, nullptr, 1, nullptr);
  xTaskCreate(vPowerMonitorTask, "Power", 1024, nullptr, 1, nullptr);
  xTaskCreate(vWashingMachineTask, "Washing", 1024, nullptr, 2, nullptr);

  Serial.println("The washing machine is ready, please_prepare_your_laundry...");
}

void loop() {
}

// simulated_door_sensor_detection_task
void vDoorSensorTask(void *pvParameters) {
  while (true) {
    // simulation_gate_state_changes
    vTaskDelay(pdMS_TO_TICKS(3000));

    // whether_the_random_simulation_door_is_closed
    if (random(0, 2) == 1) {
      Serial.println("Washing machine door closed");
      xEventGroupSetBits(xWashingEventGroup, DOOR_CLOSED);
    } else {
      Serial.println("The washing machine door is not closed, please close the door");
      xEventGroupClearBits(xWashingEventGroup, DOOR_CLOSED);
    }
  }
}

// water_level_sensor_task
void vWaterSensorTask(void *pvParameters) {
  while (true) {
    // simulate_water_level_changes
    vTaskDelay(pdMS_TO_TICKS(5000));

    // random_simulation_of_whether_the_water_level_meets_the_requirements
    if (random(0, 2) == 1) {
      Serial.println("The water level has reached the requirements");
      xEventGroupSetBits(xWashingEventGroup, WATER_READY);
    } else {
      Serial.println("The water level is insufficient, water_is_inflowing...");
      xEventGroupClearBits(xWashingEventGroup, WATER_READY);
    }
  }
}

// power_monitoring_tasks
void vPowerMonitorTask(void *pvParameters) {
  while (true) {
    // simulate_power_supply_status_changes
    vTaskDelay(pdMS_TO_TICKS(2000));

    // whether_the_random_analog_power_supply_is_turned_on
    if (random(0, 2) == 1) {
      Serial.println("The power supply is turned on");
      xEventGroupSetBits(xWashingEventGroup, POWER_ON);
    } else {
      Serial.println("The power supply is not turned on, please_check_the_power_supply...");
      xEventGroupClearBits(xWashingEventGroup, POWER_ON);
    }
  }
}

// washing_machine_task - wait_until_all_conditions_are_met_to_start_working
void vWashingMachineTask(void *pvParameters) {
  while (true) {
    // wait_for_all_conditions_to_be_met（door_closes、water_level_meets_standard、power_supply_is_turned_on）
    constexpr EventBits_t waitBits = (DOOR_CLOSED | WATER_READY | POWER_ON);

    const EventBits_t uxBits = xEventGroupWaitBits(
      xWashingEventGroup, // event_group_that_needs_to_be_listened_to
      waitBits, // what_events_need_to_be_listened_to，multiple_events_use_bitwise_or_combination_of_operations
      pdFALSE, // after_the_conditions_are_met，not_clearing_waiting_event_bits
      pdTRUE, // you_need_to_wait_for_all_conditions_to_be_met，if_false_it_means_that_any_condition_is_met
      portMAX_DELAY // unlimited_waiting
    );

    // check_if_all_conditions_are_met
    if ((uxBits & waitBits) == waitBits) {
      Serial.println("=== start_the_laundry_program ===");
      Serial.println("Laundry mode: Standard");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Washing...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Rinse...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Dehydration...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Laundry is finished!");
      Serial.println("===================");
    }
  }
}
