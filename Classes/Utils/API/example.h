#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "Utils/API/BaseURL.h"
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

/**
 * 示例：发送 GET 请求到 BASE_URL 获取字符串
 */
class Example {
 public:
  /**
   * 发送 GET 请求到 BASE_URL
   * @param path API 路径（可选，默认为空）
   * @param callback 回调函数，参数为获取到的字符串
   */
  static void getRequest(
      const std::string& path = "",
      std::function<void(const std::string&)> callback = nullptr);
};

#endif  // __EXAMPLE_H__
