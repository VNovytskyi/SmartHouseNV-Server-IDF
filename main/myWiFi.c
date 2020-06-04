/* Wi-Fi*/
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
