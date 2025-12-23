#include "Utils/API/Battle/Battle.h"

#include "Utils/API/BaseURL.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

void Battle::searchOpponent(
    int userId,
    std::function<void(bool success, const std::string& message,
                       int opponentId, const std::string& opponentName,
                       const std::string& mapJsonData)> callback) {
  // 构建完整 URL
  std::string url = BASE_URL + "/api/oppenet/get?user_id=" +
                    std::to_string(userId);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "创建请求失败", 0, "", "");
    }
    return;
  }

  // 设置请求属性
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](cocos2d::network::HttpClient* client,
                 cocos2d::network::HttpResponse* response) {
        bool success = false;
        std::string message = "";
        int opponentId = 0;
        std::string opponentName = "";
        std::string mapJsonData = "";

        if (response && response->getResponseCode() == 200) {
          // 获取响应数据
          std::vector<char>* buffer = response->getResponseData();
          if (buffer && !buffer->empty()) {
            std::string result(buffer->begin(), buffer->end());

            // 解析 JSON 响应
            rapidjson::Document doc;
            doc.Parse(result.c_str());

            if (!doc.HasParseError() && doc.IsObject()) {
              // 解析 success
              if (doc.HasMember("success") && doc["success"].IsBool()) {
                success = doc["success"].GetBool();
              }

              // 解析 message
              if (doc.HasMember("message") && doc["message"].IsString()) {
                message = doc["message"].GetString();
              }

              // 解析 opponent_id
              if (doc.HasMember("opponent_id") &&
                  doc["opponent_id"].IsInt()) {
                opponentId = doc["opponent_id"].GetInt();
              }

              // 解析 opponent_name
              if (doc.HasMember("opponent_name") &&
                  doc["opponent_name"].IsString()) {
                opponentName = doc["opponent_name"].GetString();
              }

              // 解析 map_data 并转换为 JSON 字符串
              if (doc.HasMember("map_data") && doc["map_data"].IsObject()) {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc["map_data"].Accept(writer);
                mapJsonData = buffer.GetString();
              }
            } else {
              message = "解析响应数据失败";
            }
          } else {
            message = "响应数据为空";
          }
        } else {
          int code = response ? response->getResponseCode() : 0;
          message = "请求失败，错误代码: " + std::to_string(code);
        }

        // 调用回调函数
        if (callback) {
          callback(success, message, opponentId, opponentName, mapJsonData);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

