#include "RecordManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Utils/PathUtils.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "platform/CCFileUtils.h"

RecordManager::RecordManager() : _isRecording(false) {}

RecordManager::~RecordManager() { clear(); }

bool RecordManager::init() {
  clear();
  return true;
}

void RecordManager::startAttack() {
  clear();
  _attackStartTime = std::chrono::steady_clock::now();
  _isRecording = true;
  CCLOG("RecordManager: Attack started, recording enabled");
}

void RecordManager::recordTroopPlacement(const std::string& category, int level,
                                         float x, float y, int timestamp) {
  if (!_isRecording) {
    return;
  }

  PlacementRecord record;
  record.type = "troop";
  record.category = category;
  record.level = level;
  record.x = x;
  record.y = y;
  record.timestamp = timestamp;

  _records.push_back(record);
  CCLOG(
      "RecordManager: Recorded troop placement - %s Lv%d at (%.1f, %.1f) @ %ds",
      category.c_str(), level, x, y, timestamp);
}

void RecordManager::recordSpellPlacement(const std::string& category, float x,
                                         float y, int timestamp) {
  if (!_isRecording) {
    return;
  }

  PlacementRecord record;
  record.type = "spell";
  record.category = category;
  record.level = 0;  // 法术没有等级
  record.x = x;
  record.y = y;
  record.timestamp = timestamp;

  _records.push_back(record);
  CCLOG("RecordManager: Recorded spell placement - %s at (%.1f, %.1f) @ %ds",
        category.c_str(), x, y, timestamp);
}

int RecordManager::getCurrentTimestamp() const {
  if (!_isRecording) {
    return 0;
  }

  auto now = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::seconds>(now - _attackStartTime);
  return static_cast<int>(duration.count());
}

bool RecordManager::endAttackAndSave(const std::string& filePath) {
  if (!_isRecording) {
    CCLOG("RecordManager: Not recording, nothing to save");
    return false;
  }

  // 构建JSON文档
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

  // 创建记录数组
  rapidjson::Value recordsArray(rapidjson::kArrayType);

  for (const auto& record : _records) {
    rapidjson::Value recordObj(rapidjson::kObjectType);

    // 添加类型
    rapidjson::Value typeValue;
    typeValue.SetString(record.type.c_str(), allocator);
    recordObj.AddMember("type", typeValue, allocator);

    // 添加类别
    rapidjson::Value categoryValue;
    categoryValue.SetString(record.category.c_str(), allocator);
    recordObj.AddMember("category", categoryValue, allocator);

    // 添加等级（如果有）
    if (record.type == "troop") {
      recordObj.AddMember("level", record.level, allocator);
    }

    // 添加坐标
    recordObj.AddMember("x", record.x, allocator);
    recordObj.AddMember("y", record.y, allocator);

    // 添加时间戳
    recordObj.AddMember("timestamp", record.timestamp, allocator);

    recordsArray.PushBack(recordObj, allocator);
  }

  doc.AddMember("records", recordsArray, allocator);

  // 添加元数据
  rapidjson::Value metadata(rapidjson::kObjectType);
  metadata.AddMember("totalRecords", static_cast<int>(_records.size()),
                     allocator);
  rapidjson::Value endTimeValue;
  endTimeValue.SetString(std::to_string(getCurrentTimestamp()).c_str(),
                         allocator);
  metadata.AddMember("duration", getCurrentTimestamp(), allocator);
  doc.AddMember("metadata", metadata, allocator);

  // 将JSON转换为字符串
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  std::string jsonString = buffer.GetString();
  // 使用 PathUtils 获取真实写入路径
  std::string fullPath = PathUtils::getRealFilePath(filePath, true);

  // [新增] 确保目录存在
  PathUtils::ensureDirectoryExists(fullPath);

  // 确保目录存在 (PathUtils 返回的是文件路径，我们需要父目录)
  // 确保目录存在
  size_t pos = fullPath.find_last_of("/\\");
  if (pos != std::string::npos) {
    std::string dir = fullPath.substr(0, pos);
// 创建目录（如果不存在）
#ifdef _WIN32
    // Windows下使用_mkdir，如果目录不存在则创建
    if (_access(dir.c_str(), 0) != 0) {
      _mkdir(dir.c_str());
    }
#else
    // Linux/Mac下使用mkdir
    struct stat info;
    if (stat(dir.c_str(), &info) != 0) {
      mkdir(dir.c_str(), 0755);
    }
#endif
  }

  // 写入文件
  std::ofstream outFile(fullPath, std::ios::out | std::ios::trunc);
  if (!outFile.is_open()) {
    CCLOG("RecordManager: Failed to open file for writing: %s",
          fullPath.c_str());
    return false;
  }

  outFile << jsonString;
  outFile.close();

  CCLOG("RecordManager: Saved %zu records to %s", _records.size(),
        fullPath.c_str());

  // 停止记录
  _isRecording = false;

  return true;
}

void RecordManager::clear() {
  _records.clear();
  _isRecording = false;
}
