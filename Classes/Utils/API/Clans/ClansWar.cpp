#include "Utils/API/Clans/ClansWar.h"

#include "Utils/API/URLEncoder.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

void ClansWar::startWar(const std::string& clans_id,
                        StartWarCallback callback) {
  std::string url = BASE_URL + "/api/clanswar/start";
  url += "?clans_id=" + urlEncode(clans_id);

  auto request = new (std::nothrow) network::HttpRequest();
  if (!request) {
    if (callback) callback(false, "Failed to create HTTP request", "", "");
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](network::HttpClient* client, network::HttpResponse* response) {
        bool success = false;
        std::string message = "Unknown error";
        std::string war_id = "";
        std::string history_path = "";

        if (!response) {
          if (callback) callback(false, "Response is null", "", "");
          return;
        }

        int statusCode = response->getResponseCode();
        std::vector<char>* buffer = response->getResponseData();
        if (buffer && !buffer->empty()) {
          std::string resp(buffer->begin(), buffer->end());
          rapidjson::Document doc;
          doc.Parse(resp.c_str());
          if (!doc.HasParseError() && doc.IsObject()) {
            if (doc.HasMember("success") && doc["success"].IsBool())
              success = doc["success"].GetBool();
            if (doc.HasMember("message") && doc["message"].IsString())
              message = doc["message"].GetString();
            if (doc.HasMember("war_id") && doc["war_id"].IsString())
              war_id = doc["war_id"].GetString();
            if (doc.HasMember("history_path") && doc["history_path"].IsString())
              history_path = doc["history_path"].GetString();
          } else {
            message = "Failed to parse response";
          }
        } else {
          if (statusCode == 400)
            message = "Bad request";
          else
            message = "Empty response";
        }

        if (callback) callback(success, message, war_id, history_path);
      });

  network::HttpClient::getInstance()->send(request);
  request->release();
}

void ClansWar::getWarOverview(const std::string& clans_id,
                              WarOverviewCallback callback) {
  std::string url = BASE_URL + "/api/clanswar/overview";
  url += "?clans_id=" + urlEncode(clans_id);

  auto request = new (std::nothrow) network::HttpRequest();
  if (!request) {
    if (callback) {
      WarOverviewResult r;
      r.success = false;
      r.message = "Failed to create HTTP request";
      callback(r);
    }
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](network::HttpClient* client, network::HttpResponse* response) {
        WarOverviewResult result;
        if (!response) {
          result.success = false;
          result.message = "Response is null";
          if (callback) callback(result);
          return;
        }
        int statusCode = response->getResponseCode();
        std::vector<char>* buffer = response->getResponseData();
        if (buffer && !buffer->empty()) {
          std::string resp(buffer->begin(), buffer->end());
          rapidjson::Document doc;
          doc.Parse(resp.c_str());
          if (!doc.HasParseError() && doc.IsObject()) {
            if (doc.HasMember("success") && doc["success"].IsBool())
              result.success = doc["success"].GetBool();
            if (doc.HasMember("message") && doc["message"].IsString())
              result.message = doc["message"].GetString();
            if (doc.HasMember("overview") && doc["overview"].IsObject()) {
              const rapidjson::Value& ov = doc["overview"];
              for (auto itr = ov.MemberBegin(); itr != ov.MemberEnd(); ++itr) {
                WarMapOverview m;
                m.id = itr->name.GetString();
                const rapidjson::Value& v = itr->value;
                if (v.HasMember("stars") && v["stars"].IsInt())
                  m.stars = v["stars"].GetInt();
                if (v.HasMember("cnt") && v["cnt"].IsInt())
                  m.cnt = v["cnt"].GetInt();
                result.maps.push_back(m);
              }
            }
          } else {
            result.success = false;
            result.message = "Failed to parse response";
          }
        } else {
          result.success = false;
          if (statusCode == 400)
            result.message = "Bad request";
          else
            result.message = "Empty response";
        }
        if (callback) callback(result);
      });

  network::HttpClient::getInstance()->send(request);
  request->release();
}

void ClansWar::getWarMap(const std::string& clans_id, const std::string& map_id,
                         GetMapCallback callback) {
  std::string url = BASE_URL + "/api/clanswar/map";
  url += "?clans_id=" + urlEncode(clans_id);
  url += "&map_id=" + urlEncode(map_id);

  auto request = new (std::nothrow) network::HttpRequest();
  if (!request) {
    if (callback) callback(false, "Failed to create HTTP request", "");
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](network::HttpClient* client, network::HttpResponse* response) {
        bool success = false;
        std::string message = "Unknown error";
        std::string data = "";
        if (!response) {
          if (callback) callback(false, "Response is null", "");
          return;
        }
        int statusCode = response->getResponseCode();
        std::vector<char>* buffer = response->getResponseData();
        if (buffer && !buffer->empty()) {
          std::string resp(buffer->begin(), buffer->end());
          rapidjson::Document doc;
          doc.Parse(resp.c_str());
          if (!doc.HasParseError() && doc.IsObject()) {
            if (doc.HasMember("success") && doc["success"].IsBool())
              success = doc["success"].GetBool();
            if (doc.HasMember("message") && doc["message"].IsString())
              message = doc["message"].GetString();
            if (doc.HasMember("data")) {
              // 将 data 原样返回为字符串
              rapidjson::StringBuffer bufferOut;
              rapidjson::Writer<rapidjson::StringBuffer> writer(bufferOut);
              doc["data"].Accept(writer);
              data = bufferOut.GetString();
            }
          } else {
            message = "Failed to parse response";
          }
        } else {
          if (statusCode == 400)
            message = "Bad request";
          else
            message = "Empty response";
        }
        if (callback) callback(success, message, data);
      });

  network::HttpClient::getInstance()->send(request);
  request->release();
}

void ClansWar::saveWarMap(const std::string& clans_id,
                          const std::string& map_id,
                          const std::string& map_data_json,
                          SaveMapCallback callback) {
  std::string url = BASE_URL + "/api/clanswar/map/save";
  url += "?clans_id=" + urlEncode(clans_id);
  url += "&map_id=" + urlEncode(map_id);
  url += "&map_data=" + urlEncode(map_data_json);

  auto request = new (std::nothrow) network::HttpRequest();
  if (!request) {
    if (callback) callback(false, "Failed to create HTTP request");
    return;
  }

  request->setUrl(url.c_str());
  request->setRequestType(network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](network::HttpClient* client, network::HttpResponse* response) {
        bool success = false;
        std::string message = "Unknown error";
        if (!response) {
          if (callback) callback(false, "Response is null");
          return;
        }
        int statusCode = response->getResponseCode();
        std::vector<char>* buffer = response->getResponseData();
        if (buffer && !buffer->empty()) {
          std::string resp(buffer->begin(), buffer->end());
          rapidjson::Document doc;
          doc.Parse(resp.c_str());
          if (!doc.HasParseError() && doc.IsObject()) {
            if (doc.HasMember("success") && doc["success"].IsBool())
              success = doc["success"].GetBool();
            if (doc.HasMember("message") && doc["message"].IsString())
              message = doc["message"].GetString();
          } else {
            message = "Failed to parse response";
          }
        } else {
          if (statusCode == 400)
            message = "Bad request";
          else
            message = "Empty response";
        }
        if (callback) callback(success, message);
      });

  network::HttpClient::getInstance()->send(request);
  request->release();
}
