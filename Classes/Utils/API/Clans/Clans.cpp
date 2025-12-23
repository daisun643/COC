#include "Utils/API/Clans/Clans.h"

#include "Utils/API/URLEncoder.h"
#include "json/document.h"

void Clans::getAllClansInfo(ClansListCallback callback) {
  // 构建完整 URL
  std::string url = BASE_URL + "/api/clans/all-info";

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      ClansListResult result;
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
        ClansListResult result;

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

            // 解析 data 字段（部落列表）
            if (doc.HasMember("data") && doc["data"].IsArray()) {
              const rapidjson::Value& dataArray = doc["data"];
              for (rapidjson::SizeType i = 0; i < dataArray.Size(); i++) {
                const rapidjson::Value& clanObj = dataArray[i];
                if (clanObj.IsObject()) {
                  ClanInfo clan;
                  if (clanObj.HasMember("id") &&
                      clanObj["id"].IsString()) {
                    clan.id = clanObj["id"].GetString();
                  }
                  if (clanObj.HasMember("name") &&
                      clanObj["name"].IsString()) {
                    clan.name = clanObj["name"].GetString();
                  }
                  if (clanObj.HasMember("member_count") &&
                      clanObj["member_count"].IsInt()) {
                    clan.member_count = clanObj["member_count"].GetInt();
                  }
                  result.clans.push_back(clan);
                }
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
          if (statusCode == 400) {
            result.message = "Bad request";
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

void Clans::searchClans(const std::string& name_keyword,
                        ClansListCallback callback) {
  // 构建完整 URL，添加 GET 参数（进行 URL 编码）
  std::string url = BASE_URL + "/api/clans/search";
  url += "?name=" + urlEncode(name_keyword);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      ClansListResult result;
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
        ClansListResult result;

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

            // 解析 data 字段（部落列表）
            if (doc.HasMember("data") && doc["data"].IsArray()) {
              const rapidjson::Value& dataArray = doc["data"];
              for (rapidjson::SizeType i = 0; i < dataArray.Size(); i++) {
                const rapidjson::Value& clanObj = dataArray[i];
                if (clanObj.IsObject()) {
                  ClanInfo clan;
                  if (clanObj.HasMember("id") &&
                      clanObj["id"].IsString()) {
                    clan.id = clanObj["id"].GetString();
                  }
                  if (clanObj.HasMember("name") &&
                      clanObj["name"].IsString()) {
                    clan.name = clanObj["name"].GetString();
                  }
                  if (clanObj.HasMember("member_count") &&
                      clanObj["member_count"].IsInt()) {
                    clan.member_count = clanObj["member_count"].GetInt();
                  }
                  result.clans.push_back(clan);
                }
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
          if (statusCode == 400) {
            result.message = "Bad request";
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

void Clans::createClan(const std::string& name, int owner_id, CreateClanCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/create";
  url += "?name=" + urlEncode(name);
  url += "&owner_id=" + std::to_string(owner_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request", "");
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
        std::string message = "Unknown error";
        std::string clan_id = "";

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message, clan_id);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }

            // 解析 clan_id 字段
            if (doc.HasMember("clan_id")) {
              if (doc["clan_id"].IsString()) {
                clan_id = doc["clan_id"].GetString();
              } else if (doc["clan_id"].IsInt()) {
                clan_id = std::to_string(doc["clan_id"].GetInt());
              }
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message, clan_id);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::joinClan(const std::string& clan_id, int user_id, JoinClanCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/join";
  url += "?clan_id=" + urlEncode(clan_id);
  url += "&user_id=" + std::to_string(user_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request");
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
        std::string message = "Unknown error";

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::getClanMembers(const std::string& clan_id, ClanMembersCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/members";
  url += "?clan_id=" + urlEncode(clan_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request", std::vector<std::string>());
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
        std::string message = "Unknown error";
        std::vector<std::string> members;

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message, members);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }

            // 解析 members 字段（成员列表）
            if (doc.HasMember("members") && doc["members"].IsArray()) {
              const rapidjson::Value& membersArray = doc["members"];
              for (rapidjson::SizeType i = 0; i < membersArray.Size(); i++) {
                if (membersArray[i].IsString()) {
                  members.push_back(membersArray[i].GetString());
                }
              }
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message, members);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::getClanChatMessages(const std::string& clan_id, int limit, ChatMessagesCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/chat/messages";
  url += "?clan_id=" + urlEncode(clan_id);
  if (limit > 0) {
    url += "&limit=" + std::to_string(limit);
  }

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request", std::vector<ChatMessage>(), 0);
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
        std::string message = "Unknown error";
        std::vector<ChatMessage> messages;
        int count = 0;

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message, messages, count);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }

            // 解析 count 字段
            if (doc.HasMember("count") && doc["count"].IsInt()) {
              count = doc["count"].GetInt();
            }

            // 解析 messages 字段（消息列表）
            if (doc.HasMember("messages") && doc["messages"].IsArray()) {
              const rapidjson::Value& messagesArray = doc["messages"];
              for (rapidjson::SizeType i = 0; i < messagesArray.Size(); i++) {
                const rapidjson::Value& msgObj = messagesArray[i];
                if (msgObj.IsObject()) {
                  ChatMessage chatMsg;
                  if (msgObj.HasMember("sender") && msgObj["sender"].IsString()) {
                    chatMsg.sender = msgObj["sender"].GetString();
                  }
                  if (msgObj.HasMember("content") && msgObj["content"].IsString()) {
                    chatMsg.content = msgObj["content"].GetString();
                  }
                  if (msgObj.HasMember("time") && msgObj["time"].IsString()) {
                    chatMsg.time = msgObj["time"].GetString();
                  }
                  messages.push_back(chatMsg);
                }
              }
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message, messages, count);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::sendClanChatMessage(const std::string& clan_id, int user_id, 
                                const std::string& content, SendChatMessageCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/chat/send";
  url += "?clan_id=" + urlEncode(clan_id);
  url += "&user_id=" + std::to_string(user_id);
  url += "&content=" + urlEncode(content);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request");
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
        std::string message = "Unknown error";

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::getClanOwner(const std::string& clan_id, GetClanOwnerCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/owner";
  url += "?clan_id=" + urlEncode(clan_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request", 0);
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
        std::string message = "Unknown error";
        int owner_id = 0;

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message, owner_id);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }

            // 解析 owner_id 字段
            if (doc.HasMember("owner_id") && doc["owner_id"].IsInt()) {
              owner_id = doc["owner_id"].GetInt();
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message, owner_id);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::leaveClan(const std::string& clan_id, int user_id, JoinClanCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/leave";
  url += "?clan_id=" + urlEncode(clan_id);
  url += "&user_id=" + std::to_string(user_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request");
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
        std::string message = "Unknown error";

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}

void Clans::disbandClan(const std::string& clan_id, int user_id, JoinClanCallback callback) {
  // 构建完整 URL，添加 GET 参数
  std::string url = BASE_URL + "/api/clans/disband";
  url += "?clan_id=" + urlEncode(clan_id);
  url += "&user_id=" + std::to_string(user_id);

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback(false, "Failed to create HTTP request");
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
        std::string message = "Unknown error";

        if (!response) {
          message = "Response is null";
          if (callback) {
            callback(success, message);
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
              success = doc["success"].GetBool();
            }

            // 解析 message 字段
            if (doc.HasMember("message") && doc["message"].IsString()) {
              message = doc["message"].GetString();
            }
          } else {
            // JSON 解析失败
            message = "Failed to parse response";
          }
        } else {
          // 响应数据为空
          if (statusCode == 400) {
            message = "Bad request";
          } else {
            message = "Empty response";
          }
        }

        // 调用回调函数
        if (callback) {
          callback(success, message);
        }
      });

  // 发送请求
  cocos2d::network::HttpClient::getInstance()->send(request);

  // 释放请求对象
  request->release();
}
