#include "static_handler.h"
#include <string>
#include <fcntl.h>

#include <sdkconfig.h>
#include <esp_spiffs.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <esp_vfs.h>


using namespace HttpD;
namespace {
  static const char *TAG = "httpd::static";
  static const char *kStaticPath = "/www";
  static const char *kStaticPartition = "www";

  typedef struct {
    char scratch[CONFIG_KESPR_HTTP_BUF_SIZE];
  } ServeContext;

  esp_err_t mountStatic()
  {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = kStaticPath,
      .partition_label = kStaticPartition,
      .max_files = 20,
      .format_if_mount_failed = false
    };
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    ESP_RETURN_ON_ERROR(err, TAG, "initialize SPIFFS: %s", esp_err_to_name(err));

    size_t total = 0, used = 0;
    err = esp_spiffs_info(kStaticPartition, &total, &used);
    ESP_RETURN_ON_ERROR(err, TAG, "get SPIFFS partition information: %s", esp_err_to_name(err));
    ESP_LOGI(TAG, "static partition size: total: %d, used: %d", total, used);

    return ESP_OK;
  }

  esp_err_t setFileHeaders(httpd_req_t *req, const std::string& filepath)
  {
    bool forceCache = true;
    const char *type = "application/octet-stream";
    if (filepath.ends_with(".html")) {
      type = "text/html";
      forceCache = false;
    } else if (filepath.ends_with(".js")) {
      type = "application/javascript";
    } else if (filepath.ends_with(".css")) {
      type = "text/css";
    } else if (filepath.ends_with(".png")) {
      type = "image/png";
    } else if (filepath.ends_with(".ico")) {
      type = "image/x-icon";
    } else if (filepath.ends_with(".webp")) {
      type = "image/webp";
    } else if (filepath.ends_with(".svg")) {
      type = "text/xml";
    }

    if (forceCache) {
      esp_err_t err = httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=604800");
      if (err != ESP_OK) {
        return err;
      }
    }

    return httpd_resp_set_type(req, type);
  }

  esp_err_t setGzipFileHeaders(httpd_req_t *req, const std::string& filepath)
  {
    bool forceCache = true;
    const char *type = "application/octet-stream";
    if (filepath.ends_with(".html.gz")) {
      type = "text/html";
      forceCache = false;
    } else if (filepath.ends_with(".js.gz")) {
      type = "application/javascript";
    } else if (filepath.ends_with(".css.gz")) {
      type = "text/css";
    } else if (filepath.ends_with(".png.gz")) {
      type = "image/png";
    } else if (filepath.ends_with(".ico.gz")) {
      type = "image/x-icon";
    } else if (filepath.ends_with(".webp.gz")) {
      type = "image/webp";
    } else if (filepath.ends_with(".svg.gz")) {
      type = "text/xml";
    }

    if (forceCache) {
      esp_err_t err = httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=604800");
      if (err != ESP_OK) {
        return err;
      }
    }

    esp_err_t err = httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    if (err != ESP_OK) {
      return err;
    }

    return httpd_resp_set_type(req, type);
  }

  bool isAcceptGzip(httpd_req_t *req)
  {
  #if CONFIG_KESPR_HTTP_FORCE_GZIP
    return true;
  #else

    ServeContext *ctx = static_cast<ServeContext *>(req->user_ctx);
    char *acceptVal = ctx->scratch;
    if (httpd_req_get_hdr_value_str(req, "Accept-Encoding", acceptVal, CONFIG_KESPR_HTTP_BUF_SIZE) != ESP_OK) {
      return false;
    }

    return strstr(acceptVal, "gzip");
    #endif
  }

  esp_err_t staticHandler(httpd_req_t *req)
  {
    std::string filepath = kStaticPath;
    if (!strstr(req->uri, ".")) {
      filepath += "/index.html";
    } else {
      filepath += req->uri;
    }

    const bool useGzip = isAcceptGzip(req);
    if (useGzip) {
      filepath += ".gz";
    }

    int fd = open(filepath.c_str(), O_RDONLY, 0);
    if (fd == -1) {
      ESP_LOGW(TAG, "failed to open file: %s", filepath.c_str());
      httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "file not found");
      return ESP_FAIL;
    }

    if (useGzip) {
      setGzipFileHeaders(req, filepath);
    } else {
      setFileHeaders(req, filepath);
    }

    ServeContext *ctx = static_cast<ServeContext *>(req->user_ctx);
    char *chunk = ctx->scratch;
    ssize_t read_bytes;
    do {
      /* Read file in chunks into the scratch buffer */
      read_bytes = read(fd, chunk, CONFIG_KESPR_HTTP_BUF_SIZE);
      if (read_bytes == -1) {
        ESP_LOGE(TAG, "failed to read file: %s", filepath.c_str());
      } else if (read_bytes > 0) {
        /* Send the buffer contents as HTTP response chunk */
        if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
          close(fd);
          ESP_LOGE(TAG, "File sending failed!");
          /* Abort sending file */
          httpd_resp_sendstr_chunk(req, nullptr);
          /* Respond with 500 Internal Server Error */
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "failed to send file");
          return ESP_FAIL;
        }
      }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
  }
}

esp_err_t StaticHandler::Register(httpd_handle_t server)
{
  ESP_LOGI(TAG, "mount fs");
  ESP_RETURN_ON_ERROR(mountStatic(), TAG, "mount fs");

  static ServeContext ctx = {};
  httpd_uri_t rootURI = {
      .uri = "/*",
      .method = HTTP_GET,
      .handler = staticHandler,
      .user_ctx = &ctx
  };
  return httpd_register_uri_handler(server, &rootURI);
}
