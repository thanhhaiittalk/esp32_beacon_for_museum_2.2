/****************************************************************************

****************************************************************************/

#include "project_main.h"
#include "my_ibeacon.h"
#include "my_sd_card.h"
#include "http_download.h"
#include "updater.h"
#include "aplay.h"

xQueueHandle Beacon_Queue_Handle = 0;
xQueueHandle HttpDownload_Queue_Handle = 0;
xQueueHandle HttpUpdate_Queue_Handle = 0;
xQueueHandle Audio_Queue_Handle = 0;


TaskHandle_t xHttp_download_Handle = NULL;
TaskHandle_t xHttp_update_Handle = NULL;
TaskHandle_t xUpdater_Handle = NULL;
TaskHandle_t xAudio_task_handle = NULL;

xSemaphoreHandle jsonSignal = NULL;
xSemaphoreHandle downldSignal = NULL;
xSemaphoreHandle updateSignal = NULL;

bool json_downloaded = false;

bool download_complete = false;

void app_main()
{
	Beacon_Queue_Handle = xQueueCreate(3,sizeof(simple_beacon));
    HttpDownload_Queue_Handle = xQueueCreate(1,sizeof(data));
    HttpUpdate_Queue_Handle = xQueueCreate(1,sizeof(char*));

    downldSignal = xSemaphoreCreateMutex();
    updateSignal = xSemaphoreCreateMutex();

	ESP_ERROR_CHECK(nvs_flash_init());
    sd_card_init();
    initialise_wifi();
    WM8978_config();
/*
	ibeacon_init();
    xTaskCreate(&action_inzone,"action_inzone",4096,NULL,5,NULL);
*/

    xTaskCreate(&updater,"updater",2048,NULL,7,xUpdater_Handle);
    xTaskCreate(&http_download_task,"http_download_task",2048,NULL,6,xHttp_download_Handle);

}
