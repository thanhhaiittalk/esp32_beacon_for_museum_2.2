/*
 * updater.c
 *
 *  Created on: Jul 30, 2018
 *      Author: hai_dotcom
 */

#include "updater.h"

extern bool JSON_done;
extern bool new_version;
extern bool check_update;

extern xQueueHandle HttpDownload_Queue_Handle;
extern xQueueHandle HttpUpdate_Queue_Handle;

extern TaskHandle_t xUpdater_Handle;
extern TaskHandle_t xHttp_download_Handle;
extern TaskHandle_t xHttp_update_Handle;

extern bool download_complete;
TaskHandle_t xParseJSON_Handle = NULL;

extern xSemaphoreHandle downldSignal;
extern xSemaphoreHandle updateSignal;

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
	data.url = JSON_URL;
	data.name = JSON_NAME;
	data.request = 	"GET "JSON_URL" HTTP/1.0\r\n"
		    		"Host: "WEB_SERVER"\r\n"
					"User-Agent: esp-idf/1.0 esp32\r\n"
					"\r\n";
	if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY))
		printf("Failed to send request to HTTP download\r\n");
	else
		printf("Send request to download JSON \r\n");
}

char* read_JSON(char* json_name)
{
	char* json_string;
	char line[64];
	FILE* json_file = fopen(json_name,"r");
	if(json_file == NULL)
		printf("Failed to open JSON for reading \r\n");
	fseek(json_file,0L,SEEK_END);
	long size = ftell(json_file);
	fseek(json_file,0L,SEEK_SET);
	printf("Size : %ld\n",size);
	json_string = (char*)calloc(size,sizeof(char));
	while(fgets(line,sizeof(line),json_file)!=NULL){
		strcat(json_string,line);
	}
	fclose(json_file);
	return json_string;
}

void parseJSON_downld_data(void *pvParameters)
{
	//printf("parse json to download\r\n");
	const char* json_string = read_JSON(JSON_NAME);
	data data;
	const cJSON *artifacts;
	const cJSON *artifact;

	uint8_t url_size = 0;
	cJSON *muse_json = cJSON_Parse(json_string);
	if(muse_json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			fprintf(stderr,"Error before: %s\n",error_ptr);
		}
		goto end;

	}
	artifacts = cJSON_GetObjectItemCaseSensitive(muse_json,"artifacts");
	while(1){
		cJSON_ArrayForEach(artifact,artifacts){
			cJSON *base = cJSON_GetObjectItemCaseSensitive(artifact,"base");
			cJSON *id = cJSON_GetObjectItemCaseSensitive(artifact,"id");
			printf("parseJSON_downld_data: %d",id->valueint);
			if(cJSON_IsString(base) && (base -> valuestring != NULL)){
				/*Calculate string length of URL and */
				url_size = strlen(WEB_URL_BASE) + strlen(base->valuestring)*2+4;
				data.url = calloc(url_size,sizeof(char));
				sprintf(data.url,WEB_URL_CONTENT,base->valuestring,base->valuestring);

				int request_size = url_size + strlen(WEB_REQUEST) +10;
				data.request = calloc(request_size,sizeof(char));
				sprintf(data.request,WEB_URL_REQUEST,data.url);
				printf("data request %s\r\n",data.request);
				printf("dat url: %s\r\n", data.url);
			}
			if(cJSON_IsNumber(id)){
				int name_size = strlen("/sdcard/xxx.wav");
				data.name=calloc(name_size,sizeof(char));
				sprintf(data.name,"/sdcard/%d.wav",id->valueint);
			}
			else{
				goto end;
			}

			while(xSemaphoreTake(downldSignal,portMAX_DELAY) == pdFALSE);
			if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY))
				printf("Failed to send request to HTTP download\r\n");
			else
				printf("Send request to download data %d \r\n", id->valueint);
		}

		while(xSemaphoreTake(downldSignal,portMAX_DELAY) == pdFALSE);
end:
		printf("Download data complete\nDelete http download task \n");
		cJSON_Delete(muse_json);
		vTaskDelete(xHttp_download_Handle);
		vTaskDelete(xParseJSON_Handle);
		break;
	}

}

/*Parse old JSON to delete all old files before downloading new version*/
void parseJSON_delete(){
	printf("parse json to delete \n");
	const char* json_string = read_JSON(JSON_NAME);
	const cJSON *artifacts;
	const cJSON *artifact;
	cJSON *muse_json = cJSON_Parse(json_string);
	artifacts = cJSON_GetObjectItemCaseSensitive(muse_json,"artifacts");
	cJSON_ArrayForEach(artifact,artifacts){
		cJSON *id = cJSON_GetObjectItemCaseSensitive(artifact,"id");
		if(cJSON_IsNumber(id)){
			int name_size = strlen("/sdcard/xxx.wav");
			char* name=calloc(name_size,sizeof(char));
			sprintf(name,"/sdcard/%d.wav",id->valueint);
			if(unlink(name)==0)
				printf("Delete file %s successful\n",name);
			else
				printf("Delete file %s failed\n",name);
		}else goto end;
	}
end:
	cJSON_Delete(muse_json);
}

/*Read version from JSON*/
char* read_version(){
	char* version;
	const char* json_string = read_JSON(JSON_NAME);
	const cJSON *update;
	cJSON *muse_json = cJSON_Parse(json_string);
    update = cJSON_GetObjectItemCaseSensitive(muse_json, "update");
    version = (char*)calloc(20,sizeof(char));
    if (cJSON_IsString(update) && (update->valuestring != NULL))
    {
//        printf("Checking update \"%s\"\n", update->valuestring);
        sprintf(version,"%s",update->valuestring);
        printf(version);
    }else goto end;
    cJSON_Delete(muse_json);
    return version;
end:
	cJSON_Delete(muse_json);
	return "error";
}

void updater(void *pvParameters)
{
	static bool send_flag = false;
	while(1){
		//printf("Task Updater \r\n");
		if(check_file(JSON_NAME) == not_available && send_flag == false){
			printf("UPDATER: JSON is not available\r\n");
			send_JSON_request();
			send_flag = true;
		}else{
			if(!send_flag){
				printf("UPDATER: JSON file was available \r\n");
				static char* version;
				xTaskCreate(&http_check_update_task,"check update task",2048,NULL,6,xHttp_update_Handle);
				if(xSemaphoreTake(updateSignal,portMAX_DELAY)){
					version = (char*)calloc(20,sizeof(char));
					version = read_version();
					printf(version);
					if(!xQueueSend(HttpUpdate_Queue_Handle,&version,portMAX_DELAY))
						printf("Failed to send request to check update task\n");
				}
				if(check_update == true){
					if(new_version == available ){
						parseJSON_delete();
						unlink("/sdcard/json.txt");
					}else{
						printf("Update not available \r\n");
						printf("Delete task \n");
						download_complete = true;
						vTaskDelete(xHttp_download_Handle);
						vTaskDelete(xUpdater_Handle);
					}
				}
			}
		}
		if(JSON_done && send_flag){
			printf("UPDATER: JSON was downloaded \r\n");
			xTaskCreate(&parseJSON_downld_data,"parseJSON to download data",2048,NULL,4,xParseJSON_Handle);
			vTaskDelay(2/portTICK_PERIOD_MS);
			vTaskDelete(xUpdater_Handle);
		}
	}
}
