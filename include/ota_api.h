#ifndef _OTA_API_H_
#define _OTA_API_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "esp_log.h"

#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"

#include "esp_ota_ops.h"

#include "store.h"
#include "store_rtos.h"
#include "store_api.h"
#include "op_mode_api.h"
#include "mqtt_api.h"
#include "mqtt_publish_responses.h"

#include "modem_network_api.h"

#include "ota_err.h"

#define TAG_OTA "OTA"

#define OTA_URL_SIZE 100

uint8_t ota_percent;

/**
 * @brief ROOT CA Certificate for AWS
 *
 */
extern const uint8_t server_cert_pem_start[] asm("_binary_root_ca_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_root_ca_pem_end");

/**
 * @brief HTTP Client event handler
 *
 * @param evt HTTP Client events data struct
 * @return ota_err_t
 */
ota_err_t client_event_handler(esp_http_client_event_t *evt);

/**
 * @brief Validate if the FW version is new or old
 *
 * @param incoming_ota_desc OTA image description struct
 * @return ota_err_t
 */
ota_err_t validate_image_header(esp_app_desc_t *incoming_ota_desc);

/**
 * @brief Perform OTA update
 *
 * @return ota_err_t
 */
ota_err_t run_ota();

/**
 * @brief Check FW version and save it in store
 *
 */
void check_ota_version();

#endif
