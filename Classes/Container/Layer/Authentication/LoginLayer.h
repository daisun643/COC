#ifndef __LOGIN_LAYER_H__
#define __LOGIN_LAYER_H__

#include "Utils/API/Authentication/Login.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

/**
 * 登录界面 Layer
 * 包含用户ID输入框、密码输入框、登录按钮和注册按钮
 */
class LoginLayer : public Layer {
 public:
  CREATE_FUNC(LoginLayer);
  virtual bool init();

  // 设置登录成功回调
  void setOnLoginSuccessCallback(std::function<void()> callback);

  // 设置登录失败回调
  void setOnLoginFailureCallback(
      std::function<void(const std::string& message)> callback);

  // 设置切换到注册界面的回调
  void setOnSwitchToRegisterCallback(std::function<void()> callback);

 private:
  TextField* _idTextField;
  TextField* _passwordTextField;
  Button* _loginButton;
  Button* _registerButton;

  std::function<void()> _onLoginSuccess;
  std::function<void(const std::string&)> _onLoginFailure;
  std::function<void()> _onSwitchToRegister;

  // 登录按钮点击事件
  void onLoginButtonClicked(Ref* sender);

  // 注册按钮点击事件（切换到注册界面）
  void onRegisterButtonClicked(Ref* sender);
};

#endif  // __LOGIN_LAYER_H__
