#include "api_service.h"
#include <string>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <cJSON.h>


using namespace Service;
namespace {
  static const char *TAG = "api_svc";
}

esp_err_t Api::Start(httpd_handle_t server)
{
  httpd_uri_t apiConfigURI = {
    .uri = "/api/v1/config",
    .method = HTTP_GET,
    .handler = [](httpd_req_t *req) -> esp_err_t {
      httpd_resp_set_type(req, "application/json");
      cJSON *root = cJSON_CreateObject();

    #if CONFIG_KESPR_DEVMODE
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    #endif

    #if CONFIG_KESPR_KBD_ENABLED
      {
        cJSON *keyboardRoot = cJSON_AddObjectToObject(root, "keyboard");

        cJSON_AddBoolToObject(keyboardRoot, "enabled", true);
      }
    #endif

    #if CONFIG_KESPR_UARTD_ENABLED
      {
        cJSON *uartRoot = cJSON_AddObjectToObject(root, "uart");

        cJSON_AddBoolToObject(uartRoot, "enabled", true);
        cJSON_AddNumberToObject(uartRoot, "braud", CONFIG_KESPR_UARTD_DEFAULT_BRAUD);
      }
    #endif

      const char *configRsp = cJSON_PrintUnformatted(root);
      httpd_resp_sendstr(req, configRsp);

      free((void *)configRsp);
      cJSON_Delete(root);
      return ESP_OK;
    },
    .user_ctx = nullptr
  };
  httpd_register_uri_handler(server, &apiConfigURI);

  return ESP_OK;
}
