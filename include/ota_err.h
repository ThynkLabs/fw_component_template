#ifndef _OTA_ERR_H_
#define _OTA_ERR_H_

#include <stdio.h>

#include "esp_log.h"

typedef int ota_err_t;
#define OTA_FAIL -1
#define OTA_OK 0
#define OTA_FAIL_SAME_FW_VERSION -2
#define OTA_FAIL_NETWORK_NOT_CONNECTED -3


#endif