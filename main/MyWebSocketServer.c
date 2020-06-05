#include "websocket_server.h"

static QueueHandle_t client_queue;
const static int client_queue_size = 10;


extern const uint8_t error_html_start[];
extern const uint8_t error_html_end[];

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
			if(strstr(buf,"GET / ") && strstr(buf,"Upgrade: websocket")) 
			{
				ESP_LOGI(TAG,"Requesting websocket on /");
				ws_server_add_client(conn,buf,buflen,"/",websocket_callback);
				netbuf_delete(inbuf);
			}
			else if(strstr(buf,"GET /")) 
			{
				ESP_LOGI(TAG,"Unknown request, sending error page: %s",buf);
                
                char ERROR_HEADER[] = "HTTP/1.1 404 Not Found\nContent-type: text/html\n\n";
                
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

static void server_task(void* pvParameters) 
{
	const static char* TAG = "server_task";
	struct netconn *conn, *newconn;
	static err_t err;
  
	client_queue = xQueueCreate(client_queue_size, sizeof(struct netconn*));

	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 88);
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