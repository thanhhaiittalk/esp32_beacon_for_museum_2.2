/*
 * http_download.h
 *
 *  Created on: Jul 3, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_HTTP_DOWNLOAD_H_
#define MAIN_HTTP_DOWNLOAD_H_

//http request library
#include "project_main.h"

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "stdbool.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "sdkconfig.h"

/*Define*/
#define WEB_SERVER "www.stream.esy.es"
#define WEB_PORT 80

#define WIFI_SSID	"Hai Dotcom"
#define WIFI_PASS 	"doremonnobita"

/*Prototype*/
void http_download_task(void *pvParameters);
void http_check_update_task(void *pvParameters);
void initialise_wifi(void);
esp_err_t event_handler(void *ctx, system_event_t *event);

#endif /* MAIN_HTTP_DOWNLOAD_H_ */
