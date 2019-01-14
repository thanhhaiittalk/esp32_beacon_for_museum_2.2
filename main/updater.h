/*
 * updater.h
 *
 *  Created on: Jul 30, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_UPDATER_H_
#define MAIN_UPDATER_H_

/*Declare libraries*/
#include "project_main.h"
#include "cJSON.h"
#include "my_sd_card.h"

/*Prototype*/
void updater(void *pvParameters);
void send_JSON_request();
bool check_file(char * file_name);
void parseJSON_downld_data(void *pvParameters);
void parseJSON_delete();
char* read_JSON(char* json_name);
char* read_version();



/*Define*/
#define JSON_NAME			"/sdcard/json.txt"
#define WEB_URL				"https://host767.000webhostapp.com/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt"
#define WEB_SERVER			"host767.000webhostapp.com"
#define WEB_PORT 			80
#define WEB_URL_BASE		"https://"WEB_SERVER"/?dir=data/hcm_fine_arts_museum//english/sound/"
#define WEB_URL_CONTENT		"https://"WEB_SERVER"/?dir=data/hcm_fine_arts_museum/%s/english/sound/%s.wav"
#define	WEB_REQUEST			"GET HTTP/1.0\r\nHost: "WEB_SERVER" \r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n"
#define WEB_URL_REQUEST		"GET %s HTTP/1.0\r\nHost: "WEB_SERVER"\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n"
#define WIFI_SSID			"Hai Dotcom"
#define WIFI_PASS 			"doremonnobita"

#endif /* MAIN_UPDATER_H_ */
