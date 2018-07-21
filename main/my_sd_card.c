/*
 * my_sd_card.c
 *
 *  Created on: Jun 14, 2018
 *      Author: hai_dotcom
 */

#include "my_sd_card.h"
#include "project_main.h"

extern xQueueHandle HttpDownload_Queue_Handle;
extern TaskHandle_t xhttp_download_Handle;

void sd_card_init()
{
	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	// GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
	// Internal pull-ups are not sufficient. However, enabling internal pull-ups
	// does make a difference some boards, so we do that here.

	gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
	gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
	gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
	gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
	gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
	        .format_if_mount_failed = false,
	        .max_files = 5,
	        .allocation_unit_size = 16 * 1024
	    };
	// Use settings defined above to initialize SD card and mount FAT filesystem.
	    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
	    // Please check its source code and implement error recovery when developing
	    // production applications.
	    sdmmc_card_t* card;
	    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

	    if (ret != ESP_OK) {
	        if (ret == ESP_FAIL) {
	            printf( "Failed to mount filesystem.\n");
	        } else {
	            printf("Failed to initialize the card (%d).\n",ret);
	        }
	    }
	    // Card has been initialized, print its properties
	    sdmmc_card_print_info(stdout, card);
}

/*Calculate size of file*/
long cal_size(FILE * file)
{
	fseek(file,0L,SEEK_END);
	long size = ftell(file);
	fseek(file,0L,SEEK_SET);
	return size;
}

/*Check the existence of the json file
 *If json isn't available, download it*/
bool check_JSON()
{

	data data;
	//TaskHandle_t xHttpHandle;
	FILE* json= fopen("/sdcard/json.txt", "r");
	if (json==NULL){
			printf("JSON file isn't available \r\n Downloading JSON ... \n ");
			data.url = "http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt";
			data.name = "/sdcard/json.txt";
			data.request= "GET http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt HTTP/1.0\r\n"
				    "Host: "WEB_SERVER"\r\n"
				    "User-Agent: esp-idf/1.0 esp32\r\n"
				    "\r\n";
			data.version = "0xxx";
			//xTaskCreate(&http_download_task,"http_download_task",2048,NULL,6,&xhttp_download_Handle);
			if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY)){
				printf("Failed to send request to HTTP download task \n");
			}
			return false;
		}
	else{
		printf("JSON file is available \n");
		//printf("Size: %ld bytes",cal_size(json));
		return true;
	}
	fclose(json);

}


