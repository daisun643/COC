#include "AppDelegate.h"
#include "Game/GameScene.h"
#include "Config/ConfigManager.h"
#include "Game/BuildingManager.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

USING_NS_CC;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // 初始化配置管理器
    auto configManager = ConfigManager::getInstance();
    if (!configManager) {
        CCLOG("Failed to initialize ConfigManager!");
        return false;
    }
    
    auto bgConfig = configManager->getBackgroundConfig();
    
    // 从配置文件读取分辨率
    cocos2d::Size designResolutionSize = cocos2d::Size((float)bgConfig.designWidth, (float)bgConfig.designHeight);
    cocos2d::Size smallResolutionSize = cocos2d::Size((float)bgConfig.smallWidth, (float)bgConfig.smallHeight);
    cocos2d::Size mediumResolutionSize = cocos2d::Size((float)bgConfig.mediumWidth, (float)bgConfig.mediumHeight);
    cocos2d::Size largeResolutionSize = cocos2d::Size((float)bgConfig.largeWidth, (float)bgConfig.largeHeight);
    
    // 初始化导演类
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("COC", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
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
    director->setDisplayStats(true);

    // 设置 FPS 为 60
    director->setAnimationInterval(1.0f / 60);

    // 设置设计分辨率
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    auto frameSize = glview->getFrameSize();
    
    // 根据屏幕大小选择合适的分辨率
    if (frameSize.height > mediumResolutionSize.height)
    {
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    else if (frameSize.height > smallResolutionSize.height)
    {
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    else
    {
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }

    // 初始化 BuildingManager（需要在设置分辨率之后，创建场景之前）
    // 计算地图原点 p00，用于 BuildingManager 初始化
    auto constantConfig = configManager->getConstantConfig();
    float L = constantConfig.grassLength;    // 图片长度
    float W = constantConfig.grassWidth;     // 图片宽度
    float H = constantConfig.grassHeight;    // 图片高度
    
    // 获取可见区域大小和原点（此时分辨率已设置，可以正确获取）
    auto visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    
    // 根据公式计算地图原点 p00
    // p[43][43].x = p[0][0].x + W/2 * 84
    // 总宽度 = (W / 2) * 84 + L
    float totalWidth = (W / 2.0f) * 84 + L;  // 最右端x坐标 + 图片长度
    
    // 计算 p00 位置（最左端，居中显示）
    Vec2 p00(origin.x + (visibleSize.width - totalWidth) / 2,
             origin.y + visibleSize.height / 2);
    
    // 初始化 BuildingManager
    BuildingManager::initialize(p00);
    
    // 创建游戏场景
    auto scene = GameScene::createScene();
    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();
}

