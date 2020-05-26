/*
	Команды IDF
	
	idf.py menuconfig - вызов оконного интерфейса для настройки нашей прошивки
*/


#include <stdio.h>



/*
	Библиотека для вывода логов.
	
	Выбор уровня детализации логинга:
		Перед сборкой:   menuconfig -> bootloader config -> bootloader log verbosity;
		Во время работы: esp_log_level_set(EESP_LOG_NONE | ESP_LOG_ERROR | ESP_LOG_WARN | ESP_LOG_INFO | ESP_LOG_DEBUG | ESP_LOG_VERBOSE). ! Но не выше чем было
		
	ESP_LOGE - error 	(lowest level)
	ESP_LOGW - warning
	ESP_LOGI - info
	ESP_LOGD - debug
	ESP_LOGV - verbose  (highest level)
	
	
*/
#include "esp_log.h"


/*
	FreeRTOS
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#define WIFI_SSID "MERCUSYS_7EBA"
#define WIFI_PASS "3105vlad3010vlada"

#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static EventGroupHandle_t s_wifi_event_group;

static int reconnectCounter = 0;
static int maxRecconectCounter = 3;

//Сюда будут попадать уведомления о привязаных, с помощью esp_event_handler_register, событиях
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if(event_base == WIFI_EVENT)
	{
		if(event_id == WIFI_EVENT_STA_START)
		{
			esp_wifi_connect();
		} 
		
		if(event_id == WIFI_EVENT_STA_DISCONNECTED)
		{
			if(++reconnectCounter < maxRecconectCounter)
			{
				esp_wifi_connect();
				ESP_LOGW("Wi-Fi", "Reconnect to AP");
			}
			else
			{
				ESP_LOGE("Wi-Fi", "Failed connect to AP");
				xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
			}
		}
		
	}
	
	if(event_base == IP_EVENT)
	{
		if(event_id == IP_EVENT_STA_GOT_IP)
		{
			reconnectCounter = 0;
			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			ESP_LOGI("IP", "Got ip: %s", ip4addr_ntoa(&event->ip_info.ip));
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		}
	}
}


void connectToAP()
{
	s_wifi_event_group = xEventGroupCreate();
	
	tcpip_adapter_init();
	
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	ESP_LOGI("Test", "1");
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_LOGI("Test", "2");
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_LOGI("Test", "3");
	
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
	
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
	
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
	

	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
	
	if (bits & WIFI_CONNECTED_BIT) 
	{
        ESP_LOGI("Wi-Fi", "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    } 
	else if (bits & WIFI_FAIL_BIT) 
	{
        ESP_LOGI("Wi-Fi", "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
    } 
	else 
	{
        ESP_LOGE("Wi-Fi", "UNEXPECTED EVENT");
    }
	
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}


/* Точка входа */
void app_main(void)
{
	//Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	connectToAP();
	
	ESP_LOGI("My", "Ready");
}
