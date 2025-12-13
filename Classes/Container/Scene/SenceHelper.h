#ifndef __SCENE_HELPER_H__
#define __SCENE_HELPER_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 场景管理器
 * 用于维护场景实例，支持场景间的切换和恢复
 */
class SceneHelper {
 public:
  /**
   * 获取单例实例
   */
  static SceneHelper* getInstance();

  /**
   * 保存 GameScene 实例
   * @param scene GameScene 实例
   */
  void setGameScene(Scene* scene);

  /**
   * 获取保存的 GameScene 实例
   * @return GameScene 实例，如果不存在则返回 nullptr
   */
  Scene* getGameScene() const;

  /**
   * 清除保存的 GameScene 实例
   */
  void clearGameScene();

 private:
  SceneHelper() : _gameScene(nullptr) {}
  ~SceneHelper() {}

  static SceneHelper* _instance;
  Scene* _gameScene;  // 保存的 GameScene 实例
};

#endif  // __SCENE_HELPER_H__

