/*
 * parse_JSON.h
 *
 *  Created on: Jul 17, 2018
 *      Author: hai_dotcom
 */

#ifndef MAIN_PARSE_JSON_H_
#define MAIN_PARSE_JSON_H_
/*Declare library*/
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "cJSON.h"

/*Prototype*/
void check_update(const char * const database);
void read_JSON_for_checking_version();
void read_JSON_for_downloading(void *pdparameter);
#endif /* MAIN_PARSE_JSON_H_ */
