#include "Building.h"

Building::Building()
: _buildingType(BuildingType::TOWN_HALL)
, _level(1)
, _maxLevel(10)
, _infoLabel(nullptr)
, _buildingName("Building")
{
}

Building::~Building()
{
}

Building* Building::create(const std::string& imagePath, BuildingType type, int level)
{
    Building* building = new (std::nothrow) Building();
    if (building && building->init(imagePath, type, level))
    {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool Building::init(const std::string& imagePath, BuildingType type, int level)
{
    _buildingType = type;
    _level = level;
    
    // 尝试加载图片，如果失败则创建默认外观
    if (!Sprite::initWithFile(imagePath))
    {
        createDefaultAppearance();
    }
    
    // 设置锚点为中心
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    
    // 创建信息标签（初始隐藏）
    _infoLabel = Label::createWithSystemFont("", "Arial", 12);
    _infoLabel->setPosition(Vec2(this->getContentSize().width / 2, 
                                  this->getContentSize().height + 20));
    _infoLabel->setColor(Color3B::WHITE);
    _infoLabel->setVisible(false);
    this->addChild(_infoLabel, 10);
    
    // 添加触摸事件（点击显示信息）
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        Vec2 locationInNode = this->convertToNodeSpace(touch->getLocation());
        Rect rect = Rect(0, 0, this->getContentSize().width, this->getContentSize().height);
        if (rect.containsPoint(locationInNode))
        {
            this->showInfo();
            return true;
        }
        return false;
    };
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        this->hideInfo();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

void Building::createDefaultAppearance()
{
    // 根据建筑类型创建不同颜色的默认外观
    Color4B color = Color4B::WHITE;
    switch (_buildingType)
    {
        case BuildingType::TOWN_HALL:
            color = Color4B(139, 69, 19, 255); // 棕色
            break;
        case BuildingType::DEFENSE:
            color = Color4B(255, 0, 0, 255); // 红色
            break;
        case BuildingType::RESOURCE:
            color = Color4B(255, 215, 0, 255); // 金色
            break;
        case BuildingType::STORAGE:
            color = Color4B(0, 0, 255, 255); // 蓝色
            break;
        case BuildingType::BARRACKS:
            color = Color4B(0, 255, 0, 255); // 绿色
            break;
    }
    
    // 创建彩色矩形作为默认外观
    auto layer = LayerColor::create(color, 80, 80);
    layer->setPosition(Vec2::ZERO);
    this->addChild(layer);
    
    // 设置内容大小
    this->setContentSize(Size(80, 80));
}

bool Building::upgrade()
{
    if (_level >= _maxLevel)
    {
        return false; // 已达到最高等级
    }
    
    _level++;
    // 可以在这里添加升级后的视觉效果
    return true;
}

int Building::getUpgradeCost() const
{
    // 基础升级成本 * 等级
    return 100 * _level;
}

std::string Building::getBuildingInfo() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s\nLevel: %d/%d\nUpgrade: %d", 
             _buildingName.c_str(), _level, _maxLevel, getUpgradeCost());
    return std::string(buffer);
}

void Building::showInfo()
{
    if (_infoLabel)
    {
        _infoLabel->setString(getBuildingInfo());
        _infoLabel->setVisible(true);
    }
}

void Building::hideInfo()
{
    if (_infoLabel)
    {
        _infoLabel->setVisible(false);
    }
}

