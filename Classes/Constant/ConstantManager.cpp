#include "ConstantManager.h"
#include "base/CCDirector.h"

USING_NS_CC;

ConstantManager* ConstantManager::_instance = nullptr;

const ConstantManager* ConstantManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new (std::nothrow) ConstantManager();
        if (_instance && _instance->init()) {
            // 初始化成功
        } else {
            CC_SAFE_DELETE(_instance);
            _instance = nullptr;
        }
    }
    return _instance;
}

ConstantManager::ConstantManager() : _p00(0, 0) {
}

ConstantManager::~ConstantManager() {
}

bool ConstantManager::init() {
    calculateP00();
    return true;
}

void ConstantManager::calculateP00() {
    // 获取配置管理器
    auto configManager = ConfigManager::getInstance();
    if (!configManager) {
        CCLOG("ConfigManager not initialized");
        return;
    }
    
    auto constantConfig = configManager->getConstantConfig();
    
    // 从配置文件读取参数
    const float W = constantConfig.grassWidth;    // 图片宽度
    const float H = constantConfig.grassHeight;    // 图片高度
    
    // 获取可见区域大小和原点（此时分辨率已设置，可以正确获取）
    auto director = Director::getInstance();
    auto visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    
    float totalWidth = (W / 2.0f) * 84 + W;  // 最右端x坐标 + 图片长度
    float minY = -(H / 2.0f) * 43;           // 最上端相对y坐标
    float maxY = (H / 2.0f) * 43;            // 最下端相对y坐标
    float totalHeight = maxY - minY + H;
    
    // 计算 p00 位置（最左端，居中显示）
    _p00 = Vec2(origin.x + (visibleSize.width - totalWidth) / 2,
                origin.y + visibleSize.height / 2);
}

void ConstantManager::recalculateP00() {
    calculateP00();
}

