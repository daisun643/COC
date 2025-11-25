#include "GameScene.h"

Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建背景
    auto background = LayerColor::create(Color4B(135, 206, 235, 255)); // 天蓝色背景
    this->addChild(background, -1);
    
    // 添加标题
    auto titleLabel = Label::createWithSystemFont("COC - Village", "Arial", 36);
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, 
                                 origin.y + visibleSize.height - 30));
    titleLabel->setColor(Color3B::WHITE);
    this->addChild(titleLabel, 1);
    
    // 添加说明文字
    auto infoLabel = Label::createWithSystemFont("Click buildings to see info", "Arial", 16);
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, 
                                origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B::YELLOW);
    this->addChild(infoLabel, 1);
    
    // 初始化各种建筑
    initTownHall();
    initDefenseBuildings();
    initResourceBuildings();
    initStorageBuildings();
    initBarracks();
    
    return true;
}

void GameScene::initTownHall()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建大本营（放在中心位置）
    _townHall = TownHall::create(1);
    _townHall->setPosition(Vec2(origin.x + visibleSize.width / 2, 
                                origin.y + visibleSize.height / 2));
    this->addChild(_townHall, 1);
}

void GameScene::initDefenseBuildings()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建不同类型的防御建筑
    auto cannon1 = DefenseBuilding::create(DefenseType::CANNON, 1);
    addBuilding(cannon1, Vec2(origin.x + 150, origin.y + 200));
    
    auto cannon2 = DefenseBuilding::create(DefenseType::CANNON, 2);
    addBuilding(cannon2, Vec2(origin.x + visibleSize.width - 150, origin.y + 200));
    
    auto archerTower = DefenseBuilding::create(DefenseType::ARCHER_TOWER, 1);
    addBuilding(archerTower, Vec2(origin.x + visibleSize.width / 2, origin.y + 100));
    
    auto mortar = DefenseBuilding::create(DefenseType::MORTAR, 1);
    addBuilding(mortar, Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 150));
}

void GameScene::initResourceBuildings()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建资源建筑
    auto goldMine1 = ResourceBuilding::create(ResourceType::GOLD, 1);
    addBuilding(goldMine1, Vec2(origin.x + 100, origin.y + 400));
    
    auto goldMine2 = ResourceBuilding::create(ResourceType::GOLD, 2);
    addBuilding(goldMine2, Vec2(origin.x + 250, origin.y + 400));
    
    auto elixirCollector1 = ResourceBuilding::create(ResourceType::ELIXIR, 1);
    addBuilding(elixirCollector1, Vec2(origin.x + visibleSize.width - 100, origin.y + 400));
    
    auto elixirCollector2 = ResourceBuilding::create(ResourceType::ELIXIR, 1);
    addBuilding(elixirCollector2, Vec2(origin.x + visibleSize.width - 250, origin.y + 400));
}

void GameScene::initStorageBuildings()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建储存建筑
    auto goldStorage = StorageBuilding::create(ResourceType::GOLD, 1);
    addBuilding(goldStorage, Vec2(origin.x + 150, origin.y + 300));
    
    auto elixirStorage = StorageBuilding::create(ResourceType::ELIXIR, 1);
    addBuilding(elixirStorage, Vec2(origin.x + visibleSize.width - 150, origin.y + 300));
}

void GameScene::initBarracks()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 创建兵营
    auto barracks = Barracks::create(1);
    addBuilding(barracks, Vec2(origin.x + visibleSize.width / 2, origin.y + 250));
}

void GameScene::addBuilding(Building* building, const Vec2& position)
{
    if (building)
    {
        building->setPosition(position);
        this->addChild(building, 1);
    }
}

