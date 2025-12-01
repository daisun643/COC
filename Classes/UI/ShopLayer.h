#ifndef __SHOP_LAYER_H__
#define __SHOP_LAYER_H__

#include <functional>
#include <vector>

#include "Game/Building/Building.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

struct ShopItem {
  std::string id;                                  // 唯一标识，用于创建建筑
  std::string displayName;                         // 显示名称
  std::string description;                         // 简短描述
  int costGold = 0;                                // 金币价格
  int costElixir = 0;                              // 圣水价格
  int defaultLevel = 1;                            // 初始等级
  BuildingType category = BuildingType::RESOURCE;  // 分类
  cocos2d::Color4B placeholderColor = cocos2d::Color4B::WHITE;
  int gridCount = 2;          // 占用网格大小（菱形边长）
  float anchorRatioX = 0.5f;  // 锚点X比例
  float anchorRatioY = 0.5f;  // 锚点Y比例
  float imageScale = 0.75f;   // 缩放占位
};

class ShopLayer : public cocos2d::LayerColor {
 public:
  static ShopLayer* createWithItems(const std::vector<ShopItem>& items);

  bool initWithItems(const std::vector<ShopItem>& items);

  void setPurchaseCallback(
      const std::function<bool(const ShopItem&)>& callback);

 private:
  void buildUI();
  void populateItems();
  cocos2d::ui::Layout* createItemCard(const ShopItem& item, int globalIndex);
  void showMessage(const std::string& text);

  bool onBackgroundTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

  std::vector<ShopItem> _items;
  cocos2d::ui::Layout* _panel;
  cocos2d::ui::ScrollView* _scrollView;
  cocos2d::Label* _messageLabel;
  cocos2d::Label* _emptyLabel;
  std::function<bool(const ShopItem&)> _purchaseCallback;
};

#endif  // __SHOP_LAYER_H__
