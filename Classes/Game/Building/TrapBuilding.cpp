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
    _isArmed = false; // [新增] 标记为已触发，防止重复爆炸

    // 1. 显形
    reveal();

    // 2. 播放爆炸特效
    auto particle = ParticleExplosion::create();
    if (particle) {
        particle->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height / 2));
        particle->setAutoRemoveOnFinish(true);
        particle->setScale(0.8f);
        // 设置粒子颜色为黑/红/橙，模拟炸弹
        particle->setStartColor(Color4F::ORANGE);
        particle->setEndColor(Color4F::BLACK);
        this->addChild(particle, 100);
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

    // 4. 自身状态变为废墟 (变灰)
    this->setColor(Color3B::GRAY);
    this->setOpacity(128);
    
    CCLOG("Trap %s exploded! Dealt %d damage.", _buildingName.c_str(), _damage);
}

void TrapBuilding::rearm() {
    _isArmed = true;
    this->setColor(Color3B::WHITE);
    this->setOpacity(180); // 恢复半透明状态
}