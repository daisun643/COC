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
  int getGems() const { return _gems; }
  int getMaxGold() const { return _maxGold; }
  int getMaxElixir() const { return _maxElixir; }
  int getGoldProduction() const { return _goldProduction; }
  int getElixirProduction() const { return _elixirProduction; }

  // Setter / Modifiers
  void setGold(int amount);
  void setElixir(int amount);
  void setGems(int amount) { _gems = amount; }
  // 只有在允许更新时（即在主城时）才更新上限，防止进攻时被敌人的空上限覆盖
  void setMaxGold(int amount) { 
      if (_allowUpdateMaxLimit) _maxGold = amount; 
  }
  void setMaxElixir(int amount) { 
      if (_allowUpdateMaxLimit) _maxElixir = amount; 
  }
  void setGoldProduction(int amount) { _goldProduction = amount; }
  void setElixirProduction(int amount) { _elixirProduction = amount; }

  void addGold(int amount);
  void addElixir(int amount);
  void addGems(int amount);

  bool consumeGold(int amount);
  bool consumeElixir(int amount);
  bool consumeGems(int amount);

  // 新增保存和加载方法
  void saveUserData();
  bool loadUserData();

  // 设置自动保存回调函数
  void setAutoSaveCallback(const std::function<void()>& callback);

  bool isNewGame() const { return _isNewGame; }
  void setIsNewGame(bool isNew) { _isNewGame = isNew; }

  // 设置是否允许更新资源上限
  void setAllowUpdateMaxLimit(bool allow) { _allowUpdateMaxLimit = allow; }

 private:
  PlayerManager();
  ~PlayerManager();

  static PlayerManager* _instance;

  bool _isNewGame;
  int _gold;
  int _elixir;
  int _gems;
  int _maxGold;
  int _maxElixir;
  int _goldProduction;
  int _elixirProduction;
  // 控制标志位
  bool _allowUpdateMaxLimit;

  // 自动保存回调
  std::function<void()> _autoSaveCallback;
};

#endif  // __PLAYER_MANAGER_H__
