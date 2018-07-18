/****************************************************************************

****************************************************************************/

#include "project_main.h"
#include "my_ibeacon.h"
#include "my_sd_card.h"
#include "http_download.h"
#include "parse_JSON.h"

xQueueHandle Beacon_Queue_Handle = 0;
xQueueHandle HttpDownload_Queue_Handle = 0;
xQueueHandle HttpUpdate_Queue_Handle = NULL;
TaskHandle_t xhttp_download_Handle = NULL;
TaskHandle_t xhttp_update_Handle = NULL;

void app_main()
{
	Beacon_Queue_Handle = xQueueCreate(3,sizeof(simple_beacon));
    HttpDownload_Queue_Handle = xQueueCreate(3,sizeof(data));
    HttpUpdate_Queue_Handle = xQueueCreate(2,sizeof(data));
	ESP_ERROR_CHECK(nvs_flash_init());
    ibeacon_init();
    sd_card_init();
    initialise_wifi();
    read_JSON();
    xTaskCreate(&action_inzone,"action_inzone",2048,NULL,7,NULL);
}
