#ifndef __BATTLE_H__
#define __BATTLE_H__

#include <functional>
#include <string>

#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

/**
 * 战斗相关 API
 */
class Battle {
 public:
  /**
   * 搜索对手
   * @param userId 用户ID
   * @param callback 回调函数，参数为 (success, message, opponentId, opponentName, mapJsonData)
   */
  static void searchOpponent(
      int userId,
      std::function<void(bool success, const std::string& message,
                         int opponentId, const std::string& opponentName,
                         const std::string& mapJsonData)> callback);
};

#endif  // __BATTLE_H__

