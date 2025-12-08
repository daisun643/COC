#ifndef __RECORD_SCENE_H__
#define __RECORD_SCENE_H__

#include <string>
#include <vector>

#include "Container/Scene/Basic/BasicScene.h"
#include "Game/Soldier/BasicSoldier.h"
#include "Game/Spell/BasicSpell.h"
#include "Manager/Record/RecordManager.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

/**
 * 回放场景
 * 根据JSON记录文件回放进攻过程
 */
class RecordScene : public BasicScene {
 public:
  static Scene* createScene();

  virtual bool init() override;

  CREATE_FUNC(RecordScene);

  virtual ~RecordScene();

 private:
  /**
   * 加载JSON记录文件
   */
  bool loadRecordFile(const std::string& filePath);

  /**
   * 开始回放
   */
  void startPlayback();

  /**
   * 停止回放
   */
  void stopPlayback();

  /**
   * 回放更新函数
   */
  void updatePlayback(float dt);

  /**
   * 根据记录创建士兵
   */
  void createSoldierFromRecord(const PlacementRecord& record);

  /**
   * 根据记录创建法术
   */
  void createSpellFromRecord(const PlacementRecord& record);

  /**
   * 创建回放控制按钮
   */
  void createPlaybackButtons();

  std::vector<PlacementRecord> _records;      // 记录列表
  std::vector<BasicSoldier*> _placedSoldiers;  // 已布置的士兵列表
  std::vector<BasicSpell*> _activeSpells;      // 活跃的法术列表

  // 回放控制相关
  cocos2d::ui::Button* _playButton;    // 播放按钮
  cocos2d::ui::Button* _pauseButton;    // 暂停按钮
  cocos2d::ui::Button* _stopButton;    // 停止按钮
  Label* _timeLabel;                   // 时间标签
  bool _isPlaying;                     // 是否正在播放
  bool _isPaused;                      // 是否暂停
  float _currentTime;                  // 当前回放时间（秒）
  size_t _currentRecordIndex;         // 当前记录索引
  float _playbackSpeed;                // 回放速度倍数
};

#endif  // __RECORD_SCENE_H__
