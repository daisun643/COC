#include "Container/Scene/Authentication/AuthScene.h"

#include <string>

#include "Container/Layer/Authentication/LoginLayer.h"
#include "Container/Layer/Authentication/RegisterLayer.h"
#include "Container/Scene/GameScene.h"
#include "cocos2d.h"

// Windows.h 只在 Windows 平台使用
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <Windows.h>
#endif

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
    auto msg = StringUtils::format("注册成功，用户ID=%d，返回登录", user_id);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    auto utf8ToWstring = [](const std::string& s) -> std::wstring {
      if (s.empty()) return std::wstring();
      int size =
          ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
      std::wstring w;
      w.resize(size);
      ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], size);
      return w;
    };

    std::wstring wmsg = utf8ToWstring(msg);
    std::wstring wcap = utf8ToWstring("注册成功");
    MessageBoxW(NULL, wmsg.c_str(), wcap.c_str(), MB_OK);
#else
    // 在非 Windows 平台使用 Cocos2d-x 的 MessageBox（接受 UTF-8）
    ccMessageBox(msg.c_str(), "注册成功");
#endif

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
