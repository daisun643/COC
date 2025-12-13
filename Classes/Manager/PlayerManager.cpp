#include "PlayerManager.h"

PlayerManager* PlayerManager::_instance = nullptr;

PlayerManager* PlayerManager::getInstance() {
  if (_instance == nullptr) {
    _instance = new (std::nothrow) PlayerManager();
    if (_instance && _instance->init()) {
      // 成功初始化
    } else {
      CC_SAFE_DELETE(_instance);
    }
  }
  return _instance;
}

void PlayerManager::destroyInstance() { CC_SAFE_DELETE(_instance); }

PlayerManager::PlayerManager()
    : _gold(0),
      _elixir(0),
      _maxGold(10000),
      _maxElixir(10000),
      _goldProduction(0),
      _elixirProduction(0) {}

PlayerManager::~PlayerManager() {}

bool PlayerManager::init() {
  // 初始化默认资源，以后可以从存档读取
  _gold = 5000;
  _elixir = 500;
  _maxGold = 10000;
  _maxElixir = 10000;
  _goldProduction = 100;
  _elixirProduction = 100;
  return true;
}

void PlayerManager::setGold(int amount) {
  _gold = amount;
  // 这里可以发送一个事件通知UI更新，或者让UI在update里轮询
  // 为了简单起见，我们暂时让UI主动获取，或者在MainUILayer里监听变化
}

void PlayerManager::setElixir(int amount) { _elixir = amount; }

void PlayerManager::addGold(int amount) {
  _gold += amount;
  if (_gold > _maxGold) {
    _gold = _maxGold;
  }
}

void PlayerManager::addElixir(int amount) {
  _elixir += amount;
  if (_elixir > _maxElixir) {
    _elixir = _maxElixir;
  }
}

bool PlayerManager::consumeGold(int amount) {
  if (_gold >= amount) {
    _gold -= amount;
    return true;
  }
  return false;
}

bool PlayerManager::consumeElixir(int amount) {
  if (_elixir >= amount) {
    _elixir -= amount;
    return true;
  }
  return false;
}
