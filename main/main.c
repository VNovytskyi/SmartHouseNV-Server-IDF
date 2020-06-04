/*
	System
*/
#include <stdio.h>
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <esp_system.h>
#include <sys/param.h>
#include <string.h>
#include "esp_wifi.h"


/*
	FreeRTOS
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#include "MyWiFi.c"
#include "MyServer.c"


/* Точка входа */
void app_main(void)
{
	//Инициализация NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
	{
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	connectToAP();
    
	int wsStatus = ws_server_start();
	
	if(wsStatus == 1)
	{
		ESP_LOGI("WS", "Successful start");
	}
	else if(wsStatus == 0)
	{
		ESP_LOGW("WS", "Server already running");
	}
	else
	{
		ESP_LOGE("WS", "Not handled status of webSocket server");
	}
	
	xTaskCreate(&server_task, "server_task", 3000, NULL, 9, NULL);
    xTaskCreate(&server_handle_task, "server_handle_task", 4000, NULL, 6, NULL);
	
	ESP_LOGI("app_main", "Ready");
    
    
}
