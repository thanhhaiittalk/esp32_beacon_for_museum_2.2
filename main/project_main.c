/****************************************************************************

****************************************************************************/

#include "project_main.h"
#include "my_ibeacon.h"
#include "my_sd_card.h"
#include "http_download.h"
#include "updater.h"

xQueueHandle Beacon_Queue_Handle = 0;
xQueueHandle HttpDownload_Queue_Handle = 0;
//xQueueHandle HttpUpdate_Queue_Handle = NULL;
TaskHandle_t xhttp_download_Handle = NULL;
//TaskHandle_t xhttp_update_Handle = NULL;
//TaskHandle_t xread_downld_Handle = NULL;
TaskHandle_t xUpdater_Handle = NULL;

xSemaphoreHandle jsonSignal = NULL;
xSemaphoreHandle downldSignal = NULL;

bool update_flag = false;
bool json_downloaded = false;

void app_main()
{
	Beacon_Queue_Handle = xQueueCreate(3,sizeof(simple_beacon));
    HttpDownload_Queue_Handle = xQueueCreate(3,sizeof(data));
//    vSemaphoreCreateBinary(jsonSignal);
    downldSignal = xSemaphoreCreateMutex();
//    HttpUpdate_Queue_Handle = xQueueCreate(2,sizeof(data));
	ESP_ERROR_CHECK(nvs_flash_init());
    ibeacon_init();
    sd_card_init();
    initialise_wifi();
    xTaskCreate(&updater,"updater",2048,NULL,7,xUpdater_Handle);
    xTaskCreate(&http_download_task,"http_download_task",2048,NULL,6,xhttp_download_Handle);
    xTaskCreate(&action_inzone,"action_inzone",2048,NULL,5,NULL);
}
