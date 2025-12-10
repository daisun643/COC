#ifndef __MAIN_UI_LAYER_H__
#define __MAIN_UI_LAYER_H__

#include "Container/Node/ResourceWidget.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class MainUILayer : public cocos2d::Layer {
 public:
  CREATE_FUNC(MainUILayer);
  virtual bool init();
  virtual void update(float dt);  // 用于每帧更新资源显示

  // 设置回调
  void setOnShopClickCallback(std::function<void()> callback);
  void setOnAttackClickCallback(std::function<void()> callback);
  void setOnReplayClickCallback(std::function<void()> callback);

 private:
  ResourceWidget* _goldWidget;
  ResourceWidget* _elixirWidget;

  cocos2d::ui::Button* _shopButton;
  cocos2d::ui::Button* _attackButton;
  cocos2d::ui::Button* _replayButton;

  std::function<void()> _onShopClick;
  std::function<void()> _onAttackClick;
  std::function<void()> _onReplayClick;
};

#endif  // __MAIN_UI_LAYER_H__
