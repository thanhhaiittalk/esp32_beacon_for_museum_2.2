/*
 * project_main.h
 *
 *  Created on: Jun 14, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_PROJECT_MAIN_H_
#define MAIN_PROJECT_MAIN_H_

//Declare library
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "controller.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event_loop.h"
#include "esp_log.h"


#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/*My library*/
#include "http_download.h"
#include "my_ibeacon.h"
#include "my_sd_card.h"
#include "parse_JSON.h"

/*Define*/
#define not_available 	false
#define available 		true

/*Struct*/
typedef struct{
	char *name;
	char *url;
	char *request;
	char *version;
}data;

#endif /* MAIN_PROJECT_MAIN_H_ */
