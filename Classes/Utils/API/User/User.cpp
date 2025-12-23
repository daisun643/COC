#include "Utils/API/User/User.h"
#include "Utils/API/URLEncoder.h"
#include "json/document.h"

void UserAPI::setClanId(const std::string& user_id, const std::string& clan_id, SetClanCallback callback) {
  std::string url = BASE_URL + "/api/set_clanid";
  url += "?id=" + urlEncode(user_id);
  url += "&clan_id=" + urlEncode(clan_id);

  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) callback(false, "Failed to create HTTP request");
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback([callback](cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response) {
    bool success = false;
    std::string message = "Unknown error";

    if (!response) {
      if (callback) callback(false, "Response is null");
      return;
    }

    int statusCode = response->getResponseCode();
    std::vector<char>* buffer = response->getResponseData();
    if (buffer && !buffer->empty()) {
      std::string responseData(buffer->begin(), buffer->end());
      rapidjson::Document doc;
      doc.Parse(responseData.c_str());
      if (!doc.HasParseError() && doc.IsObject()) {
        if (doc.HasMember("success") && doc["success"].IsBool()) success = doc["success"].GetBool();
        if (doc.HasMember("message") && doc["message"].IsString()) message = doc["message"].GetString();
      } else {
        message = "Failed to parse response";
      }
    } else {
      if (statusCode == 400) message = "Bad request"; else message = "Empty response";
    }

    if (callback) callback(success, message);
  });

  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}

void UserAPI::getClanId(const std::string& user_id, GetClanCallback callback) {
  std::string url = BASE_URL + "/api/get_clanid";
  url += "?id=" + urlEncode(user_id);

  cocos2d::network::HttpRequest* request = new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) callback(false, "Failed to create HTTP request", "");
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback([callback](cocos2d::network::HttpClient* client, cocos2d::network::HttpResponse* response) {
    bool success = false;
    std::string message = "Unknown error";
    std::string clan_id = "";

    if (!response) {
      if (callback) callback(false, "Response is null", clan_id);
      return;
    }

    int statusCode = response->getResponseCode();
    std::vector<char>* buffer = response->getResponseData();
    if (buffer && !buffer->empty()) {
      std::string responseData(buffer->begin(), buffer->end());
      rapidjson::Document doc;
      doc.Parse(responseData.c_str());
      if (!doc.HasParseError() && doc.IsObject()) {
        if (doc.HasMember("success") && doc["success"].IsBool()) success = doc["success"].GetBool();
        if (doc.HasMember("message") && doc["message"].IsString()) message = doc["message"].GetString();
        if (doc.HasMember("clan_id")) {
          if (doc["clan_id"].IsString()) clan_id = doc["clan_id"].GetString();
          else if (doc["clan_id"].IsInt()) clan_id = std::to_string(doc["clan_id"].GetInt());
        }
      } else {
        message = "Failed to parse response";
      }
    } else {
      if (statusCode == 400) message = "Bad request"; else message = "Empty response";
    }

    if (callback) callback(success, message, clan_id);
  });

  cocos2d::network::HttpClient::getInstance()->send(request);
  request->release();
}
