#include "api_service.h"
#include <string>
#include <functional>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <cJSON.h>
#include <defer.h>


using namespace Service;
namespace {
  static const char *TAG = "api_svc";

  using ApiHandlerFn = std::function<esp_err_t(ApiService::Context *ctx, httpd_req_t *req, cJSON *rsp)>;

  esp_err_t apiHandler(httpd_req_t *req, const ApiHandlerFn &fn)
  {
    cJSON *rsp = cJSON_CreateObject();
    if (rsp == nullptr) {
      ESP_LOGE(TAG, "unable to allocate result object: %s", esp_err_to_name(ESP_ERR_NO_MEM));
      return ESP_ERR_NO_MEM;
    }
    REF_DEFER(cJSON_Delete(rsp));

    ApiService::Context *ctx = static_cast<ApiService::Context*>(req->user_ctx);
    esp_err_t err = fn(ctx, req, rsp);
    if (err != ESP_OK) {
      if (!cJSON_HasObjectItem(rsp, "error")) {
        ESP_LOGE(TAG, "handler process fail: %s", esp_err_to_name(err));
        cJSON_AddStringToObject(rsp, "error", esp_err_to_name(err));
      } else {
        ESP_LOGE(TAG,
          "handler process fail [%d]: %s",
          err, cJSON_GetStringValue(cJSON_GetObjectItem(rsp, "error"))
        );
      }
    }

    const char *status;
    switch (err) {
    case ESP_OK:
      status = HTTPD_200;
      break;
    case ESP_ERR_NOT_FOUND:
      status = HTTPD_404;
      break;
    case HTTPD_400_BAD_REQUEST:
      status = HTTPD_404;
      break;
    default:
      status = HTTPD_500;
    }

    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  #if CONFIG_KESPR_DEVMODE
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  #endif

    const char *rspBody = cJSON_PrintUnformatted(rsp);
    err = httpd_resp_sendstr(req, rspBody);
    free((void *)rspBody);
    return err;
  }

  esp_err_t parseReqBody(httpd_req_t *req, ApiService::Context *ctx, cJSON **out)
  {
    if (out == nullptr) {
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "shit happens");
      ESP_LOGE(TAG, "trynying to parse request body w/o out ptr");
      return ESP_ERR_INVALID_ARG;
    }

    int bodySize = req->content_len;
    if (bodySize >= CONFIG_KESPR_HTTP_BUF_SIZE) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "content too long");
      return ESP_FAIL;
    }

    int curSize = 0;
    char *buf = ctx->scratch;
    int received = 0;
    while (curSize < bodySize) {
        received = httpd_req_recv(req, buf + curSize, bodySize);
        if (received <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        curSize += received;
    }
    buf[bodySize] = '\0';

    *out = cJSON_ParseWithOpts(buf, NULL, true);
    return ESP_OK;
  }

  bool isServiceEnabled(ApiService::Context *ctx, const std::string& service)
  {
    auto const &svcItem = ctx->services.find(service);
    return svcItem != ctx->services.end();
  }

  bool isServiceStarted(ApiService::Context *ctx, const std::string& service)
  {
    auto const &svcItem = ctx->services.find(service);
    if (svcItem == ctx->services.end()) {
      return false;
    }

    return svcItem->second->Started();
  }

  esp_err_t startService(ApiService::Context *ctx, const std::string& service)
  {
    if (ctx->curService == service) {
      return ESP_OK;
    }

    auto const &svcItem = ctx->services.find(service);
    if (svcItem == ctx->services.end()) {
      return ESP_ERR_NOT_FOUND;
    }

    for (auto const& [name, svc] : ctx->services)
    {
      esp_err_t err = svc->Stop();
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to stop service: %s", name.c_str());
        continue;
      }

      ESP_LOGI(TAG, "service stoped before start another one: %s", name.c_str());
    }

    esp_err_t err = svcItem->second->Start();
    ESP_RETURN_ON_ERROR(err, TAG, "unable to start service: %s", service.c_str());

    ESP_LOGI(TAG, "service started %s", service.c_str());
    ctx->curService = service;
    return ESP_OK;
  }

  esp_err_t serviceStartHandler(httpd_req_t *req)
  {
    return apiHandler(req, [](ApiService::Context *ctx, httpd_req_t *req, cJSON *rsp) -> esp_err_t {
      cJSON *reqBody = nullptr;
      esp_err_t err = parseReqBody(req, ctx, &reqBody);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to parse request body: %s", esp_err_to_name(err));
        cJSON_AddStringToObject(rsp, "error", "invalid JSON in request body");
        return ESP_ERR_INVALID_ARG;
      }

      if (reqBody == nullptr) {
        cJSON_AddStringToObject(rsp, "error", "no request body");
        return ESP_ERR_INVALID_ARG;
      }

      char* svcName = cJSON_GetStringValue(cJSON_GetObjectItem(reqBody, "name"));
      if (svcName == nullptr) {
        cJSON_AddStringToObject(rsp, "error", "req[name] is required name request body");
        return ESP_ERR_INVALID_ARG;
      }

      ESP_RETURN_ON_ERROR(startService(ctx, svcName), TAG, "start service failed: %s", svcName);
      cJSON_AddBoolToObject(rsp, "ok", true);

      return ESP_OK;
    });
  }

  esp_err_t configHandler(httpd_req_t *req)
  {
    return apiHandler(req, [](ApiService::Context *ctx, httpd_req_t *req, cJSON *rsp) -> esp_err_t {
      {
        std::string serviceName = "usbkb";
        cJSON *usbkbRoot = cJSON_AddObjectToObject(rsp, serviceName.c_str());
        if (usbkbRoot == nullptr) {
          return ESP_ERR_NO_MEM;
        }

        cJSON_AddBoolToObject(usbkbRoot, "enabled", isServiceEnabled(ctx, serviceName));
        cJSON_AddBoolToObject(usbkbRoot, "started", isServiceStarted(ctx, serviceName));
      }

      {
        std::string serviceName = "uart";
        cJSON *uartRoot = cJSON_AddObjectToObject(rsp, serviceName.c_str());
        if (uartRoot == nullptr) {
          return ESP_ERR_NO_MEM;
        }

        cJSON_AddBoolToObject(uartRoot, "enabled", isServiceEnabled(ctx, serviceName));
        cJSON_AddBoolToObject(uartRoot, "started", isServiceStarted(ctx, serviceName));
        cJSON_AddNumberToObject(uartRoot, "braud", CONFIG_KESPR_UARTD_DEFAULT_BRAUD);
      }

      return ESP_OK;
    });
  }

  esp_err_t notFoundHandler(httpd_req_t *req)
  {
    return apiHandler(req, [](ApiService::Context *ctx, httpd_req_t *req, cJSON *rsp) -> esp_err_t {
      (void)(ctx);
      (void)(req);
      (void)(rsp);
      return ESP_ERR_NOT_FOUND;
    });
  }
}

esp_err_t ApiService::Initialize(httpd_handle_t server)
{
  httpd_uri_t apiConfigURI = {
    .uri = "/api/v1/config",
    .method = HTTP_GET,
    .handler = configHandler,
    .user_ctx = &this->ctx
  };
  httpd_register_uri_handler(server, &apiConfigURI);

  httpd_uri_t apiStartURI = {
    .uri = "/api/v1/service/start",
    .method = HTTP_POST,
    .handler = serviceStartHandler,
    .user_ctx = &this->ctx
  };
  httpd_register_uri_handler(server, &apiStartURI);

  httpd_uri_t api404URI = {
    .uri = "/api/*",
    .method = HTTP_GET,
    .handler = notFoundHandler,
    .user_ctx = &this->ctx
  };
  httpd_register_uri_handler(server, &api404URI);

  return BaseService::Initialize(server);
}

esp_err_t ApiService::RegisterService(BaseService *svc)
{
  this->ctx.services[svc->Name()] = svc;
  return ESP_OK;
}
