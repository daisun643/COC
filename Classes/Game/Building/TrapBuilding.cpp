#include "TrapBuilding.h"
#include "Manager/Config/ConfigManager.h"

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
    _triggerRange = config.attackRange; // 复用 attackRange 作为触发范围

    // 设置初始状态
    _isArmed = true;

    // 半透明显示表示它是陷阱（对自己可见）
    this->setOpacity(180);
    
    return true;
}

void TrapBuilding::upgrade() {
    // 调用父类升级逻辑（扣除资源、UI更新等）
    Building::upgrade();
    
    // 更新属性
    auto config = ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);
    _damage = config.damage;
    _triggerRange = config.attackRange;
    
    // 升级后自动重置
    rearm();
}

bool TrapBuilding::checkTrigger(const std::vector<BasicSoldier*>& soldiers) {
    if (!_isArmed) return false;

    // 遍历敌人，检查是否有进入触发范围的
    bool triggered = false;
    
    // 这里的坐标计算需要根据实际场景坐标系调整
    Vec2 myPos = this->getPosition(); 
    
    for (const auto& soldier : soldiers) {
        if (soldier && soldier->getPosition().distance(myPos) <= _triggerRange) {
            triggered = true;
            break;
        }
    }

    if (triggered) {
        explode();
        return true;
    }

    return false;
}

void TrapBuilding::explode() {
    _isArmed = false;
    
    // 1. 播放爆炸特效 (示例)
    auto particle = ParticleExplosion::create();
    particle->setPosition(this->getContentSize().width / 2, this->getContentSize().height / 2);
    particle->setAutoRemoveOnFinish(true);
    this->addChild(particle, 20);

    // 2. 变更外观为已损毁 (可选)
    this->setColor(Color3B::GRAY);

    CCLOG("Trap %s exploded! Dealt %d damage.", _buildingName.c_str(), _damage);
    
    // 注意：实际的伤害扣除逻辑通常由 BattleLayer 调用 checkTrigger 后，
    // 获取返回值，再对范围内的所有单位进行扣血。
    // 或者在这里通过回调函数通知战斗层造成区域伤害。
}

void TrapBuilding::rearm() {
    _isArmed = true;
    this->setColor(Color3B::WHITE); // 恢复颜色
}