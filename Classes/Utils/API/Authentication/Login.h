#ifndef __LOGIN_H__
#define __LOGIN_H__

#include "Utils/API/BaseURL.h"
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

/**
 * 登录响应结果结构
 */
struct LoginResult {
  bool success;
  std::string message;
  std::string name;

  LoginResult() : success(false), name("") {}
};

/**
 * 登录 API 类
 * 发送 GET 请求到 /api/auth/login
 */
class Login {
 public:
  /**
   * 登录回调函数类型
   * @param result 登录结果
   */
  typedef std::function<void(const LoginResult&)> LoginCallback;

  /**
   * 发送登录请求
   * @param id 用户ID
   * @param password 密码
   * @param callback 登录结果回调函数
   */
  static void login(const int& id, const std::string& password,
                    LoginCallback callback);
};

#endif  // __LOGIN_H__
