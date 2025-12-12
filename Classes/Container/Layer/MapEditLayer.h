#ifndef __MAP_EDIT_LAYER_H__
#define __MAP_EDIT_LAYER_H__

#include <map>
#include <string>
#include <vector>

#include "Container/Layer/ShopLayer.h"  // For ShopItem struct
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class MapEditLayer : public cocos2d::Layer {
 public:
  static MapEditLayer* createWithItems(const std::vector<ShopItem>& allItems);
  virtual bool initWithItems(const std::vector<ShopItem>& allItems);

  // Callbacks
  void setOnRemoveAllCallback(std::function<void()> callback);
  void setOnSaveCallback(std::function<void()> callback);
  void setOnCancelCallback(std::function<void()> callback);
  void setOnItemClickCallback(
      std::function<void(const std::string& id)> callback);

  // Update inventory display
  void updateInventory(const std::map<std::string, int>& inventory);

 private:
  void buildUI();
  void populateInventory();
  cocos2d::ui::Layout* createInventoryCard(const ShopItem& item, int count);

  std::vector<ShopItem> _allItems;
  std::map<std::string, int> _currentInventory;

  cocos2d::ui::ScrollView* _scrollView;
  cocos2d::ui::Button* _removeAllButton;
  cocos2d::ui::Button* _saveButton;
  cocos2d::ui::Button* _cancelButton;

  std::function<void()> _onRemoveAll;
  std::function<void()> _onSave;
  std::function<void()> _onCancel;
  std::function<void(const std::string&)> _onItemClick;
};

#endif  // __MAP_EDIT_LAYER_H__
