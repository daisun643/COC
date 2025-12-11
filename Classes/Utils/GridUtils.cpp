#include "Utils/GridUtils.h"

#include "Manager/Config/ConfigManager.h"

Vec2 GridUtils::gridToScene(float row, float col, const Vec2& p00) {
  // 从 ConfigManager 获取 deltaX 和 deltaY
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not initialized in GridUtils::gridToScene");
    return Vec2::ZERO;
  }
  auto constantConfig = configManager->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;

  Vec2 pos;
  pos.x = p00.x + (row + col) * deltaX;
  pos.y = p00.y + (col - row) * deltaY;
  return pos;
}

bool GridUtils::screenToGrid(const Vec2& screenPos, const Vec2& p00, float& row,
                             float& col) {
  // 从 ConfigManager 获取 deltaX, deltaY 和 gridSize
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not initialized in GridUtils::screenToGrid");
    return false;
  }
  auto constantConfig = configManager->getConstantConfig();
  float deltaX = constantConfig.deltaX;
  float deltaY = constantConfig.deltaY;
  int gridSize = constantConfig.gridSize;

  // 从屏幕坐标转换为网格坐标
  // 使用逆变换公式
  float dx = screenPos.x - p00.x;
  float dy = screenPos.y - p00.y;

  // 解方程组：
  // dx = (row + col) * deltaX
  // dy = (col - row) * deltaY
  //
  // 解得：
  // col = (dx/deltaX + dy/deltaY) / 2
  // row = (dx/deltaX - dy/deltaY) / 2

  col = (dx / deltaX + dy / deltaY) / 2.0f;
  row = (dx / deltaX - dy / deltaY) / 2.0f;

  // 检查是否在有效范围内
  return (row >= 0.0f && row <= static_cast<float>(gridSize) && col >= 0.0f &&
          col <= static_cast<float>(gridSize));
}

bool GridUtils::findNearestGrassVertex(const Vec2& screenPos, const Vec2& p00,
                                       float& row, float& col,
                                       Vec2& nearestPos) {
  // 从 ConfigManager 获取 gridCount
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not initialized in GridUtils::findNearestGrassVertex");
    return false;
  }
  auto constantConfig = configManager->getConstantConfig();
  int gridSize = constantConfig.gridSize;

  // 先转换为网格坐标
  float tempRow, tempCol;
  if (!screenToGrid(screenPos, p00, tempRow, tempCol)) {
    return false;
  }

  // 检查周围的几个点，找到最近的
  float minDist = FLT_MAX;
  float bestRow = tempRow, bestCol = tempCol;

  // 将浮点坐标四舍五入到最近的整数，然后检查周围的点
  int centerRow = static_cast<int>(tempRow + 0.5f);
  int centerCol = static_cast<int>(tempCol + 0.5f);

  for (int r = centerRow - 1; r <= centerRow + 1; ++r) {
    for (int c = centerCol - 1; c <= centerCol + 1; ++c) {
      if (r >= 0 && r <= gridSize && c >= 0 && c < gridSize) {
        Vec2 gridPos =
            gridToScene(static_cast<float>(r), static_cast<float>(c), p00);
        float dist = screenPos.distance(gridPos);
        if (dist < minDist) {
          minDist = dist;
          bestRow = static_cast<float>(r);
          bestCol = static_cast<float>(c);
        }
      }
    }
  }

  row = bestRow;
  col = bestCol;
  nearestPos = gridToScene(row, col, p00);
  return true;
}
