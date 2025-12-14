#include "AppDelegate.h"

#include "Container/Scene/AttackScence/AttackScene.h"
#include "Container/Scene/GameScene.h"
#include "Container/Scene/Record/RecordScene.h"
#include "Manager/Config/ConfigManager.h"
#include "Manager/PlayerManager.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

USING_NS_CC;

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

bool AppDelegate::applicationDidFinishLaunching() {
  // 初始化配置管理器
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("Failed to initialize ConfigManager!");
    return false;
  }

  auto bgConfig = configManager->getBackgroundConfig();

  // 从配置文件读取分辨率
  cocos2d::Size designResolutionSize =
      cocos2d::Size((float)bgConfig.designWidth, (float)bgConfig.designHeight);
  cocos2d::Size smallResolutionSize =
      cocos2d::Size((float)bgConfig.smallWidth, (float)bgConfig.smallHeight);
  cocos2d::Size mediumResolutionSize =
      cocos2d::Size((float)bgConfig.mediumWidth, (float)bgConfig.mediumHeight);
  cocos2d::Size largeResolutionSize =
      cocos2d::Size((float)bgConfig.largeWidth, (float)bgConfig.largeHeight);

  // 初始化导演类
  auto director = Director::getInstance();
  auto glview = director->getOpenGLView();
  if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || \
    (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) ||   \
    (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    glview = GLViewImpl::createWithRect(
        "COC", cocos2d::Rect(0, 0, designResolutionSize.width,
                             designResolutionSize.height));
    if (!glview) {
      MessageBoxA(nullptr, "Failed to create OpenGL view!", "Error", MB_OK);
      return false;
    }
#else
    glview = GLViewImpl::create("COC");
    if (!glview) {
      return false;
    }
#endif
    director->setOpenGLView(glview);
  }

  // 设置 FPS 显示
  director->setDisplayStats(false);

  // 设置 FPS 为 60
  director->setAnimationInterval(1.0f / 60);

  // 设置设计分辨率
  glview->setDesignResolutionSize(designResolutionSize.width,
                                  designResolutionSize.height,
                                  ResolutionPolicy::NO_BORDER);
  auto frameSize = glview->getFrameSize();

  // 根据屏幕大小选择合适的分辨率
  if (frameSize.height > mediumResolutionSize.height) {
    director->setContentScaleFactor(
        MIN(largeResolutionSize.height / designResolutionSize.height,
            largeResolutionSize.width / designResolutionSize.width));
  } else if (frameSize.height > smallResolutionSize.height) {
    director->setContentScaleFactor(
        MIN(mediumResolutionSize.height / designResolutionSize.height,
            mediumResolutionSize.width / designResolutionSize.width));
  } else {
    director->setContentScaleFactor(
        MIN(smallResolutionSize.height / designResolutionSize.height,
            smallResolutionSize.width / designResolutionSize.width));
  }

  // 创建游戏场景
  auto scene = GameScene::createScene();
  // auto scene = AttackScene::createScene();
  // auto scene = RecordScene::createScene();
  director->runWithScene(scene);

  return true;
}

void AppDelegate::applicationDidEnterBackground() {
  Director::getInstance()->stopAnimation();
  // 这是移动设备上保存进度的标准时机
  auto playerManager = PlayerManager::getInstance();
  if (playerManager) {
      playerManager->saveUserData();
      CCLOG("AppDelegate: Application entered background, data saved.");
  }
}

void AppDelegate::applicationWillEnterForeground() {
  Director::getInstance()->startAnimation();
}
