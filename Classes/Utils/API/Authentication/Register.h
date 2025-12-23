#ifndef __REGISTER_H__
#define __REGISTER_H__

#include "Utils/API/BaseURL.h"
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

/**
 * 注册响应结果结构
 */
struct RegisterResult {
  bool success;
  std::string message;
  int user_id;

  RegisterResult() : success(false), user_id(0) {}
};

/**
 * 注册 API 类
 * 发送 GET 请求到 /api/register
 */
class Register {
 public:
  /**
   * 注册回调函数类型
   * @param result 注册结果
   */
  typedef std::function<void(const RegisterResult&)> RegisterCallback;

  /**
   * 发送注册请求
   * @param name 用户名
   * @param password 密码
   * @param callback 注册结果回调函数
   */
  static void registerUser(const std::string& name, const std::string& password,
                           RegisterCallback callback);
};

#endif  // __REGISTER_H__
