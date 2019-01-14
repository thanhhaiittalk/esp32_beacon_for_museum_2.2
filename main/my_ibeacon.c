/*
 * my_ibeacon.c
 *
 *  Created on: Jun 14, 2018
 *      Author: hai_dotcom
 */

#include "my_ibeacon.h"

static const char* DEMO_TAG = "IBEACON";
extern esp_ble_ibeacon_vendor_t vendor_config;
extern xQueueHandle Beacon_Queue_Handle;

//(IBEACON_MODE == IBEACON_RECEIVER)
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30
};

void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:{
    	//for BEACON_SENDER only
        break;
    }
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
#if (IBEACON_MODE == IBEACON_RECEIVER)
        //the unit of the duration is second, 0 means scan permanently
        uint32_t duration = 0;
        esp_ble_gap_start_scanning(duration);
#endif
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(DEMO_TAG, "Scan start failed");
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //adv start complete event to indicate adv start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(DEMO_TAG, "Adv start failed");
        }
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            /* Search for BLE iBeacon Packet */
            if (esp_ble_is_ibeacon_packet(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len)){
                esp_ble_ibeacon_t *ibeacon_data = (esp_ble_ibeacon_t*)(scan_result->scan_rst.ble_adv);
                static uint8_t number_of_scannings = 0;
                printf("________________________________________________________________________ \n");
                ESP_LOGI(DEMO_TAG, "----------iBeacon Found----------");
                esp_log_buffer_hex("IBEACON_DEMO: Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN );
                esp_log_buffer_hex("IBEACON_DEMO: Proximity UUID:", ibeacon_data->ibeacon_vendor.proximity_uuid, ESP_UUID_LEN_128);

                uint16_t major = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.major);
                uint16_t minor = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.minor);
                ibeacon_data->ibeacon_vendor.rssi = scan_result->scan_rst.rssi;
                ESP_LOGI(DEMO_TAG, "Major: 0x%04x (%d)", major, major);
                ESP_LOGI(DEMO_TAG, "Minor: 0x%04x (%d)", minor, minor);
                //ESP_LOGI(DEMO_TAG, "Measured power (RSSI at a 1m distance):%d dbm", ibeacon_data->ibeacon_vendor.measured_power);
                ESP_LOGI(DEMO_TAG, "RSSI of packet:%d dbm", ibeacon_data->ibeacon_vendor.rssi);
                number_of_scannings++;
                if(number_of_scannings <= 5){
                	process_beacon_array(ibeacon_data,number_of_scannings);
                	if(number_of_scannings == 5){
                		number_of_scannings = 0;
                		//vTaskDelay(2000/portTICK_PERIOD_MS);
                		printf("\n----------------------Complete a session---------------------- \n");
                	}
                }
            }
            break;
        default:
            break;
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(DEMO_TAG, "Scan stop failed");
        }
        else {
            ESP_LOGI(DEMO_TAG, "Stop scan successfully");
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(DEMO_TAG, "Adv stop failed");
        }
        else {
            ESP_LOGI(DEMO_TAG, "Stop adv successfully");
        }
        break;

    default:
        break;
    }
}

void ble_ibeacon_appRegister(void)
{
    esp_err_t status;

    ESP_LOGI(DEMO_TAG, "register callback");

    //register the scan callback function to the gap module
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        ESP_LOGE(DEMO_TAG, "gap register error, error code = %x", status);
        return;
    }
}

void ibeacon_init()
{
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    /*ble ibeacon init*/
    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_ibeacon_appRegister();
    /* set scan parameters */
    esp_ble_gap_set_scan_params(&ble_scan_params);
}

void bubbleSort(esp_ble_ibeacon_t *ibeacon_array, uint8_t number_of_beacons)
{
	uint8_t i,j;
	esp_ble_ibeacon_t *temp;
	temp = calloc(1,sizeof(esp_ble_ibeacon_t));
	printf("Run bubble sort \n");
	for(i=0;i<number_of_beacons-1;i++){
		for(j=0;j<number_of_beacons-i-1;j++){
			if((ibeacon_array+j)->ibeacon_vendor.rssi < (ibeacon_array+j+1)->ibeacon_vendor.rssi){
				printf("check crash \n");
				*temp = *(ibeacon_array+j);
				*(ibeacon_array+j) = *(ibeacon_array+j+1);
				*(ibeacon_array+j+1)= *temp;
				free(temp);
			}
		}
	}
}

void process_beacon_array(esp_ble_ibeacon_t *ibeacon_data_recieved,uint8_t number_of_scanning)
{
	static esp_ble_ibeacon_t *ibeacon_array;
	static uint8_t number_of_beacons = 0;
	simple_beacon beacon;
	bool check = false;
	printf("processing beacon array - number of scanning: %d \n",number_of_scanning);
	if(number_of_scanning == 1){
		printf("Check 1\n");
		ibeacon_array= calloc(5,sizeof(esp_ble_ibeacon_t));
		*ibeacon_array=*ibeacon_data_recieved;
		uint16_t major = ENDIAN_CHANGE_U16(ibeacon_array->ibeacon_vendor.major);
		uint16_t minor = ENDIAN_CHANGE_U16(ibeacon_array->ibeacon_vendor.minor);
		printf("ibeacon_array[0]->major: %d , minor: %d \n",major,minor);
		number_of_beacons++;
	}
	else if(number_of_scanning >1 && number_of_scanning <= 5){
		printf("Check 2 \n");
		int i = 0;

		//Check if new beacon received was available in array
		for(i = 0;i<number_of_beacons;i++){
			printf("for loop ... \n");
			uint16_t major_received = ENDIAN_CHANGE_U16(ibeacon_data_recieved->ibeacon_vendor.major);
			uint16_t major_in_arr = ENDIAN_CHANGE_U16((ibeacon_array+i)->ibeacon_vendor.major);
			uint16_t minor_received = ENDIAN_CHANGE_U16(ibeacon_data_recieved->ibeacon_vendor.minor);
			uint16_t minor_in_arr = ENDIAN_CHANGE_U16((ibeacon_array+i)->ibeacon_vendor.minor);
			printf("beacon received-> major: %d minor: %d - beacon_arr[%d]->major: %d minor: %d \n",
					major_received,minor_received,i,major_in_arr,minor_in_arr);
			if(	major_in_arr == major_received && minor_in_arr == minor_received ){
				check = false;
				printf("Check  = false \n");
				break;
			}
			else{
				printf("Check  = true \n");
				check= true;
			}
		}

		if(check==true){
			printf("check available \n");
			*(ibeacon_array+number_of_beacons)=*ibeacon_data_recieved;
			number_of_beacons++;
		}
		if(number_of_scanning == 5){
			printf("Check 3\n");
			bubbleSort(ibeacon_array,number_of_beacons);
			printf("number of beacons: %d \n",number_of_beacons);
			printf("max of RSSI: %d\n",(int)ibeacon_array->ibeacon_vendor.rssi);
			if(number_of_beacons>1){
				printf("2nd RSSI: %d\n",(int)(ibeacon_array+1)->ibeacon_vendor.rssi);
			}

			beacon.major = ENDIAN_CHANGE_U16(ibeacon_array->ibeacon_vendor.major);
			beacon.minor = ENDIAN_CHANGE_U16(ibeacon_array->ibeacon_vendor.minor);
			beacon.rssi  = (int)ibeacon_array->ibeacon_vendor.rssi;
			for(uint8_t i = 0; i < 16; i++){
				beacon.uuid[i] = ibeacon_array->ibeacon_vendor.proximity_uuid[i];
				printf("%x",beacon.uuid[i]);
			}

			if(! xQueueSend(Beacon_Queue_Handle,&beacon,portMAX_DELAY)){
				printf("Failed to send \n");
			}
			number_of_beacons = 0;
			free(ibeacon_array);
		}
	}
}

/*Combine beacon id into a string*/
void combine_beacon_id(simple_beacon beacon, char * beacon_id)
{
	beacon_id = (char*)calloc(40,sizeof(char*));

	sprintf(beacon_id,"%2x%2x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x_%02d_%04d",beacon.uuid[0],beacon.uuid[1],beacon.uuid[2],beacon.uuid[3],
	beacon.uuid[4],beacon.uuid[5],beacon.uuid[6],beacon.uuid[7],beacon.uuid[8],beacon.uuid[9],beacon.uuid[10],beacon.uuid[11],
	beacon.uuid[12],beacon.uuid[13],beacon.uuid[14],beacon.uuid[15],beacon.major,beacon.minor);
}

bool check_database(char * beacon_id, const char * const json_string)
{

	bool result;
	cJSON * artifacts = NULL;
	cJSON * artifact = NULL;
	char file_play[20];
	cJSON * muse_json = cJSON_Parse(json_string);
	printf("check_database function: %s \r\n",beacon_id);

	if(muse_json == NULL){
		const char * error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL){
			printf("Error before: %s\r\n", error_ptr);
		}
		printf("json not found \r\n");
		return false;
	}
	artifacts = cJSON_GetObjectItemCaseSensitive(muse_json, "artifacts");
	cJSON_ArrayForEach(artifact, artifacts){
		cJSON * id = cJSON_GetObjectItemCaseSensitive(artifact, "id");
		cJSON * beacon = cJSON_GetObjectItemCaseSensitive(artifact, "beacon");
		if(cJSON_IsString(beacon)){
			printf("%s\r\n",beacon->valuestring);
			if(strstr(beacon_id, beacon->valuestring) != NULL){
				sprintf(file_play,"/sdcard/%d.wav",id->valueint);
				aplay_wav(&file_play);
				printf("Check database: Found \r\n");
				return true;
			}
			else
			{
				result = false;
				printf("Check database: Not Found\r\n");
			}
		}
	}
	return result;
}

bool introduce(simple_beacon beacon)
{
	bool result = false;
	char * json_string;
	char line[64];
	char * beacon_id;

	/*Get JSON string*/
	FILE * json_file = fopen("/sdcard/json.txt","r");
    if (json_file==NULL){
    	printf("Failed to open json file for reading\r\n");
    	return false;
    }
    fseek(json_file,0L,SEEK_END);
    long size = ftell(json_file);
    printf("size: %ld \n",size);
    fseek(json_file,0L,SEEK_SET);
    json_string =(char*) calloc(size,sizeof(char*));
    while(fgets(line,sizeof(line),json_file)!=NULL){
    	strcat(json_string,line);
    }

    /*Combine beacon ID into a string*/
    beacon_id = (char*)calloc(40,sizeof(char*));

    sprintf(beacon_id,"%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x_%d_%d",beacon.uuid[0],beacon.uuid[1],beacon.uuid[2],beacon.uuid[3],
    		beacon.uuid[4],beacon.uuid[5],beacon.uuid[6],beacon.uuid[7],beacon.uuid[8],beacon.uuid[9],beacon.uuid[10],beacon.uuid[11],
			beacon.uuid[12],beacon.uuid[13],beacon.uuid[14],beacon.uuid[15],beacon.major,beacon.minor);
    printf("beacon recv: %s\r\n",beacon_id);

    /*Check database to play audio*/
    if(check_database(beacon_id, json_string) == false){
    	return false;
    }

    free(beacon_id);
    free(json_string);
	return result;
};

void action_process(simple_beacon beacon_now, simple_beacon beacon_new)
{
	char id[50];
	if(beacon_now.major != beacon_new.major && beacon_now.minor != beacon_new.minor){
		beacon_now = beacon_new;
		printf("action_process: new beacon \r\n");
		introduce(beacon_new);
	}
	else{
		/*do nothing*/
		return;
	}
}

void action_inzone(void *parameter)
{
	simple_beacon beacon_new;
	simple_beacon beacon_now;
	while(1){
		if(xQueueReceive(Beacon_Queue_Handle,&beacon_new,portMAX_DELAY)){
			action_process(beacon_now,beacon_new);
			printf("action_inzone: action process done \r\n");
		}
		else{
			printf("Failed to received \n");
		}
	}
}

