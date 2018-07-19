///*
// * http_download.c
// *
// *  Created on: Jul 3, 2018
// *      Author: hai_dotcom
// */
#include "http_download.h"
#include "my_sd_card.h"
//

//char REQUEST[] = "GET " WEB_URL " HTTP/1.0\r\n"
//    "Host: "WEB_SERVER"\r\n"
//    "User-Agent: esp-idf/1.0 esp32\r\n"
//    "\r\n";

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

extern xQueueHandle HttpDownload_Queue_Handle;
extern TaskHandle_t xhttp_download_Handle;
extern TaskHandle_t xhttp_update_Handle;
extern char temp_update[32];

esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id){
	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_event_group,CONNECTED_BIT);
		break;
	default:
		break;
	}
	return ESP_OK;
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
    			.sta = {
    					.ssid = "Hai Dotcom",
    					.password = "doremonnobita",
    					.bssid_set = 0
    			},
    		};
    printf("Setting WiFi configuration SSID %s... \n", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void http_download_task(void *pvParameters)
{
	const struct addrinfo hints = {
			.ai_family = AF_INET,
			.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	struct in_addr *addr;
	int s,r;
	char recv_buf[64];
	data rec_data;
	while(1){
		if(xQueueReceive(HttpDownload_Queue_Handle,&rec_data,portMAX_DELAY)){
			xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
			                            false, true, portMAX_DELAY);

			printf("Connected to AP");

			int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

			if(err != 0 || res == NULL) {
				printf("DNS lookup failed err=%d res=%p \n", err, res);
	            vTaskDelay(1000 / portTICK_PERIOD_MS);
	            continue;
	        }

	        /* Code to print the resolved IP.
	           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
	        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
	        printf("DNS lookup succeeded. IP=%s \n", inet_ntoa(*addr));

	        s = socket(res->ai_family, res->ai_socktype, 0);
	        if(s < 0) {
	            printf("... Failed to allocate socket. \n");
	            freeaddrinfo(res);
	            vTaskDelay(1000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        printf("... allocated socket\n");

	        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
	        	printf("... socket connect failed errno=%d \n", err);
	        	close(s);
	        	freeaddrinfo(res);
	        	vTaskDelay(4000 / portTICK_PERIOD_MS);
	        	continue;
	        }

	        printf("... connected \n");
	        freeaddrinfo(res);

	        if (write(s, rec_data.request, strlen(rec_data.request)) < 0) {
	            printf("... socket send failed \n");
	            close(s);
	            vTaskDelay(4000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        printf("... socket send success \n");
	        struct timeval receiving_timeout;
	        receiving_timeout.tv_sec = 5;
	        receiving_timeout.tv_usec = 0;
	        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
	            printf("... failed to set socket receiving timeout \n");
	            close(s);
	            vTaskDelay(4000 / portTICK_PERIOD_MS);
	            continue;
	        }
	        printf("... set socket receiving timeout success\n");

	        /* Read HTTP response */
	        printf("Read HTTP response \n");
	        bool flag = false;
//	        bool update_flag = false;
	        FILE * f = fopen(rec_data.name,"r");
	        if(f==NULL){
	        	printf("Json doesn't exist \n");
	        }
	        int count = 0;
	        //If it's not a json file, don't need to filter out http header
	        if(strstr(rec_data.name,"json.txt") == NULL){
	        	printf("Not JSON \n");
	        	flag = true;
	        }
	        else{
	        	printf(rec_data.name);
	        }

	        do {
	            bzero(recv_buf, sizeof(recv_buf));
	            r = read(s, recv_buf, sizeof(recv_buf)-1);
	            if(strstr(recv_buf,"update")){
	          		if(strstr(recv_buf,rec_data.version) == NULL){
	               		printf("Updates are available \n");
	               		printf(rec_data.version);
	               		printf("\n");
	               		printf("%s\n",recv_buf);
	               		f = fopen(rec_data.name,"wb");
	               	  if(f == NULL){
	               	    printf("Failed to open file for writing \n");
	               	  }else
	               		printf("Open file \n");
	                }
	            	else{
	            		printf("No update available \n");
	            		printf(rec_data.version);
	            		printf("\n");
	            		break;
	            	}

	            }
	            for(int i = 0; i<r; i++){
	            	//'{': begin of json file
	            	if(recv_buf[i] == '{')
	            		flag = true;
	            	if(flag == true)
	            		fputc(recv_buf[i],f);
	            }
	            printf("\n downloading ... %d \n",count++);
	        } while(r > 0);

	        fclose(f);
	        printf("... done reading from socket. Last read return=%d errno=%d\r\n", r, err);
	        close(s);
	        if(xhttp_download_Handle != NULL){
	        	vTaskDelete(xhttp_download_Handle);
	        	printf("HTTP task was deleted after using \n");
	        }
	    }
	}
}

//void http_check_update_task(void *pvParameters)
//{
//	const struct addrinfo hints = {
//				.ai_family = AF_INET,
//				.ai_socktype = SOCK_STREAM,
//	};
//	struct addrinfo *res;
//	struct in_addr *addr;
//	int s,r;
//	char recv_buf[64];
//	data rec_data;
//	while(1){
//		if(xQueueReceive(HttpDownload_Queue_Handle,&rec_data,portMAX_DELAY)){
//			xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
//			                            false, true, portMAX_DELAY);
//			printf("Connected to AP");
//
//			int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);
//
//			if(err != 0 || res == NULL) {
//				printf("DNS lookup failed err=%d res=%p \n", err, res);
//		        vTaskDelay(1000 / portTICK_PERIOD_MS);
//		        continue;
//	        }
//
//	        /* Code to print the resolved IP.
//           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
//	        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
//	        printf("DNS lookup succeeded. IP=%s \n", inet_ntoa(*addr));
//
//	        s = socket(res->ai_family, res->ai_socktype, 0);
//	        if(s < 0) {
//	        	printf("... Failed to allocate socket. \n");
//	            freeaddrinfo(res);
//	            vTaskDelay(1000 / portTICK_PERIOD_MS);
//	            continue;
//	        }
//	        printf("... allocated socket\n");
//
//	        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
//	        	printf("... socket connect failed errno=%d \n", err);
//	        	close(s);
//	        	freeaddrinfo(res);
//	        	vTaskDelay(4000 / portTICK_PERIOD_MS);
//	        	continue;
//	        }
//
//	        printf("... connected \n");
//	        freeaddrinfo(res);
//	        if (write(s, rec_data.request, strlen(rec_data.request)) < 0) {
//	            printf("... socket send failed \n");
//	            close(s);
//	            vTaskDelay(4000 / portTICK_PERIOD_MS);
//	            continue;
//	        }
//	        printf("... socket send success \n");
//	        struct timeval receiving_timeout;
//	        receiving_timeout.tv_sec = 5;
//	        receiving_timeout.tv_usec = 0;
//	        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
//                sizeof(receiving_timeout)) < 0) {
//	            printf("... failed to set socket receiving timeout \n");
//	            close(s);
//	            vTaskDelay(4000 / portTICK_PERIOD_MS);
//	            continue;
//	        }
//	        printf("... set socket receiving timeout success\n");
//	        /* Read HTTP response */
//	        printf("Read HTTP response \n");
//	        do {
//	            bzero(recv_buf, sizeof(recv_buf));
//	            r = read(s, recv_buf, sizeof(recv_buf)-1);
//	            if(strstr(recv_buf,rec_data.update) == NULL){
//	            	printf("Updates are available \n");
//	            	printf("Downloading new version ... \n");
//	            	goto end;
//	            }
//	        } while(r > 0);
//	        printf("... done reading from socket. Last read return=%d errno=%d\r\n", r, err);
//	        close(s);
//	        end:
//				xTaskCreate(&http_download_task,"http_download_task",2048,NULL,6,&xhttp_download_Handle);
//				if(!xQueueSend(HttpDownload_Queue_Handle,&rec_data,portMAX_DELAY)){
//					printf("HTTP update task: Failed to send request to http download task \n");
//				}
//	        	if(xhttp_update_Handle != NULL){
//	        		vTaskDelete(xhttp_update_Handle);
//	        		printf("HTTP update task was deleted after using \n");
//	        	}
//
//	    }
//	}
//}
