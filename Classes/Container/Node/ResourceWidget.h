#ifndef __RESOURCE_WIDGET_H__
#define __RESOURCE_WIDGET_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class ResourceWidget : public cocos2d::Node {
 public:
  enum class Type { GOLD, ELIXIR };

  static ResourceWidget* create(Type type);
  virtual bool init(Type type);

  void updateAmount(int amount, int maxAmount);
  void setProduction(int production);

 private:
  void showInfoPopup();
  void hideInfoPopup();

  cocos2d::Label* _amountLabel;
  cocos2d::Sprite* _icon;
  cocos2d::Sprite* _background;
  cocos2d::ui::LoadingBar* _progressBar;
  cocos2d::Node* _infoPopup;

  int _currentAmount;
  int _maxAmount;
  int _production;
  Type _type;
};

#endif  // __RESOURCE_WIDGET_H__
