#ifndef __CLANS_H__
#define __CLANS_H__

#include "Utils/API/BaseURL.h"
#include "cocos2d.h"
#include "network/HttpClient.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

USING_NS_CC;

/**
 * 部落信息结构
 */
struct ClanInfo {
  std::string id;  // 部落ID
  std::string name;
  int member_count;

  ClanInfo() : id(""), name(""), member_count(0) {}
};

/**
 * 获取部落列表响应结果结构
 */
struct ClansListResult {
  bool success;
  std::string message;
  std::vector<ClanInfo> clans;

  ClansListResult() : success(false) {}
};

/**
 * 部落 API 类
 * 发送 GET 请求到 /api/clans/*
 */
class Clans {
 public:
  /**
   * 获取部落列表回调函数类型
   * @param result 结果
   */
  typedef std::function<void(const ClansListResult&)> ClansListCallback;

  /**
   * 获取所有部落的简要信息
   * @param callback 结果回调函数
   */
  static void getAllClansInfo(ClansListCallback callback);

  /**
   * 搜索部落（按名称）
   * @param name_keyword 部落名称关键词
   * @param callback 结果回调函数
   */
  static void searchClans(const std::string& name_keyword,
                          ClansListCallback callback);

  /**
   * 加入部落回调函数类型
   * @param success 是否成功
   * @param message 消息
   */
  typedef std::function<void(bool success, const std::string& message)> JoinClanCallback;

  /**
   * 加入部落
   * @param clan_id 部落ID
   * @param user_id 用户ID
   * @param callback 结果回调函数
   */
  static void joinClan(const std::string& clan_id, int user_id, JoinClanCallback callback);

  /**
   * 获取部落成员回调函数类型
   * @param success 是否成功
   * @param message 消息
   * @param members 成员列表（用户名列表）
   */
  typedef std::function<void(bool success, const std::string& message, const std::vector<std::string>& members)> ClanMembersCallback;

  /**
   * 获取部落成员
   * @param clan_id 部落ID
   * @param callback 结果回调函数
   */
  static void getClanMembers(const std::string& clan_id, ClanMembersCallback callback);

  /**
   * 聊天消息结构
   */
  struct ChatMessage {
    std::string sender;    // 发送者用户名
    std::string content;   // 消息内容
    std::string time;      // 发送时间

    ChatMessage() : sender(""), content(""), time("") {}
  };

  /**
   * 获取聊天消息回调函数类型
   * @param success 是否成功
   * @param message 消息
   * @param messages 消息列表
   * @param count 总消息数
   */
  typedef std::function<void(bool success, const std::string& message, 
                             const std::vector<ChatMessage>& messages, int count)> ChatMessagesCallback;

  /**
   * 获取部落聊天室消息
   * @param clan_id 部落ID
   * @param limit 可选，返回条数限制
   * @param callback 结果回调函数
   */
  static void getClanChatMessages(const std::string& clan_id, int limit, ChatMessagesCallback callback);

  /**
   * 发送聊天消息回调函数类型
   * @param success 是否成功
   * @param message 消息
   */
  typedef std::function<void(bool success, const std::string& message)> SendChatMessageCallback;

  /**
   * 发送部落聊天室消息
   * @param clan_id 部落ID
   * @param user_id 用户ID
   * @param content 消息内容
   * @param callback 结果回调函数
   */
  static void sendClanChatMessage(const std::string& clan_id, int user_id, 
                                   const std::string& content, SendChatMessageCallback callback);
};

#endif  // __CLANS_H__

