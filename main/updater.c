/*
 * updater.c
 *
 *  Created on: Jul 30, 2018
 *      Author: hai_dotcom
 */

#include "updater.h"
extern xQueueHandle HttpDownload_Queue_Handle
void updater(void *pvParameters)
{
	data data;
	while(1){
		data.url = "http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt";
		data.name = "/sdcard/test.txt";
		data.version = "00";
		data.request = 	"GET http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt HTTP/1.0\r\n"
			    		"Host: "WEB_SERVER"\r\n"
						"User-Agent: esp-idf/1.0 esp32\r\n"
						"\r\n";
		if(xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY))
			printf("Failed to send requets to HTTP download\r\n");
		vTaskDelay(5000/portTICK_PERIOD_MS);
	}
}
