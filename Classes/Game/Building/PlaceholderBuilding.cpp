#include "PlaceholderBuilding.h"

USING_NS_CC;

PlaceholderBuilding::PlaceholderBuilding() : _customColor(Color4B::WHITE) {}

PlaceholderBuilding::~PlaceholderBuilding() {}

PlaceholderBuilding* PlaceholderBuilding::create(
    const std::string& name, BuildingType type, const Color4B& color, int level,
    int gridCount, float anchorRatioX, float anchorRatioY, float imageScale) {
  PlaceholderBuilding* building = new (std::nothrow) PlaceholderBuilding();
  if (building &&
      building->initPlaceholder(name, type, color, level, gridCount,
                                anchorRatioX, anchorRatioY, imageScale)) {
    building->autorelease();
    return building;
  }
  CC_SAFE_DELETE(building);
  return nullptr;
}

bool PlaceholderBuilding::initPlaceholder(
    const std::string& name, BuildingType type, const Color4B& color, int level,
    int gridCount, float anchorRatioX, float anchorRatioY, float imageScale) {
  _customColor = color;
  _buildingName = name;

  if (!Building::init("", type, level, gridCount, anchorRatioX, anchorRatioY,
                      imageScale)) {
    return false;
  }

  return true;
}

void PlaceholderBuilding::createDefaultAppearance() {
  // 占位渲染：绘制一个简单的彩色块
  auto size = Size(100.0f, 100.0f);
  auto layer = LayerColor::create(_customColor, size.width, size.height);
  layer->setPosition(Vec2::ZERO);
  this->addChild(layer);
  this->setContentSize(size);
}
