/*
	Как добавить новую страницу:
		1) Копировать файл в папку serverFiles/
		2) Добавить указатели на начало и конец файла;
		3) Добавить условие в http_serve();
		4) Добавить путь у файлу в component.mk и CMakeLists.txt
*/



//TODO: Глянуть в спецификации, может заголовки уже есть
const static char HTML_HEADER [] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
const static char JS_HEADER   [] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
const static char CSS_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
const static char PNG_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
const static char ICO_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
const static char PDF_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
const static char JSON_HEADER [] = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";


extern const uint8_t main_html_start[] asm("_binary_main_html_start");
extern const uint8_t main_html_end[] asm("_binary_main_html_end");

extern const uint8_t script_js_start[] asm("_binary_script_js_start");
extern const uint8_t script_js_end[] asm("_binary_script_js_end");

extern const uint8_t webSocket_js_start[] asm("_binary_webSocket_js_start");
extern const uint8_t webSocket_js_end[] asm("_binary_webSocket_js_end");

extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");

extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

extern const uint8_t error_html_start[] asm("_binary_error_html_start");
extern const uint8_t error_html_end[] asm("_binary_error_html_end");


static esp_err_t home_get_handler(httpd_req_t *req){
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, (const char*)main_html_start, main_html_end - main_html_start);
    return ESP_OK;
}

static esp_err_t style_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, (const char*)style_css_start, style_css_end - style_css_start);
    return ESP_OK;
}

static esp_err_t script_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, (const char*)script_js_start, script_js_end - script_js_start);
    return ESP_OK;
}

static esp_err_t webSocket_get_handler(httpd_req_t *req){
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, (const char*)webSocket_js_start, webSocket_js_end - webSocket_js_start);
    return ESP_OK;
}

static esp_err_t favicon_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, (const char*)favicon_ico_start, favicon_ico_end - favicon_ico_start);
    return ESP_OK;
}

static esp_err_t homeLocations_get_handler(httpd_req_t *req){
    char *str = "[{\"idHomeLocation\":1, \"title\":\"Bedroom\"},{\"idHomeLocation\":2, \"title\":\"Hall\"},{\"idHomeLocation\":3,\"title\":\"Bathroom\"},{\"idHomeLocation\":4,\"title\":\"Kitchen\"}]";

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, str, strlen(str));
    return ESP_OK;
}

static esp_err_t elementTypes_get_handler(httpd_req_t *req){
    char *str = "[{\"idElementType\":1,\"title\":\"SimpleButton\"},{\"idElementType\":2,\"title\":\"DoubleButton\"},{\"idElementType\":3,\"title\":\"Range\"},{\"idElementType\":4,\"title\":\"ChangingButton\"},{\"idElementType\":5,\"title\":\"TripleButton\"},{\"idElementType\":6,\"title\":\"ColorPicker\"}]";
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, str, strlen(str));
    return ESP_OK;
}

static esp_err_t locationElements_get_handler(httpd_req_t *req){
    char *str = "[{\"idLocationElement\":1,\"idHomeLocation\":1,\"idElementType\":4,\"title\":\"Main light\",\"port\":\"A0\"},{\"idLocationElement\":2,\"idHomeLocation\":1,\"idElementType\":4,\"title\":\"Lamp\",\"port\":\"A1\"},{\"idLocationElement\":3,\"idHomeLocation\":1,\"idElementType\":3,\"title\":\"Ventilation\",\"port\":\"B0\"},{\"idLocationElement\":120,\"idHomeLocation\":1,\"idElementType\":6,\"title\":\"Rgb\",\"port\":\"B0\"}]";
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, str, strlen(str));
    return ESP_OK;
}


static const httpd_uri_t home = {
    .uri       = "/home",
    .method    = HTTP_GET,
    .handler   = home_get_handler,
    .user_ctx  = "HomePage"
};

static const httpd_uri_t style = {
    .uri       = "/style.css",
    .method    = HTTP_GET,
    .handler   = style_get_handler,
    .user_ctx  = "style.css"
};

static const httpd_uri_t script = {
    .uri       = "/script.js",
    .method    = HTTP_GET,
    .handler   = script_get_handler,
    .user_ctx  = "script.js"
};

static const httpd_uri_t webSocket = {
    .uri       = "/webSocket.js",
    .method    = HTTP_GET,
    .handler   = webSocket_get_handler,
    .user_ctx  = "webSocket.js"
};

static const httpd_uri_t favicon = {
    .uri       = "/favicon.ico",
    .method    = HTTP_GET,
    .handler   = favicon_get_handler,
    .user_ctx  = "favicon.ico"
};


static const httpd_uri_t homeLocations = {
    .uri       = "/getHomeLocations",
    .method    = HTTP_GET,
    .handler   = homeLocations_get_handler,
    .user_ctx  = "/getLocationElements"
};

static const httpd_uri_t elementTypes = {
    .uri       = "/getElementTypes",
    .method    = HTTP_GET,
    .handler   = elementTypes_get_handler,
    .user_ctx  = "/getElementTypes"
};

static const httpd_uri_t locationElements = {
    .uri       = "/getLocationElements",
    .method    = HTTP_GET,
    .handler   = locationElements_get_handler,
    .user_ctx  = "/getLocationElements"
};



static httpd_handle_t startHTTPSserver(void)
{
    const static char *TAG = "HTTP Server";
	httpd_handle_t server = NULL;
	
	/* 
		Проводится конфигурация сервера стандартными значениями. 
		Подробно: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html
	*/
    httpd_config_t config = {                        
        .task_priority      = tskIDLE_PRIORITY+5,       
        .stack_size         = 4096,                     
        .server_port        = 80,                       
        .ctrl_port          = 32768,                    
        .max_open_sockets   = 7,                        
        .max_uri_handlers   = 8,                        
        .max_resp_headers   = 8,                        
        .backlog_conn       = 5,                        
        .lru_purge_enable   = false,                    
        .recv_wait_timeout  = 5,                        
        .send_wait_timeout  = 5,                        
    };
	
	ESP_LOGI(TAG, "Starting on port: '%d'", config.server_port);
	
	if(httpd_start(&server, &config) == ESP_OK)
	{
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &style);
		httpd_register_uri_handler(server, &script);
        httpd_register_uri_handler(server, &webSocket);
        httpd_register_uri_handler(server, &favicon);
        
        httpd_register_uri_handler(server, &homeLocations);
        httpd_register_uri_handler(server, &elementTypes);
        httpd_register_uri_handler(server, &locationElements);
       
		return server;
	}
	
	ESP_LOGI("Server", "Error starting server!");
    return NULL;
}