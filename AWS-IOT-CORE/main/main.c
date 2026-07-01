#include "mqtt_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <esp_log.h>
#include <string.h>

#define THINGNAME "ESP32" // AWS IoT requires a Client ID / Thing Name


//Put aws root ca1 certificat here , delete "put certificate " line and paste there , copy it all with the begin and end
static const char aws_root_ca_pem[] = R"raw(put certificate)raw";

//put client certificate here same as before
static const char client_crt_pem[] = R"raw(put certificate)raw";

//put client privvate key here same as before
static const char client_private_key_pem[] = R"raw(put certificate)raw";

static bool wifi_connected = false;
static bool mqtt_connected = false;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT) {

        switch (event_id) {

        case WIFI_EVENT_STA_START:
            printf("WiFi: Connecting...\n");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            printf("WiFi: Disconnected. Reconnecting...\n");
            wifi_connected = false;
            mqtt_connected = false;
            esp_wifi_connect();
            break;

        default:
            break;
        }
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        printf("WiFi Connected!\n");
        printf("IP Address: " IPSTR "\n",
               IP2STR(&event->ip_info.ip));

        wifi_connected = true;
    }
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:

        mqtt_connected = true;

        printf("AWS MQTT Connected!\n");

        esp_mqtt_client_subscribe(
            event->client,
            "iotfrontier/sub",
            1);

        break;

    case MQTT_EVENT_DISCONNECTED:

        mqtt_connected = false;
        printf("MQTT Disconnected!\n");

        break;

    case MQTT_EVENT_DATA:

        printf("Topic : %.*s\n",
               event->topic_len,
               event->topic);

        printf("Data  : %.*s\n",
               event->data_len,
               event->data);

        break;

    case MQTT_EVENT_ERROR:

        printf("MQTT ERROR!\n");

        break;

    default:
        break;
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Wokwi-GUEST",
            .password = "",
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Waiting for WiFi connection...\n");

    while (!wifi_connected) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    printf("WiFi Ready.\n");
    printf("Starting MQTT...\n");

    const esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.hostname =
            "", //domain name/endpoint

        .broker.address.port = 8883,

        .broker.address.transport =
            MQTT_TRANSPORT_OVER_SSL,

        .broker.verification.certificate =
            aws_root_ca_pem,

        .credentials.client_id =
            THINGNAME,

        .credentials.authentication.certificate =
            client_crt_pem,

        .credentials.authentication.key =
            client_private_key_pem,
    };

    esp_mqtt_client_handle_t client =
        esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(client);

    printf("Waiting for MQTT connection...\n");

    while (!mqtt_connected) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    printf("MQTT Connected. Publishing...\n");

    while (1) {

        const char *payload =
            "{\"temperature\":24.5,\"humidity\":50.0}";

        int msg_id =
            esp_mqtt_client_publish(
                client,
                "iotfrontier/pub",
                payload,
                0,
                1,
                0);

        printf("Published. msg_id = %d\n", msg_id);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}




//********************************************************************
//this one use esplogi


/*
static bool wifi_connected = false;
static bool mqtt_connected = false;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT) {

        switch (event_id) {

        case WIFI_EVENT_STA_START:
            ESP_LOGI("WIFI", "Connecting...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW("WIFI", "Disconnected. Reconnecting...");
            wifi_connected = false;
            mqtt_connected = false;
            esp_wifi_connect();
            break;

        default:
            break;
        }
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI("WIFI",
                 "Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));

        wifi_connected = true;
    }
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:

        mqtt_connected = true;

        ESP_LOGI("MQTT", "Connected to AWS IoT!");

        esp_mqtt_client_subscribe(
            event->client,
            "iotfrontier/sub",
            1);

        break;

    case MQTT_EVENT_DISCONNECTED:

        mqtt_connected = false;
        ESP_LOGW("MQTT", "Disconnected");

        break;

    case MQTT_EVENT_DATA:

        printf("Topic: %.*s\n",
               event->topic_len,
               event->topic);

        printf("Data : %.*s\n",
               event->data_len,
               event->data);

        break;

    case MQTT_EVENT_ERROR:

        ESP_LOGE("MQTT", "MQTT Error");

        break;

    default:
        break;
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Wokwi-GUEST",
            .password = "",
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("MAIN", "Waiting for WiFi...");

    while (!wifi_connected) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI("MAIN", "WiFi Ready");

    const esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.hostname =
            "", //domain name/endpoint

        .broker.address.port = 8883,

        .broker.address.transport =
            MQTT_TRANSPORT_OVER_SSL,

        .broker.verification.certificate =
            aws_root_ca_pem,

        .credentials.client_id =
            THINGNAME,

        .credentials.authentication.certificate =
            client_crt_pem,

        .credentials.authentication.key =
            client_private_key_pem,
    };

    esp_mqtt_client_handle_t client =
        esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(client);

    while (!mqtt_connected) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ESP_LOGI("MAIN", "Publishing...");

    while (1) {

        const char *payload =
            "{\"temperature\":24.5,\"humidity\":50.0}";

        int msg_id = esp_mqtt_client_publish(
            client,
            "iotfrontier/pub",
            payload,
            0,
            1,
            0);

        ESP_LOGI("MQTT", "Published msg_id=%d", msg_id);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

*/