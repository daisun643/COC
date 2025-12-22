#include "Container/Layer/Authentication/LoginLayer.h"

#include <cstdlib>
#include <sstream>

#include "Utils/Profile/Profile.h"
#include "Utils/API/User/User.h"

USING_NS_CC;
using namespace cocos2d::ui;

bool LoginLayer::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  Vec2 center =
      Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

  // 创建背景（可选）
  auto background = LayerColor::create(Color4B(50, 50, 50, 200),
                                       visibleSize.width, visibleSize.height);
  this->addChild(background);

  // 字体配置
  std::string fontFile = "fonts/NotoSansSC-VariableFont_wght.ttf";
  bool useTTF = FileUtils::getInstance()->isFileExist(fontFile);

  auto createLabel = [&](const std::string& text, float fontSize) -> Label* {
    Label* label;
    if (useTTF) {
      TTFConfig ttfConfig(fontFile, fontSize);
      label = Label::createWithTTF(ttfConfig, text);
    } else {
      label = Label::createWithSystemFont(text, "Arial", fontSize);
    }
    label->setTextColor(Color4B::WHITE);
    return label;
  };

  // 标题
  auto titleLabel = createLabel("用户登录", 36);
  titleLabel->setPosition(Vec2(center.x, center.y + 150));
  this->addChild(titleLabel);

  // 用户ID标签
  auto idLabel = createLabel("用户ID:", 24);
  idLabel->setPosition(Vec2(center.x - 100, center.y + 50));
  this->addChild(idLabel);

  // 用户ID输入框
  _idTextField = TextField::create("请输入用户ID", "Arial", 24);
  _idTextField->setPlaceHolderColor(Color4B::GRAY);
  _idTextField->setTextColor(Color4B::BLACK);
  _idTextField->setContentSize(Size(300, 40));
  _idTextField->setPosition(Vec2(center.x + 50, center.y + 50));
  _idTextField->setMaxLength(10);
  this->addChild(_idTextField);

  // 密码标签
  auto passwordLabel = createLabel("密码:", 24);
  passwordLabel->setPosition(Vec2(center.x - 100, center.y - 20));
  this->addChild(passwordLabel);

  // 密码输入框
  _passwordTextField = TextField::create("请输入密码", "Arial", 24);
  _passwordTextField->setPlaceHolderColor(Color4B::GRAY);
  _passwordTextField->setTextColor(Color4B::BLACK);
  _passwordTextField->setContentSize(Size(300, 40));
  _passwordTextField->setPosition(Vec2(center.x + 50, center.y - 20));
  _passwordTextField->setPasswordEnabled(true);  // 启用密码模式
  _passwordTextField->setMaxLength(20);
  this->addChild(_passwordTextField);

  // 登录按钮
  _loginButton = Button::create();
  _loginButton->setTitleText("登录");
  _loginButton->setTitleFontSize(28);
  _loginButton->setTitleColor(Color3B::WHITE);
  _loginButton->setContentSize(Size(200, 50));
  _loginButton->setPosition(Vec2(center.x - 110, center.y - 100));
  _loginButton->addClickEventListener(
      CC_CALLBACK_1(LoginLayer::onLoginButtonClicked, this));
  _loginButton->setColor(Color3B(0, 200, 0));  // 绿色
  this->addChild(_loginButton);

  // 注册按钮
  _registerButton = Button::create();
  _registerButton->setTitleText("注册");
  _registerButton->setTitleFontSize(28);
  _registerButton->setTitleColor(Color3B::WHITE);
  _registerButton->setContentSize(Size(200, 50));
  _registerButton->setPosition(Vec2(center.x + 110, center.y - 100));
  _registerButton->addClickEventListener(
      CC_CALLBACK_1(LoginLayer::onRegisterButtonClicked, this));
  _registerButton->setColor(Color3B(0, 150, 255));  // 蓝色
  this->addChild(_registerButton);

  return true;
}

void LoginLayer::setOnLoginSuccessCallback(std::function<void()> callback) {
  _onLoginSuccess = callback;
}

void LoginLayer::setOnLoginFailureCallback(
    std::function<void(const std::string& message)> callback) {
  _onLoginFailure = callback;
}

void LoginLayer::setOnSwitchToRegisterCallback(std::function<void()> callback) {
  _onSwitchToRegister = callback;
}

void LoginLayer::onLoginButtonClicked(Ref* sender) {
  // 获取输入的用户ID和密码
  std::string idStr = _idTextField->getString();
  std::string password = _passwordTextField->getString();

  // 验证输入
  if (idStr.empty() || password.empty()) {
    CCLOG("LoginLayer: 用户ID和密码不能为空");
    if (_onLoginFailure) {
      _onLoginFailure("用户ID和密码不能为空");
    }
    return;
  }

  // 将字符串转换为整数
  int id = 0;
  try {
    id = std::stoi(idStr);
  } catch (const std::exception& e) {
    CCLOG("LoginLayer: 用户ID格式错误: %s", e.what());
    if (_onLoginFailure) {
      _onLoginFailure("用户ID必须是数字");
    }
    return;
  }

  // 禁用按钮，防止重复点击
  _loginButton->setEnabled(false);
  _loginButton->setTitleText("登录中...");

  // 调用登录接口
  Login::login(id, password, [this, id](const LoginResult& result) {
    // 恢复按钮状态
    _loginButton->setEnabled(true);
    _loginButton->setTitleText("登录");

    if (result.success) {
      CCLOG("LoginLayer: 登录成功，用户ID: %d, 用户名: %s", id,
            result.name.c_str());

      // 更新 Profile
      auto profile = Profile::getInstance();
      if (profile) {
        profile->setIsLogin(true);
        profile->setId(id);
        profile->setName(result.name);
      }

      if (_onLoginSuccess) {
        _onLoginSuccess();
        // 调用 Classes\Utils\API\User\User.cpp 中的 getClanId 并设置 profile->setClansId
        auto profile = Profile::getInstance();
        if (profile) {
          UserAPI::getClanId(std::to_string(id), [profile](bool success, const std::string& message, const std::string& clan_id) {
            if (success && !clan_id.empty()) {
              try {
                int cid = std::stoi(clan_id);
                profile->setClansId(cid);
                CCLOG("LoginLayer: updated profile clansId=%d", cid);
              } catch (const std::exception& e) {
                CCLOG("LoginLayer: failed to parse clan_id: %s", e.what());
              }
            } else {
              CCLOG("LoginLayer: getClanId failed: %s", message.c_str());
            }
          });
        }
      }
    } else {
      CCLOG("LoginLayer: 登录失败: %s", result.message.c_str());
      if (_onLoginFailure) {
        _onLoginFailure(result.message);
      }
    }
  });
}

void LoginLayer::onRegisterButtonClicked(Ref* sender) {
  // 切换到注册界面
  if (_onSwitchToRegister) {
    _onSwitchToRegister();
  }
}
