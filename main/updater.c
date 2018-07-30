/*
 * updater.c
 *
 *  Created on: Jul 30, 2018
 *      Author: hai_dotcom
 */

#include "updater.h"

extern xQueueHandle HttpDownload_Queue_Handle;
//extern xSemaphoreHandle jsonSignal;
extern bool JSON_done;
extern TaskHandle_t xUpdater_Handle;
bool check_file(char * file_name)
{
	bool flag = false;
	FILE *file = fopen(file_name,"r");
	if(file == NULL)
		flag = not_available;
	else{
		flag = available;
	}

	fclose(file);
	return flag;
}

void send_JSON_request()
{
	data data;
	data.url = "http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt";
	data.name = "/sdcard/json.txt";
	data.version = "00";
	data.request = 	"GET http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt HTTP/1.0\r\n"
		    		"Host: "WEB_SERVER"\r\n"
					"User-Agent: esp-idf/1.0 esp32\r\n"
					"\r\n";
	if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY))
		printf("Failed to send request to HTTP download\r\n");
	else
		printf("Send request to download JSON \r\n");
}

void parseJSON_downld_data()
{
	printf("parse json \r\n");
	vTaskDelete(xUpdater_Handle);

}

void updater(void *pvParameters)
{
	static bool send_flag = false;
	while(1){
		if(check_file("/sdcard/json.txt") == not_available && send_flag == false){
			printf("UPDATER: JSON is not available\r\n");
			send_JSON_request();
			send_flag = true;
		}
		else{
			if(!send_flag){
				printf("UPDATER: JSON file was available \r\n");
				//if out_of_date
					//remove json
				//else delete task updater
			}
		}
		if(JSON_done && send_flag){
			printf("UPDATER: JSON was downloaded \r\n");
			parseJSON_downld_data();
		}
	}
}
