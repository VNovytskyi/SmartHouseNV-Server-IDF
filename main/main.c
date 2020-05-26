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


#ifdef HTTPD_MAX_REQ_HDR_LEN
#undefine HTTPD_MAX_REQ_HDR_LEN
#define 2048
#endif

#include "esp_event.h"
#include "nvs_flash.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"

/* Server */
#include <esp_http_server.h>
static httpd_handle_t server = NULL;


/* Wi-Fi*/
#include "esp_wifi.h"
#define WIFI_SSID "MERCUSYS_7EBA"
#define WIFI_PASS "3105vlad3010vlada"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static int reconnectCounter = 0;
static int maxRecconectCounter = 3;
static EventGroupHandle_t wifiEventGroup;



static void connectionToAPHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
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
				xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
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
			xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
		}
	}
}

void connectToAP()
{
	wifiEventGroup = xEventGroupCreate();
	
	tcpip_adapter_init();
	
	
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &connectionToAPHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connectionToAPHandler, NULL));
	
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
	

	EventBits_t bits = xEventGroupWaitBits(wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
	
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
	
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &connectionToAPHandler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &connectionToAPHandler));
    vEventGroupDelete(wifiEventGroup);
}

static esp_err_t hello_get_handler(httpd_req_t *req)
{
	char*  buf;

	size_t buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	
	if (buf_len > 1) 
	{
        buf = malloc(buf_len);

        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) 
		{
            ESP_LOGI("Request", "Found header => Host: %s", buf);
        }
		
        free(buf);
    }
	
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if(buf_len > 1)
	{
		buf = malloc(buf_len);
		
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) 
		{
			ESP_LOGI("Request", "Found URL query => %s", buf);
			
			char param[32];

            if (httpd_query_key_value(buf, "a", param, sizeof(param)) == ESP_OK) 
			{
                ESP_LOGI("Request", "Found URL query parameter => a=%s", param);
            }
		}
		
		free(buf);
	}
	
	/* Добавляем заголовки к ответу*/
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");


	/* Отправляем тело ответа */
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI("Request", "Request headers lost");
    }
    return ESP_OK;
}

static esp_err_t test_get_handler(httpd_req_t *req)
{
	char*  buf;

	size_t buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	
	if (buf_len > 1) 
	{
        buf = malloc(buf_len);

        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) 
		{
            ESP_LOGI("Request", "Found header => Host: %s", buf);
        }
		
        free(buf);
    }
	
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if(buf_len > 1)
	{
		buf = malloc(buf_len);
		
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) 
		{
			ESP_LOGI("Request", "Found URL query => %s", buf);
			
			char param[32];

            if (httpd_query_key_value(buf, "a", param, sizeof(param)) == ESP_OK) 
			{
                ESP_LOGI("Request", "Found URL query parameter => a=%s", param);
            }
		}
		
		free(buf);
	}
	
	/* Добавляем заголовки к ответу*/
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");


	/* Отправляем тело ответа */
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI("Request", "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    .user_ctx  = "Hello World!"
};

static const httpd_uri_t test = {
    .uri       = "/test",
    .method    = HTTP_GET,
    .handler   = test_get_handler,
    .user_ctx  = "This is test page"
};

static httpd_handle_t startWebserver(void)
{
	httpd_handle_t server = NULL;
	
	/* 
		Проводится конфигурация сервера стандартными значениями. 
		Подробно: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html
	*/
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	
	ESP_LOGI("Server", "Starting server on port: '%d'", config.server_port);
	
	if(httpd_start(&server, &config) == ESP_OK)
	{
		ESP_LOGI("Server", "Registering URI handlers");
        
		httpd_register_uri_handler(server, &hello);
		ESP_LOGI("Server", "Register URI: /hello");
		
		httpd_register_uri_handler(server, &test);
		ESP_LOGI("Server", "Register URI: /test");
       
		return server;
	}
	
	ESP_LOGI("Server", "Error starting server!");
    return NULL;
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
	
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	connectToAP();
	
	server = startWebserver();
	
	ESP_LOGI("My", "Ready");
	ESP_LOGI("Debug", "%d, %d", CONFIG_HTTPD_MAX_URI_LEN, CONFIG_HTTPD_MAX_REQ_HDR_LEN  );
}
