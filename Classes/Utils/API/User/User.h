#ifndef __USER_API_H__
#define __USER_API_H__

#include "Utils/API/BaseURL.h"
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

class UserAPI {
 public:
  typedef std::function<void(bool success, const std::string& message)> SetClanCallback;
  typedef std::function<void(bool success, const std::string& message, const std::string& clan_id)> GetClanCallback;

  // 设置用户的 clan_id
  static void setClanId(const std::string& user_id, const std::string& clan_id, SetClanCallback callback);

  // 获取用户的 clan_id
  static void getClanId(const std::string& user_id, GetClanCallback callback);
};

#endif  // __USER_API_H__
// 在前端封装server\app\api\user.py中的set_clanid和get_clanid接口
// 参考Classes\Utils\API\Clans\Clans.cpp实现