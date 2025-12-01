#ifndef __PLACEHOLDER_BUILDING_H__
#define __PLACEHOLDER_BUILDING_H__

#include "Game/Building/Building.h"
#include "cocos2d.h"

class PlaceholderBuilding : public Building {
 public:
  static PlaceholderBuilding* create(const std::string& name, BuildingType type,
                                     const cocos2d::Color4B& color, int level,
                                     int gridCount, float anchorRatioX,
                                     float anchorRatioY,
                                     float imageScale = 0.75f);

 protected:
  bool initPlaceholder(const std::string& name, BuildingType type,
                       const cocos2d::Color4B& color, int level, int gridCount,
                       float anchorRatioX, float anchorRatioY,
                       float imageScale);

  void createDefaultAppearance() override;

 private:
  PlaceholderBuilding();
  ~PlaceholderBuilding() override;

  cocos2d::Color4B _customColor;
};

#endif  // __PLACEHOLDER_BUILDING_H__
