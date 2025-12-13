#include "SenceHelper.h"

SceneHelper* SceneHelper::_instance = nullptr;

SceneHelper* SceneHelper::getInstance() {
  if (_instance == nullptr) {
    _instance = new (std::nothrow) SceneHelper();
  }
  return _instance;
}

void SceneHelper::setGameScene(Scene* scene) {
  // 如果之前有场景，先释放
  if (_gameScene) {
    _gameScene->release();
  }
  _gameScene = scene;
}

Scene* SceneHelper::getGameScene() const { return _gameScene; }

void SceneHelper::clearGameScene() {
  if (_gameScene) {
    _gameScene->release();
    _gameScene = nullptr;
  }
}
