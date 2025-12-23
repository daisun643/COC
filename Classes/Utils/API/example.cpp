#include "Utils/API/example.h"

void Example::getRequest(const std::string& path,
                         std::function<void(const std::string&)> callback) {
  // 构建完整 URL
  std::string url = BASE_URL;
  if (!path.empty()) {
    if (path[0] != '/') {
      url += "/";
    }
    url += path;
  }

  // 创建 HTTP 请求
  cocos2d::network::HttpRequest* request =
      new (std::nothrow) cocos2d::network::HttpRequest();
  if (!request) {
    if (callback) {
      callback("");
    }
    return;
  }

  // 设置请求属性
  request->setUrl(url.c_str());
  request->setRequestType(cocos2d::network::HttpRequest::Type::GET);
  request->setResponseCallback(
      [callback](cocos2d::network::HttpClient* client,
                 cocos2d::network::HttpResponse* response) {
        std::string result = "";

        if (response && response->getResponseCode() == 200) {
          // 获取响应数据
          std::vector<char>* buffer = response->getResponseData();
          if (buffer && !buffer->empty()) {
            result.assign(buffer->begin(), buffer->end());
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
