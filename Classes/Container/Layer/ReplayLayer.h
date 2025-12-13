#ifndef __REPLAY_LAYER_H__
#define __REPLAY_LAYER_H__

#include <functional>

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class ReplayLayer : public cocos2d::LayerColor {
 public:
  static ReplayLayer* create();
  virtual bool init();

  // Callback when a replay is selected
  void setOnReplaySelectedCallback(
      std::function<void(const std::string& recordPath)> callback);

 private:
  void buildUI();
  cocos2d::ui::Widget* createReplayItem(const std::string& recordPath,
                                        const std::string& mapPath,
                                        const std::string& opponentName,
                                        bool isVictory,
                                        const std::string& timeStr);

  cocos2d::ui::Layout* _panel;
  cocos2d::ui::ScrollView* _scrollView;
  std::function<void(const std::string&)> _onReplaySelected;
};

#endif  // __REPLAY_LAYER_H__
