#include "mqtt_client.h"
#include "esp_wifi.h"

// The build system automatically handles and embeds the certificates pointing to these external symbols
extern const uint8_t aws_root_ca_pem_start[]   asm("_binary_aws_root_ca_pem_start");
extern const uint8_t client_crt_start[]        asm("_binary_client_crt_start");
extern const uint8_t client_key_start[]        asm("_binary_client_key_start");

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("AWS_MQTT", "Successfully connected to AWS IoT Core!");
            esp_mqtt_client_subscribe(event->client, "esp32/sub", 1);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI("AWS_MQTT", "Topic = %.*s", event->topic_len, event->topic);
            ESP_LOGI("AWS_MQTT", "Data = %.*s", event->data_len, event->data);
            break;
        default:
            break;
    }
}

void app_main(void) {
    // 1. Initialize NVS and Wi-Fi Peripherals here...
    
    // 2. Configure MQTT Client structure with Mutual TLS Credentials
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://://amazonaws.com", // Replace endpoint
        .broker.verification.certificate = (const char *)aws_root_ca_pem_start,
        .credentials.authentication.certificate = (const char *)client_crt_start,
        .credentials.authentication.key = (const char *)client_key_start,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    while (1) {
        // Periodically construct a JSON payload and publish
        char payload[64];
        snprintf(payload, sizeof(payload), "{\"temperature\": %.2f}", 24.5);
        esp_mqtt_client_publish(client, "esp32/pub", payload, 0, 1, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
