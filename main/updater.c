/*
 * updater.c
 *
 *  Created on: Jul 30, 2018
 *      Author: hai_dotcom
 */

#include "updater.h"

extern bool JSON_done;
extern bool new_version;

extern xQueueHandle HttpDownload_Queue_Handle;
extern xQueueHandle HttpUpdate_Queue_Handle;

extern TaskHandle_t xUpdater_Handle;
extern TaskHandle_t xHttp_download_Handle;
extern TaskHandle_t xHttp_update_Handle;
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
	const char* json_string = read_JSON(JSON_name);
	data data;
	const cJSON *artifacts;
	const cJSON *artifact;

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
			if(cJSON_IsString(base) && (base -> valuestring != NULL)){

				int url_size = strlen("http://www.stream.esy.es/database/data/hcm_fine_arts_museum//english/sound/")
								+strlen(base->valuestring)*2+4;
				data.url = calloc(url_size,sizeof(char));
				sprintf(data.url,"http://www.stream.esy.es/database/data/hcm_fine_arts_museum/%s/english/sound/%s.mp3",
						base->valuestring,base->valuestring);
				//printf("%s\n",data.url);

				int request_size = url_size
				+ strlen("GET  HTTP/1.0\r\nHost: www.stream.esy.es \r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n") +10;
				data.request = calloc(request_size,sizeof(char));
				sprintf(data.request,"GET %s HTTP/1.0\r\nHost: www.stream.esy.es\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n",
						data.url);
				//printf("%s\n",data.request);
			}
			if(cJSON_IsNumber(id)){
				int name_size = strlen("/sdcard/x.mp3");
				data.name=calloc(name_size,sizeof(char));
				sprintf(data.name,"/sdcard/%d.mp3",id->valueint);
				//printf("%s\n",data.name);
			}
			else goto end;

			data.version = "xxx";
			while(xSemaphoreTake(downldSignal,portMAX_DELAY) == pdFALSE);
			if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY))
				printf("Failed to send request to HTTP download\r\n");
			else
				printf("Send request to download data \r\n");
		}
		goto end;
	}
end:
	printf("Download data complete\nDelete http download task \n");
	cJSON_Delete(muse_json);
	vTaskDelete(xHttp_download_Handle);
	vTaskDelete(xParseJSON_Handle);

}

/*Parse old JSON to delete all old files before downloading new version*/
void parseJSON_delete(){
	const char* json_string = read_JSON(JSON_name);
	const cJSON *artifacts;
	const cJSON *artifact;
	cJSON *muse_json = cJSON_Parse(json_string);
	artifacts = cJSON_GetObjectItemCaseSensitive(muse_json,"artifacts");
	cJSON_ArrayForEach(artifact,artifacts){
		cJSON *id = cJSON_GetObjectItemCaseSensitive(artifact,"id");
		if(cJSON_IsNumber(id)){
			int name_size = strlen("/sdcard/x.mp3");
			char* name=calloc(name_size,sizeof(char));
			sprintf(name,"/sdcard/%d.mp3",id->valueint);
			unlink(name);
		}else goto end;
	}
end:
	cJSON_Delete(muse_json);
}

/*Read version from JSON*/
char* read_version(){
	char* version;
	const char* json_string = read_JSON(JSON_name);
	const cJSON *update;
	cJSON *muse_json = cJSON_Parse(json_string);
    update = cJSON_GetObjectItemCaseSensitive(muse_json, "update");
    version = (char*)calloc(16,sizeof(char));
    if (cJSON_IsString(update) && (update->valuestring != NULL))
    {
        printf("Checking update \"%s\"\n", update->valuestring);
        sprintf(version,"%s",update->valuestring);
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
		if(check_file(JSON_name) == not_available && send_flag == false){
			printf("UPDATER: JSON is not available\r\n");
			send_JSON_request();
			send_flag = true;
		}
		else{
			if(!send_flag){
				printf("UPDATER: JSON file was available \r\n");
				static char* version;
				 xTaskCreate(&http_check_update_task,"check update task",2048,NULL,6,xHttp_update_Handle);
				if(xSemaphoreTake(updateSignal,portMAX_DELAY)){
					version = calloc(16,sizeof(char));
					version = read_version();
					if(!xQueueSend(HttpUpdate_Queue_Handle,&version,portMAX_DELAY))
						printf("Failed to send request to check update task\n");
				}

				if(new_version == available){
					parseJSON_delete();
					unlink("/sdcard/json.txt");
					printf("UPDATER: Delete old JSON\n");
				}else{
					vTaskDelete(xHttp_download_Handle);
					vTaskDelete(xHttp_update_Handle);
					vTaskDelete(xUpdater_Handle);
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
