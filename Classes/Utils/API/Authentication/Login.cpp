#include "Utils/API/Authentication/Login.h"

#include "Utils/API/URLEncoder.h"
#include "json/document.h"

void Login::login(const int& id, const std::string& password,
                  LoginCallback callback) {
  // 构建完整 URL，添加 GET 参数（进行 URL 编码）
  std::string url = BASE_URL + "/api/login";
  url += "?id=" + std::to_string(id);
  url += "&password=" + urlEncode(password);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      LoginResult result;
      result.success = false;
      result.message = "Failed to create HTTP request";
      callback(result);
    }
    return;
  }

  // 设置请求属性
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](cocos2d::network::HttpClient* client,
                 cocos2d::network::HttpResponse* response) {
        LoginResult result;

        if (!response) {
          result.success = false;
          result.message = "Response is null";
          if (callback) {
            callback(result);
          }
          return;
        }

        int statusCode = response->getResponseCode();

        // 获取响应数据
        std::vector<char>* buffer = response->getResponseData();
        if (buffer && !buffer->empty()) {
          std::string responseData(buffer->begin(), buffer->end());

          // 解析 JSON 响应
          rapidjson::Document doc;
          doc.Parse(responseData.c_str());

          if (!doc.HasParseError() && doc.IsObject()) {
            // 解析 success 字段
            if (doc.HasMember("success") && doc["success"].IsBool()) {
              result.success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              result.message = doc["message"].GetString();
            }

            // 解析 user_id 字段
            if (doc.HasMember("name")) {
              if (doc["name"].IsString()) {
                result.name = doc["name"].GetString();
              } else if (doc["name"].IsNull()) {
                result.name = "";
              }
            }
          } else {
            // JSON 解析失败
            result.success = false;
            result.message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          result.success = false;
          if (statusCode == 401) {
            result.message = "Authentication failed";
          } else {
            result.message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(result);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}
