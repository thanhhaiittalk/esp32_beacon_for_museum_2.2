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
#define JSON_name	"/sdcard/json.txt"
#endif /* MAIN_UPDATER_H_ */
