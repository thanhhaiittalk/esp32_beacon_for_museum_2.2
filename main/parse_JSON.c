/*
 * parse_JSON.c
 *
 *  Created on: Jul 17, 2018
 *      Author: hai_dotcom
 */

#include "parse_JSON.h"
#include "my_sd_card.h"
#include "http_download.h"
#include "project_main.h"

extern xQueueHandle HttpDownload_Queue_Handle;
extern xTaskHandle xhttp_download_Handle;
char temp_update[32];
extern bool update_flag;

void read_JSON_for_checking_version()
{
	char line[64];
	char * json_string;
	FILE * json_file;
	if(check_JSON() == true){
		json_file = fopen("/sdcard/json.txt","r");
		fseek(json_file,0L,SEEK_END);
		long size = ftell(json_file);
		printf("size: %ld \n",size);
		fseek(json_file,0L,SEEK_SET);
		json_string =(char*) calloc(size,sizeof(char*));
		if(json_file == NULL){
			printf("Failed to open json for reading \n");
		}
		while(fgets(line,sizeof(line),json_file)!=NULL){
			strcat(json_string,line);
		}
		fclose(json_file);
		printf(json_string);
		check_update(json_string);
		printf("Check: exit http download task \n");
		free(json_string);
	}
}

void check_update(const char * const json)
{
	data data;
	const cJSON *update = NULL;
    const cJSON *artifacts = NULL;

    cJSON *muse_json = cJSON_Parse(json);
    if (muse_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        update_flag = false;
        goto end;
    }
    update = cJSON_GetObjectItemCaseSensitive(muse_json, "update");
    if (cJSON_IsString(update) && (update->valuestring != NULL))
    {
        printf("Checking update \"%s\"\n", update->valuestring);
        sprintf(temp_update,"%s",update->valuestring);
        data.version = temp_update;
        printf("v:%s t:%s\n",data.version,temp_update);
    }
    data.url = "http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt";
   	data.name = "/sdcard/json.txt";
   	data.request= "GET http://www.stream.esy.es/database/data/hcm_fine_arts_museum/overview/hcm_fine_arts_museum.txt HTTP/1.0\r\n"
   					"Host: "WEB_SERVER"\r\n"
					"User-Agent: esp-idf/1.0 esp32\r\n"
					"\r\n";

   	if(!xQueueSend(HttpDownload_Queue_Handle,&data,portMAX_DELAY)){
   		printf("Parse JSON: Failed to send request to http_check_update_task \n");
   	}
   	else
   		printf("Parse JSON: Successfully send request to http_check_update_task \n");
	end:
		cJSON_Delete(muse_json);
}

void read_JSON_for_downloading(void *pdparameter)
{
//	data new_data;
//	new_data.version = "0xxx";
//	new_data.name = "/sdcard/test.mp3";
//	new_data.url = "xxx";
//	new_data.request = "GET http://www.stream.esy.es/database/data/hcm_fine_arts_museum/phuoc_long_operation/english/sound/phuoc_long_operation.mp3 HTTP/1.0\r\n"
//						"Host: "WEB_SERVER"\r\n"
//						"User-Agent: esp-idf/1.0 esp32\r\n"
//						"\r\n";
// 	if(!xQueueSend(HttpDownload_Queue_Handle,&new_data,portMAX_DELAY)){
//   		printf("read JSON: Failed to send request to http_check_update_task \n");
//   	}
//   	else
//   		printf("read JSON: Successfully send request to http_check_update_task \n");
	while(1){
		printf("read json task \n");
	}
}
