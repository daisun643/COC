#include "Barracks.h"

Barracks::Barracks() : _trainingCapacity(20), _currentTroops(0) {
  _buildingType = BuildingType::BARRACKS;
  _buildingName = "Barracks";
  _maxLevel = 10;
}

Barracks::~Barracks() {}

Barracks *Barracks::create(int level) {
  Barracks *barracks = new (std::nothrow) Barracks();
  if (barracks && barracks->init(level)) {
    barracks->autorelease();
    return barracks;
  }
  CC_SAFE_DELETE(barracks);
  return nullptr;
}

bool Barracks::init(int level) {
  if (!Building::init("", BuildingType::BARRACKS, level)) {
    return false;
  }

  initBarracksProperties();

  return true;
}

void Barracks::initBarracksProperties() {
  // 根据等级设置训练容量
  _trainingCapacity = 10 + (_level - 1) * 5;
}

bool Barracks::trainTroop(TroopType troopType, int count) {
  if (_currentTroops + count > _trainingCapacity) {
    return false; // 超过容量
  }

  _currentTroops += count;
  CCLOG("Training %d %s", count, getTroopName(troopType).c_str());
  return true;
}

int Barracks::getTrainingTime(TroopType troopType) const {
  // 根据兵种类型返回训练时间（秒）
  switch (troopType) {
  case TroopType::BARBARIAN:
    return 20;
  case TroopType::ARCHER:
    return 25;
  case TroopType::GIANT:
    return 60;
  case TroopType::GOBLIN:
    return 15;
  }
  return 0;
}

int Barracks::getTrainingCost(TroopType troopType) const {
  // 根据兵种类型返回训练成本
  switch (troopType) {
  case TroopType::BARBARIAN:
    return 25;
  case TroopType::ARCHER:
    return 50;
  case TroopType::GIANT:
    return 250;
  case TroopType::GOBLIN:
    return 15;
  }
  return 0;
}

std::string Barracks::getBuildingInfo() const {
  char buffer[256];
  snprintf(
      buffer, sizeof(buffer),
      "%s\nLevel: %d/%d\nCapacity: %d\nCurrent Troops: %d\nUpgrade Cost: %d",
      _buildingName.c_str(), _level, _maxLevel, _trainingCapacity,
      _currentTroops, getUpgradeCost());
  return std::string(buffer);
}

std::string Barracks::getTroopName(TroopType type) const {
  switch (type) {
  case TroopType::BARBARIAN:
    return "Barbarian";
  case TroopType::ARCHER:
    return "Archer";
  case TroopType::GIANT:
    return "Giant";
  case TroopType::GOBLIN:
    return "Goblin";
  }
  return "Unknown";
}
