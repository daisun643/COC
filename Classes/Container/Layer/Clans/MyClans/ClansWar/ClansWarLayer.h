#ifndef __CLANS_WAR_LAYER_H__
#define __CLANS_WAR_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;

/**
 * ClansWarLayer
 * - 如果未开启部落战：
 *   - 所有者：显示“开启部落战”按钮（调用 /clanswar/start）
 *   - 其他人：显示 Label 提示“部落战尚未开启”
 * - 如果已开启：展示倒计时、每张地图的 stars、剩余进攻次数与进攻按钮
 */
class ClansWarLayer : public Layer {
 public:
  static ClansWarLayer* create();
  virtual bool init();

  // 外部调用刷新（例如 MyClansLayer 切换到本标签时）
  void refresh();

 private:
  void buildUI();
  void showNotStartedUI();
  void showStartedUI(const std::string& warId);
  void onStartWarPressed();
  void loadOverview();
  void onAttackPressed(const std::string& mapId);

  Layer* _contentArea;
  ui::ScrollView* _scrollView;
  LayerColor* buttonBg;
  Label* _statusLabel;          // 用于未开启提示或倒计时
  ui::Button* _startWarButton;  // 所有者可见
  std::string _currentWarId;
  bool _isOwner;
  bool _warStarted;

  void checkOwner();
};

#endif  // __CLANS_WAR_LAYER_H__