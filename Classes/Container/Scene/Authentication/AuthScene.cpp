#include "Container/Scene/Authentication/AuthScene.h"

#include "Container/Layer/Authentication/LoginLayer.h"
#include "Container/Layer/Authentication/RegisterLayer.h"
#include "Container/Scene/GameScene.h"

USING_NS_CC;

Scene* AuthScene::createScene() { return AuthScene::create(); }

bool AuthScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // 进入场景时，默认显示登录界面
  showLoginLayer();

  return true;
}

void AuthScene::showLoginLayer() {
  // 清空旧 Layer
  this->removeAllChildren();

  auto loginLayer = LoginLayer::create();

  // 登录成功
  loginLayer->setOnLoginSuccessCallback([this]() {
    CCLOG("AuthScene: 登录成功，进入主场景");
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.3f, gameScene));
  });

  // 登录失败
  loginLayer->setOnLoginFailureCallback([](const std::string& message) {
    CCLOG("AuthScene: 登录失败: %s", message.c_str());
  });

  // 切换到注册界面
  loginLayer->setOnSwitchToRegisterCallback(
      [this]() { this->showRegisterLayer(); });

  this->addChild(loginLayer);
}

void AuthScene::showRegisterLayer() {
  this->removeAllChildren();
  auto registerLayer = RegisterLayer::create();

  // 注册成功 → 回到登录界面
  registerLayer->setOnRegisterSuccessCallback([this](int user_id) {
    CCLOG("AuthScene: 注册成功，用户ID=%d，返回登录", user_id);
    this->showLoginLayer();
  });

  // 注册失败
  registerLayer->setOnRegisterFailureCallback([](const std::string& message) {
    CCLOG("AuthScene: 注册失败: %s", message.c_str());
  });

  // 返回登录界面
  registerLayer->setOnBackToLoginCallback([this]() {
    CCLOG("AuthScene: 返回登录界面");
    this->showLoginLayer();
  });

  this->addChild(registerLayer);
}
