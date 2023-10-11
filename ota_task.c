#include "ota_task.h"

void start_ota_task()
{
    xTaskCreate(&ota_task_callback, "ota_task_callback", OTA_TASK_STACK_SIZE, NULL, OTA_TASK_PRIORITY, &task_ota_handle);
}

void suspend_ota_task()
{
    vTaskSuspend(task_ota_handle);
}

void resume_ota_task()
{
    vTaskResume(task_ota_handle);
}

void delete_ota_task()
{
    vTaskDelete(task_ota_handle);
}

void ota_task_callback(void *Params)
{
    ota_err_t rc;
    IoT_Publish_Message_Params paramsQOS1;
    // rc=run_ota();
    while (1)
    {
        EventBits_t uxBits;
        uxBits = xEventGroupWaitBits(
            Internal_Events, /* The event group being tested. */
            OTA_MODE_EVENT,  /* The bits within the event group to wait for. */
            pdTRUE,          /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,         /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY);

        if ((uxBits & OTA_MODE_EVENT) != 0)
        {
            // rc=
            // store_set_process_indication_state(OTA_MODE_IND);
            rc = run_ota();
            nvs_set_ota_url("");
            if (rc != OTA_OK)
            {
                // Publish Failed message to aws iot
                mqtt_report_ota_failure(ota_percent);
                change_operational_mode(NORMAL_MODE);
            }
            // Shutdown Modem and Restart ESP
            // change_operational_mode(MANUAL_MODE);
            // ESP_LOGI("TEST OTA", "ITS HERE");
            // xEventGroupSetBits(Internal_Events, OTA_VERIFIED_EVENT);
            // if(rc!=OTA_OK && rc!=OTA_FAIL_SAME_FW_VERSION )
            // {

            // }
        }
    }
}