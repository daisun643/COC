#ifndef __PLAYER_MANAGER_H__
#define __PLAYER_MANAGER_H__

#include <functional>

#include "cocos2d.h"

class PlayerManager {
 public:
  static PlayerManager* getInstance();
  static void destroyInstance();

  bool init();

  // Getter
  int getGold() const { return _gold; }
  int getElixir() const { return _elixir; }
  int getMaxGold() const { return _maxGold; }
  int getMaxElixir() const { return _maxElixir; }
  int getGoldProduction() const { return _goldProduction; }
  int getElixirProduction() const { return _elixirProduction; }

  // Setter / Modifiers
  void setGold(int amount);
  void setElixir(int amount);
  void setMaxGold(int amount) { _maxGold = amount; }
  void setMaxElixir(int amount) { _maxElixir = amount; }
  void setGoldProduction(int amount) { _goldProduction = amount; }
  void setElixirProduction(int amount) { _elixirProduction = amount; }

  void addGold(int amount);
  void addElixir(int amount);

  bool consumeGold(int amount);
  bool consumeElixir(int amount);

  // 新增保存和加载方法
  void saveUserData();
  bool loadUserData();

  // 设置自动保存回调函数
  void setAutoSaveCallback(const std::function<void()>& callback);

  bool isNewGame() const { return _isNewGame; }
  void setIsNewGame(bool isNew) { _isNewGame = isNew; }

 private:
  PlayerManager();
  ~PlayerManager();

  static PlayerManager* _instance;

  bool _isNewGame;
  int _gold;
  int _elixir;
  int _maxGold;
  int _maxElixir;
  int _goldProduction;
  int _elixirProduction;

  // 自动保存回调
  std::function<void()> _autoSaveCallback;
};

#endif  // __PLAYER_MANAGER_H__
