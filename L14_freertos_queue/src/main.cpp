#include <Arduino.h>
#include <queue.h>

// define_order_structure
typedef struct
{
    int customerId; // customer_id
    String dishName; // dish_name
    bool isVip; // is_it_a_vip_or_not
    unsigned long orderTime; // order_time
} Order;

// create_a_queue
QueueHandle_t orderQueue;

// customer_order_task
TaskHandle_t customerTaskHandle;
// chef_cooking_task
TaskHandle_t chefTaskHandle;

// dishes_list
const char* dishes[] = {
    "Kung Pao Chicken", "Fish-flavored shredded pork", "Mapo tofu", "Sweet and Sour Ribs", "Refried meat"
};

// customer_tasks - producer
void customerTask(void* pvParameters)
{
    Order newOrder;
    int customerCount = 0;

    while (true)
    {
        // randomly_generated_orders
        newOrder.customerId = ++customerCount;
        newOrder.dishName = dishes[random(0, 5)];
        newOrder.isVip = (random(0, 10) >= 7); // 30%probability_is_vip
        newOrder.orderTime = millis();

        Serial.print("customer #");
        Serial.print(newOrder.customerId);
        Serial.print("Clicked:");
        Serial.print(newOrder.dishName);
        Serial.print(", VIP: ");
        Serial.println(newOrder.isVip ? "yes" : "no");

        // decide_the_location_of_the_queue_based_on_whether_the_vip_is_used
        if (newOrder.isVip)
        {
            xQueueSendToFront(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("VIP orders have been processed first");
        }
        else
        {
            xQueueSendToBack(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("Ordinary orders have been added to the queue");
        }

        // waiting_for_the_next_customer (1-5 seconds)
        vTaskDelay(pdMS_TO_TICKS(random(1000, 5000)));
    }
}

// chef_mission - consumer
void chefTask(void* pvParameters)
{
    Order currentOrder;

    while (true)
    {
        // receive_orders_from_queue
        if (xQueueReceive(orderQueue, &currentOrder, portMAX_DELAY) == pdPASS)
        {
            Serial.print("The chef starts making:");
            Serial.print(currentOrder.dishName);
            Serial.print("To the customer #");
            Serial.println(currentOrder.customerId);

            // Simulate_cooking_time (2-4 seconds)
            vTaskDelay(pdMS_TO_TICKS(random(2000, 4000)));

            Serial.print("The chef completes the customer #");
            Serial.print(currentOrder.customerId);
            Serial.print("Clicked:");
            Serial.println(currentOrder.dishName);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    randomSeed(millis());
    // create_an_order_queue，store_up_to_10_orders
    orderQueue = xQueueCreate(10, sizeof(Order));
    if (orderQueue != nullptr)
    {
        // create_a_task
        xTaskCreate(customerTask, "Customer", 2048, nullptr, 1, &customerTaskHandle);
        xTaskCreate(chefTask, "Chef", 2048, nullptr, 2, &chefTaskHandle);
    }
    else
    {
        Serial.println("Failed to create a queue!");
    }
}

void loop()
{
}
