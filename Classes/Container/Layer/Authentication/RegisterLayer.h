#ifndef __REGISTER_LAYER_H__
#define __REGISTER_LAYER_H__

#include "Utils/API/Authentication/Register.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

/**
 * 注册界面 Layer
 * 包含用户名输入框、密码输入框和注册按钮
 */
class RegisterLayer : public Layer {
 public:
  CREATE_FUNC(RegisterLayer);
  virtual bool init();

  // 设置注册成功回调
  void setOnRegisterSuccessCallback(std::function<void(int user_id)> callback);

  // 设置注册失败回调
  void setOnRegisterFailureCallback(
      std::function<void(const std::string& message)> callback);

  // 设置返回登录界面的回调
  void setOnBackToLoginCallback(std::function<void()> callback);

 private:
  TextField* _nameTextField;
  TextField* _passwordTextField;
  Button* _registerButton;
  Button* _backButton;

  std::function<void(int)> _onRegisterSuccess;
  std::function<void(const std::string&)> _onRegisterFailure;
  std::function<void()> _onBackToLogin;

  // 注册按钮点击事件
  void onRegisterButtonClicked(Ref* sender);

  // 返回登录按钮点击事件
  void onBackButtonClicked(Ref* sender);
};

#endif  // __REGISTER_LAYER_H__
