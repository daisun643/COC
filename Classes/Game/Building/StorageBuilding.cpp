#include "StorageBuilding.h"

StorageBuilding::StorageBuilding()
: _resourceType(ResourceType::GOLD)
, _capacity(1000)
, _storedAmount(0)
{
    _buildingType = BuildingType::STORAGE;
    _maxLevel = 10;
}

StorageBuilding::~StorageBuilding()
{
}

StorageBuilding* StorageBuilding::create(ResourceType resourceType, int level)
{
    StorageBuilding* building = new (std::nothrow) StorageBuilding();
    if (building && building->init(resourceType, level))
    {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool StorageBuilding::init(ResourceType resourceType, int level)
{
    _resourceType = resourceType;
    
    // 根据资源类型设置名称
    switch (resourceType)
    {
        case ResourceType::GOLD:
            _buildingName = "Gold Storage";
            break;
        case ResourceType::ELIXIR:
            _buildingName = "Elixir Storage";
            break;
        case ResourceType::DARK_ELIXIR:
            _buildingName = "Dark Elixir Storage";
            break;
    }
    
    if (!Building::init("", BuildingType::STORAGE, level))
    {
        return false;
    }
    
    initStorageProperties();
    
    return true;
}

void StorageBuilding::initStorageProperties()
{
    // 根据资源类型和等级初始化容量
    switch (_resourceType)
    {
        case ResourceType::GOLD:
            _capacity = 2000 * _level;
            break;
        case ResourceType::ELIXIR:
            _capacity = 2000 * _level;
            break;
        case ResourceType::DARK_ELIXIR:
            _capacity = 1000 * _level;
            break;
    }
}

int StorageBuilding::addResource(int amount)
{
    int available = getAvailableCapacity();
    int actualAdd = (amount > available) ? available : amount;
    _storedAmount += actualAdd;
    return actualAdd;
}

int StorageBuilding::removeResource(int amount)
{
    int actualRemove = (amount > _storedAmount) ? _storedAmount : amount;
    _storedAmount -= actualRemove;
    return actualRemove;
}

int StorageBuilding::getAvailableCapacity() const
{
    return _capacity - _storedAmount;
}

std::string StorageBuilding::getBuildingInfo() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "%s\nLevel: %d/%d\nResource: %s\nCapacity: %d\nStored: %d/%d\nAvailable: %d\nUpgrade Cost: %d",
             _buildingName.c_str(), _level, _maxLevel, 
             getResourceName().c_str(), _capacity, 
             _storedAmount, _capacity, getAvailableCapacity(), getUpgradeCost());
    return std::string(buffer);
}

std::string StorageBuilding::getResourceName() const
{
    switch (_resourceType)
    {
        case ResourceType::GOLD:
            return "Gold";
        case ResourceType::ELIXIR:
            return "Elixir";
        case ResourceType::DARK_ELIXIR:
            return "Dark Elixir";
    }
    return "Unknown";
}

