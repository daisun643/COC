#ifndef __CLANS_WAR_H__
#define __CLANS_WAR_H__

#include "Utils/API/BaseURL.h"
#include "Utils/API/URLEncoder.h"
#include "cocos2d.h"
#include "json/document.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

// 单张地图的概览信息
struct WarMapOverview {
  std::string id;
  int stars;
  int cnt;
  WarMapOverview() : id(""), stars(0), cnt(0) {}
};

// 概览结果
struct WarOverviewResult {
  bool success;
  std::string message;
  std::vector<WarMapOverview> maps;
  WarOverviewResult() : success(false) {}
};

class ClansWar {
 public:
  typedef std::function<void(bool success, const std::string& message,
                             const std::string& war_id,
                             const std::string& history_path)>
      StartWarCallback;

  typedef std::function<void(const WarOverviewResult&)> WarOverviewCallback;

  typedef std::function<void(bool success, const std::string& message,
                             const std::string& map_data_json)>
      GetMapCallback;

  typedef std::function<void(bool success, const std::string& message)>
      SaveMapCallback;

  // 开启部落战
  static void startWar(const std::string& clans_id, StartWarCallback callback);

  // 获取部落战概览
  static void getWarOverview(const std::string& clans_id,
                             WarOverviewCallback callback);

  // 获取某张部落战地图（返回 JSON 字符串）
  static void getWarMap(const std::string& clans_id, const std::string& map_id,
                        GetMapCallback callback);

  // 保存/更新部落战地图，map_data_json 为 JSON 字符串
  static void saveWarMap(const std::string& clans_id, const std::string& map_id,
                         const std::string& map_data_json,
                         SaveMapCallback callback);
};

#endif  // __CLANS_WAR_H__
