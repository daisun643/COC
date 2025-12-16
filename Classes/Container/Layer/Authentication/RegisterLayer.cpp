#include "Container/Layer/Authentication/RegisterLayer.h"

USING_NS_CC;
using namespace cocos2d::ui;

bool RegisterLayer::init() {
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
  auto titleLabel = createLabel("用户注册", 36);
  titleLabel->setPosition(Vec2(center.x, center.y + 150));
  this->addChild(titleLabel);

  // 用户名标签
  auto nameLabel = createLabel("用户名:", 24);
  nameLabel->setPosition(Vec2(center.x - 100, center.y + 50));
  this->addChild(nameLabel);

  // 用户名输入框
  _nameTextField = TextField::create("请输入用户名", "Arial", 24);
  _nameTextField->setPlaceHolderColor(Color4B::GRAY);
  _nameTextField->setTextColor(Color4B::BLACK);
  _nameTextField->setContentSize(Size(300, 40));
  _nameTextField->setPosition(Vec2(center.x + 50, center.y + 50));
  _nameTextField->setMaxLength(20);
  this->addChild(_nameTextField);

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

  // 注册按钮
  _registerButton = Button::create();
  _registerButton->setTitleText("注册");
  _registerButton->setTitleFontSize(28);
  _registerButton->setTitleColor(Color3B::WHITE);
  _registerButton->setContentSize(Size(200, 50));
  _registerButton->setPosition(Vec2(center.x - 110, center.y - 100));
  _registerButton->addClickEventListener(
      CC_CALLBACK_1(RegisterLayer::onRegisterButtonClicked, this));

  // 设置按钮背景颜色
  _registerButton->setColor(Color3B(0, 150, 255));
  this->addChild(_registerButton);

  // 返回登录按钮
  _backButton = Button::create();
  _backButton->setTitleText("返回登录");
  _backButton->setTitleFontSize(28);
  _backButton->setTitleColor(Color3B::WHITE);
  _backButton->setContentSize(Size(200, 50));
  _backButton->setPosition(Vec2(center.x + 110, center.y - 100));
  _backButton->addClickEventListener(
      CC_CALLBACK_1(RegisterLayer::onBackButtonClicked, this));

  // 设置按钮背景颜色（灰色）
  _backButton->setColor(Color3B(128, 128, 128));
  this->addChild(_backButton);

  return true;
}

void RegisterLayer::setOnRegisterSuccessCallback(
    std::function<void(int user_id)> callback) {
  _onRegisterSuccess = callback;
}

void RegisterLayer::setOnRegisterFailureCallback(
    std::function<void(const std::string& message)> callback) {
  _onRegisterFailure = callback;
}

void RegisterLayer::setOnBackToLoginCallback(std::function<void()> callback) {
  _onBackToLogin = callback;
}

void RegisterLayer::onRegisterButtonClicked(Ref* sender) {
  // 获取输入的用户名和密码
  std::string name = _nameTextField->getString();
  std::string password = _passwordTextField->getString();

  // 验证输入
  if (name.empty() || password.empty()) {
    CCLOG("RegisterLayer: 用户名和密码不能为空");
    if (_onRegisterFailure) {
      _onRegisterFailure("用户名和密码不能为空");
    }
    return;
  }

  // 禁用按钮，防止重复点击
  _registerButton->setEnabled(false);
  _registerButton->setTitleText("注册中...");

  // 调用注册接口
  Register::registerUser(name, password, [this](const RegisterResult& result) {
    // 恢复按钮状态
    _registerButton->setEnabled(true);
    _registerButton->setTitleText("注册");

    if (result.success) {
      CCLOG("RegisterLayer: 注册成功，用户ID: %d", result.user_id);
      if (_onRegisterSuccess) {
        _onRegisterSuccess(result.user_id);
      }
    } else {
      CCLOG("RegisterLayer: 注册失败: %s", result.message.c_str());
      if (_onRegisterFailure) {
        _onRegisterFailure(result.message);
      }
    }
  });
}

void RegisterLayer::onBackButtonClicked(Ref* sender) {
  // 返回登录界面
  if (_onBackToLogin) {
    _onBackToLogin();
  }
}
