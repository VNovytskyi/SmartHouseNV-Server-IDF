/*
	Как добавить новую страницу:
		1) Копировать файл в папку server/
		2) Добавить указатели на начало и конец файла;
		3) Добавить условие в http_serve();
		4) Добавить путь у файлу в component.mk и CMakeLists.txt

*/




#include "websocket_server.h"
static QueueHandle_t client_queue;
const static int client_queue_size = 10;



const static char HTML_HEADER [] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
const static char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
const static char JS_HEADER   [] = "HTTP/1.1 200 OK\nContent-type: text/javascript\n\n";
const static char CSS_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
const static char PNG_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: image/png\n\n";
const static char ICO_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: image/x-icon\n\n";
const static char PDF_HEADER  [] = "HTTP/1.1 200 OK\nContent-type: application/pdf\n\n";
const static char EVENT_HEADER[] = "HTTP/1.1 200 OK\nContent-Type: text/event-stream\nCache-Control: no-cache\nretry: 3000\n\n";
const static char JSON_HEADER [] = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";

/* 
	Указатели на страницы 
*/

// default page
extern const uint8_t main_html_start[] asm("_binary_main_html_start");
extern const uint8_t main_html_end[] asm("_binary_main_html_end");

// script.js
extern const uint8_t script_js_start[] asm("_binary_script_js_start");
extern const uint8_t script_js_end[] asm("_binary_script_js_end");

// script.js
extern const uint8_t webSocket_js_start[] asm("_binary_webSocket_js_start");
extern const uint8_t webSocket_js_end[] asm("_binary_webSocket_js_end");

// test.css
extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");

// favicon.ico
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

// error page
extern const uint8_t error_html_start[] asm("_binary_error_html_start");
extern const uint8_t error_html_end[] asm("_binary_error_html_end");

//Обработчик WebSocket`а
void websocket_callback(uint8_t num, WEBSOCKET_TYPE_t type, char* msg, uint64_t msgLen) 
{
	const static char* TAG = "websocket_callback";
	
	switch(type) 
	{
		case WEBSOCKET_CONNECT:
			ESP_LOGI(TAG,"client %i connected!",num);
			break;
		
		case WEBSOCKET_DISCONNECT_EXTERNAL:
			ESP_LOGI(TAG,"client %i sent a disconnect message",num);
			break;
		
		case WEBSOCKET_DISCONNECT_INTERNAL:
			ESP_LOGI(TAG,"client %i was disconnected",num);
			break;
		
		case WEBSOCKET_DISCONNECT_ERROR:
			ESP_LOGI(TAG,"client %i was disconnected due to an error",num);
			break;
		
		case WEBSOCKET_TEXT:
			if(msgLen) 
			{
				/*
					msg - сообщение в формате JSON
					msgLen - его длинна
				*/
				
				ESP_LOGI(TAG, "msg: %s", msg);
				
				// broadcast it!
				ws_server_send_text_all_from_callback(msg, msgLen); 
			}
			break;
		
		case WEBSOCKET_BIN:
			ESP_LOGI(TAG,"client %i sent binary message of size %i:\n%s",num,(uint32_t)msgLen,msg);
			break;
		
		case WEBSOCKET_PING:
			ESP_LOGI(TAG,"client %i pinged us with message of size %i:\n%s",num,(uint32_t)msgLen,msg);
			break;
		
		case WEBSOCKET_PONG:
			ESP_LOGI(TAG,"client %i responded to the ping",num);
			break;
  }
}

static void http_serve(struct netconn *conn) 
{
	const static char* TAG = "http_server";
  
	struct netbuf* inbuf;
	static char* buf;
	static uint16_t buflen;
	static err_t err;


	netconn_set_recvtimeout(conn, 1000); // allow a connection timeout of 1 second
	ESP_LOGI(TAG,"reading from client...");
  
	err = netconn_recv(conn, &inbuf);
	ESP_LOGI(TAG,"read from client");
  
	if(err==ERR_OK) 
	{
		netbuf_data(inbuf, (void**)&buf, &buflen);
    
		if(buf) 
		{
			// default page
			if(strstr(buf,"GET / ") && !strstr(buf,"Upgrade: websocket")) 
			{
				ESP_LOGI(TAG,"Sending /");
				netconn_write(conn, HTML_HEADER, sizeof(HTML_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, main_html_start, main_html_end - main_html_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			// default page websocket
			else if(strstr(buf,"GET / ") && strstr(buf,"Upgrade: websocket")) 
			{
				ESP_LOGI(TAG,"Requesting websocket on /");
				ws_server_add_client(conn,buf,buflen,"/",websocket_callback);
				netbuf_delete(inbuf);
			}

			// script.js
			else if(strstr(buf,"GET /script.js ")) 
			{
				ESP_LOGI(TAG,"Sending /script.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, script_js_start, script_js_end - script_js_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			
			else if(strstr(buf,"GET /webSocket.js ")) 
			{
				ESP_LOGI(TAG,"Sending /webSocket.js");
				netconn_write(conn, JS_HEADER, sizeof(JS_HEADER)-1,NETCONN_NOCOPY);
				
				netconn_write(conn, webSocket_js_start, webSocket_js_end  - webSocket_js_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			// style.css
			else if(strstr(buf,"GET /style.css ")) 
			{
				ESP_LOGI(TAG,"Sending /style.css");
				netconn_write(conn, CSS_HEADER, sizeof(CSS_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, style_css_start, style_css_end - style_css_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			// favicon.ico
			else if(strstr(buf,"GET /favicon.ico ")) 
			{
				ESP_LOGI(TAG,"Sending favicon.ico");
				netconn_write(conn,ICO_HEADER,sizeof(ICO_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn,favicon_ico_start, favicon_ico_end - favicon_ico_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			
			// getHomeLocations
			else if(strstr(buf,"GET /getHomeLocations "))
			{
				char *str = "[{\"idHomeLocation\":1, \"title\":\"Bedroom\"},{\"idHomeLocation\":2, \"title\":\"Hall\"},{\"idHomeLocation\":3,\"title\":\"Bathroom\"},{\"idHomeLocation\":4,\"title\":\"Kitchen\"}]";
				
				ESP_LOGI(TAG, "Sending homeLocations");
				netconn_write(conn, JSON_HEADER, sizeof(JSON_HEADER)-1, NETCONN_NOCOPY);
				netconn_write(conn, str, strlen(str), NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			// getElementTypes
			else if(strstr(buf,"GET /getElementTypes "))
			{
				char *str = "[{\"idElementType\":1,\"title\":\"SimpleButton\"},{\"idElementType\":2,\"title\":\"DoubleButton\"},{\"idElementType\":3,\"title\":\"Range\"},{\"idElementType\":4,\"title\":\"ChangingButton\"},{\"idElementType\":5,\"title\":\"TripleButton\"},{\"idElementType\":6,\"title\":\"ColorPicker\"}]";
				
				ESP_LOGI(TAG, "Sending element types");
				netconn_write(conn, JSON_HEADER, sizeof(JSON_HEADER)-1, NETCONN_NOCOPY);
				netconn_write(conn, str, strlen(str), NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			
			// getLocationElements
			else if(strstr(buf,"GET /getLocationElements "))
			{
				char *str = "[{\"idLocationElement\":1,\"idHomeLocation\":1,\"idElementType\":4,\"title\":\"Main light\",\"port\":\"A0\"},{\"idLocationElement\":2,\"idHomeLocation\":1,\"idElementType\":4,\"title\":\"Lamp\",\"port\":\"A1\"},{\"idLocationElement\":3,\"idHomeLocation\":1,\"idElementType\":3,\"title\":\"Ventilation\",\"port\":\"B0\"},{\"idLocationElement\":120,\"idHomeLocation\":1,\"idElementType\":6,\"title\":\"Rgb\",\"port\":\"B0\"}]";
				
				ESP_LOGI(TAG, "Sending location elements");
				netconn_write(conn, JSON_HEADER, sizeof(JSON_HEADER)-1, NETCONN_NOCOPY);
				netconn_write(conn, str, strlen(str), NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
			
			else if(strstr(buf,"GET /")) 
			{
				ESP_LOGI(TAG,"Unknown request, sending error page: %s",buf);
				netconn_write(conn, ERROR_HEADER, sizeof(ERROR_HEADER)-1,NETCONN_NOCOPY);
				netconn_write(conn, error_html_start, error_html_end - error_html_start, NETCONN_NOCOPY);
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}

			else 
			{
				ESP_LOGI(TAG,"Unknown request");
				netconn_close(conn);
				netconn_delete(conn);
				netbuf_delete(inbuf);
			}
		}
		else 
		{
			ESP_LOGI(TAG,"Unknown request (empty?...)");
			netconn_close(conn);
			netconn_delete(conn);
			netbuf_delete(inbuf);
		}
	}
	else 
	{
		ESP_LOGI(TAG,"error on read, closing connection");
		netconn_close(conn);
		netconn_delete(conn);
		netbuf_delete(inbuf);
	}
}

// Принимает клиентов, которое пришли впервые и заносит их в очередь
static void server_task(void* pvParameters) 
{
	const static char* TAG = "server_task";
	struct netconn *conn, *newconn;
	static err_t err;
  
	client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));

	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 80);
	netconn_listen(conn);
  
	ESP_LOGI(TAG,"server listening");
  
	do 
	{
		err = netconn_accept(conn, &newconn);
		ESP_LOGI(TAG,"new client");
    
		if(err == ERR_OK) 
		{
			xQueueSendToBack(client_queue,&newconn,portMAX_DELAY);
			//http_serve(newconn);
		}
	} 
	while(err == ERR_OK);
  
	netconn_close(conn);
	netconn_delete(conn);
  
	ESP_LOGE(TAG,"task ending, rebooting board");
	esp_restart();
}

// Берет клиентов из очереди и обрабатывает их
static void server_handle_task(void* pvParameters) 
{
	const static char* TAG = "server_handle_task";
	struct netconn* conn;
  
	ESP_LOGI(TAG,"task starting");
  
	for(;;) 
	{
		xQueueReceive(client_queue, &conn, portMAX_DELAY);
    
		if(!conn) 
			continue;
    
		http_serve(conn);
	}
  
	vTaskDelete(NULL);
}