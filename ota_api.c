#include "ota_api.h"

ota_err_t client_event_handler(esp_http_client_event_t *evt)
{
  ESP_LOGI(TAG_OTA, "EVENT ERROR CODE : %d", evt->event_id);
  switch (evt->event_id)
  {
  case HTTP_EVENT_ERROR:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG_OTA, "HTTP_EVENT_DISCONNECTED");
    break;
  }

  return OTA_OK;
}

ota_err_t validate_image_header(esp_app_desc_t *incoming_ota_desc)
{
  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  esp_app_desc_t running_partition_description;
  esp_ota_get_partition_description(running_partition, &running_partition_description);

  ESP_LOGI(TAG_OTA, "current version is %s\n", running_partition_description.version);
  ESP_LOGI(TAG_OTA, "new version is %s\n", incoming_ota_desc->version);
  // Save Firmware Version in nvs and set op_mode flag to Normal Mode

  if (strcmp(running_partition_description.version, incoming_ota_desc->version) == 0)
  {
    ESP_LOGW(TAG_OTA, "NEW VERSION IS THE SAME AS CURRENT VERSION. ABORTING");
    return OTA_FAIL_SAME_FW_VERSION;
  }
  return OTA_OK;
}

ota_err_t run_ota()
{

  ota_err_t rc;
  ESP_LOGI(TAG_OTA, "Invoking OTA");
  unsigned int timer = 10;
  float progress;
  ota_percent = 0;
  uint8_t prev_percent = 0;
  while (store_get_modem_network_state() != NET_CONNECTED && timer >= 0)
  {
    timer--;
    if (store_get_modem_network_state() == NET_CONNECTED)
    {
      break;
    }
    else if (timer <= 0)
    {
      return OTA_FAIL_NETWORK_NOT_CONNECTED;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  ESP_LOGI(TAG_OTA, "NET OK");
  size_t len = OTA_URL_SIZE;
  char *urlTest = malloc(len);

  memset(urlTest, '\0', sizeof(char) * len);
  nvs_get_ota_url(urlTest, len);

  ESP_LOGI("TEST_OTA", "OTA_URL:%s", urlTest);
  // free(urlTest);

  esp_http_client_config_t clientConfig = {
      .url = urlTest, // seed_pump.bin" // our ota location
      .event_handler = client_event_handler,
      // .timeout_ms=60000,
      .buffer_size = 4296,
      .buffer_size_tx = 4296,
      .keep_alive_enable = true,
      // // .keep_alive_enable=true,
      .keep_alive_count = 5,
      .keep_alive_interval = 5,
      .cert_pem = (char *)server_cert_pem_start};

  esp_https_ota_config_t ota_config = {
      .http_config = &clientConfig};

  esp_https_ota_handle_t ota_handle = NULL;
  ESP_LOGI(TAG_OTA, "CONFIG OK");

  rc = esp_https_ota_begin(&ota_config, &ota_handle);
  if (rc != OTA_OK)
  {
    ESP_LOGE(TAG_OTA, "esp_https_ota_begin failed");
    // run normally
    return rc;
  }
  ESP_LOGI(TAG_OTA, "OTA BEGIN OK");

  esp_app_desc_t incoming_ota_desc;
  rc = esp_https_ota_get_img_desc(ota_handle, &incoming_ota_desc);
  if (rc != OTA_OK)
  {
    ESP_LOGE(TAG_OTA, "esp_https_ota_get_img_desc failed");
    esp_https_ota_finish(ota_handle);
    return rc;
  }
  ESP_LOGI(TAG_OTA, "OTA IMG DESC OK");

  rc = validate_image_header(&incoming_ota_desc);
  if (rc != OTA_OK)
  {
    ESP_LOGE(TAG_OTA, "validate_image_header failed");
    esp_https_ota_finish(ota_handle);
    return rc;
  }

  ESP_LOGI(TAG_OTA, "OTA IMG HEADER OK");
  int image_size = esp_https_ota_get_image_size(ota_handle);

  // Wait for OTA to finish
  while (true)
  {
    ESP_LOGI(TAG_OTA, "Performing OTA......");
    ota_err_t ota_result = esp_https_ota_perform(ota_handle);
    ESP_LOGI(TAG_OTA, "Image bytes read: %d", esp_https_ota_get_image_len_read(ota_handle));
    progress = ((float)esp_https_ota_get_image_len_read(ota_handle) / (float)esp_https_ota_get_image_size(ota_handle));
    ota_percent = (float)progress * 100;
    ESP_LOGI(TAG_OTA, "OTA Progress(in percent): %d", ota_percent);
    // On Each Fetch send OTA Percentage
    if (ota_percent != prev_percent)
    {
      mqtt_report_ota_downloading((uint8_t)ota_percent);
      prev_percent = (float)progress * 100;
    }
    if (ota_result != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
      break;
  }
  // ON OTA FINISH or Error, report this as update finish/fail to Shadow.
  mqtt_report_ota_updating((uint8_t)ota_percent);
  if (esp_https_ota_finish(ota_handle) != OTA_OK)
  {
    ESP_LOGE(TAG_OTA, "esp_https_ota_finish failed");
    free(urlTest);
    return rc;
  }
  else
  {
    mqtt_report_ota_success();
    printf("restarting in 5 seconds\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
    // change_operational_mode();
    change_operational_mode(NORMAL_MODE);
    free(urlTest);
    return OTA_OK;
  }
  ESP_LOGE(TAG_OTA, "Failed to update firmware");
  free(urlTest);
  return OTA_FAIL;
}

void check_ota_version()
{
  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  esp_app_desc_t running_partition_description;
  esp_ota_get_partition_description(running_partition, &running_partition_description);
  printf("current firmware version is: %s\n", running_partition_description.version);
  // return  &running_partition_description.version;
}
