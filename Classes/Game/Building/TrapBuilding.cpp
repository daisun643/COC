#include "TrapBuilding.h"
#include "Manager/Config/ConfigManager.h"
#include <cmath>

TrapBuilding::TrapBuilding() 
    : _triggerRange(0.0f)
    , _damage(0)
    , _isArmed(true) 
{
}

TrapBuilding::~TrapBuilding() {}

TrapBuilding* TrapBuilding::create(int level, const std::string& buildingName) {
    TrapBuilding* pRet = new(std::nothrow) TrapBuilding();
    if (pRet && pRet->init(level, buildingName)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

bool TrapBuilding::init(int level, const std::string& buildingName) {
    auto configManager = ConfigManager::getInstance();
    auto config = configManager->getBuildingConfig(buildingName, level);

    // 初始化父类
    // Trap 类型可以直接使用 BuildingType::TRAP
    if (!Building::init(config.image, BuildingType::TRAP, level, 
                       config.gridCount, config.anchorRatioX, config.anchorRatioY, config.imageScale)) {
        return false;
    }

    _buildingName = buildingName;
    _damage = config.damage;           // 读取配置中的伤害
    // 将配置中的 Grid 单位范围转换为 Pixel 单位范围
    // 获取网格的像素参数 (deltaX, deltaY)
    auto constant = configManager->getConstantConfig();
    
    // 计算 1 个网格边长对应的屏幕像素距离 (勾股定理)
    // 假设 deltaX 和 deltaY 是菱形网格半宽/半高，或者投影向量
    // 这里估算一个网格的标准像素长度
    float oneGridPixels = 50.0f; // 默认安全值
    if (constant.deltaX > 0) {
        oneGridPixels = std::sqrt(std::pow(constant.deltaX, 2) + std::pow(constant.deltaY, 2));
    }

    // attackRange 是网格数 (如 3.0)，转换为像素距离
    _triggerRange = config.attackRange * oneGridPixels * 0.6f;
    
    CCLOG("Trap Init: %s, GridRange: %.1f, PixelRange: %.1f", buildingName.c_str(), config.attackRange, _triggerRange);

    // 设置初始状态
    _isArmed = true;

    // 彻底移除血条显示
    if (_hpBarBackground) _hpBarBackground->setVisible(false);
    if (_hpBarForeground) _hpBarForeground->setVisible(false);

    // 半透明显示表示它是陷阱（对自己可见）
    this->setOpacity(180);
    
    return true;
}

// 现空的 updateHPBar
void TrapBuilding::updateHPBar() {
    // 炸弹没有血条，此函数留空，禁止父类绘制血条
    // 不要调用 Building::updateHPBar();
}

void TrapBuilding::upgrade() {
    // 调用父类升级逻辑（扣除资源、UI更新等）
    Building::upgrade();
    
    // 更新属性
    auto configManager = ConfigManager::getInstance();
    auto config = configManager->getBuildingConfig(_buildingName, _level);
    _damage = config.damage;

    auto constant = configManager->getConstantConfig();
    float oneGridPixels = 50.0f;
    if (constant.deltaX > 0) {
        oneGridPixels = std::sqrt(std::pow(constant.deltaX, 2) + std::pow(constant.deltaY, 2));
    }
    _triggerRange = config.attackRange * oneGridPixels * 0.6f;
    
    // 升级后自动重置
    rearm();
}

void TrapBuilding::hide() {
    this->setVisible(false);
}

void TrapBuilding::reveal() {
    this->setVisible(true);
    this->setOpacity(255);
}

bool TrapBuilding::checkTrigger(const std::vector<BasicSoldier*>& soldiers) {
    if (!_isArmed) return false;

    // 遍历敌人，检查是否有进入触发范围的
    bool triggered = false;
    
    // 这里的坐标计算需要根据实际场景坐标系调整
    Vec2 myPos = this->getPosition(); 
    
    for (const auto& soldier : soldiers) {
        if (soldier && soldier->isAlive() && soldier->getPosition().distance(myPos) <= _triggerRange) {
            triggered = true;
            break;
        }
    }

    if (triggered) {
        explode(soldiers);
        return true;
    }

    return false;
}

void TrapBuilding::explode(const std::vector<BasicSoldier*>& soldiers) {
    _isArmed = false; // 标记为已触发，防止重复爆炸

    // 1. 显形
    reveal();

    // 2. 播放爆炸特效
    auto particle = ParticleExplosion::create();
    if (particle) {
        // 将粒子位置设置为炸弹当前位置
        particle->setPosition(this->getPosition());
        particle->setAutoRemoveOnFinish(true);
        particle->setScale(0.8f);
        particle->setStartColor(Color4F::ORANGE);
        particle->setEndColor(Color4F::BLACK);
        
        // 将粒子添加到 MapLayer (父节点)，而不是炸弹自己
        // 这样即使炸弹隐藏了，粒子依然能正常播放完毕
        if (this->getParent()) {
            this->getParent()->addChild(particle, 100);
        } else {
            this->addChild(particle, 100);
        }
    }
    
    // 3. 造成区域伤害
    // 爆炸范围通常等于或略大于触发范围
    float damageRadius = _triggerRange * 1.5f; 
    Vec2 myPos = this->getPosition();

    // 现在可以直接使用 soldiers 变量了
    for (const auto& soldier : soldiers) {
        // 增加存活检查
        if (soldier && soldier->isAlive()) {
            float distance = soldier->getPosition().distance(myPos);
            if (distance <= damageRadius) {
                soldier->takeDamage(_damage);
            }
        }
    }

    // 4. 延迟1秒后消失
    auto delay = DelayTime::create(1.0f);
    auto hide = Hide::create();
    auto seq = Sequence::create(delay, hide, nullptr);
    this->runAction(seq);
    
    CCLOG("Trap %s exploded! Dealt %d damage.", _buildingName.c_str(), _damage);
}

void TrapBuilding::rearm() {
    _isArmed = true;
    this->setColor(Color3B::WHITE);
    // 重新布防时显示
    this->setVisible(true);
    this->setOpacity(180); 
    
    // 确保血条依然是隐藏的
    if (_hpBarBackground) _hpBarBackground->setVisible(false);
    if (_hpBarForeground) _hpBarForeground->setVisible(false);
}