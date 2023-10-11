#ifndef _OTA_TASK_H_
#define _OTA_TASK_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"

#include "store.h"
#include "store_rtos.h"
#include "ota_api.h"
#include "ota_err.h"
#include "ppp_aws_iot_api.h"
#include "mqtt_api.h"
#include "mqtt_action_decision_engine.h"
#include "mqtt_publish_responses.h"
#include "op_mode_api.h"

/**
 * @brief Start OTA task
 * 
 */
void start_ota_task();

/**
 * @brief Suspend OTA task
 * 
 */
void suspend_ota_task();

/**
 * @brief Resume OTA Task
 * 
 */
void resume_ota_task();

/**
 * @brief Delete OTA task
 * 
 */
void delete_ota_task();


void ota_task_callback(void*Params);

#endif