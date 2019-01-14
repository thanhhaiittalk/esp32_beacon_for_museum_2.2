/*
 * my_ibeacon.h
 *
 *  Created on: Jun 14, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_MY_IBEACON_H_
#define MAIN_MY_IBEACON_H_

//Declare library
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "cJSON.h"

#include "controller.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_ibeacon_api.h"
#include "esp_err.h"


#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "aplay.h"

//Struct
typedef struct{
	uint8_t uuid[16];
	uint16_t minor;
	uint16_t major;
	int16_t rssi;
}simple_beacon;

//Declare prototype
void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void process_beacon_array(esp_ble_ibeacon_t *ibeacon_data_recieved,uint8_t number_of_scanning);
void bubbleSort(esp_ble_ibeacon_t *ibeacon_array, uint8_t number_of_beacons);
void action_inzone(void *parameter);
void action_process(simple_beacon beacon_now, simple_beacon beacon_new);
void ibeacon_init();
bool introduce(simple_beacon beacon);
bool check_database(char * beacon_id, const char * const json_string);


#endif /* MAIN_MY_IBEACON_H_ */
