#include "Utils/GridUtils.h"

#include "Manager/Config/ConfigManager.h"

Vec2 GridUtils::gridToScene(int row, int col, const Vec2& p00) {
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

bool GridUtils::screenToGrid(const Vec2& screenPos, const Vec2& p00, int& row,
                             int& col) {
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

  float col_f = (dx / deltaX + dy / deltaY) / 2.0f;
  float row_f = (dx / deltaX - dy / deltaY) / 2.0f;

  // 四舍五入到最近的整数
  col = (int)(col_f + 0.5f);
  row = (int)(row_f + 0.5f);

  // 检查是否在有效范围内
  return (row >= 0 && row <= gridSize && col >= 0 && col <= gridSize);
}

bool GridUtils::findNearestGrassVertex(const Vec2& screenPos, const Vec2& p00,
                                       int& row, int& col, Vec2& nearestPos) {
  // 从 ConfigManager 获取 gridCount
  auto configManager = ConfigManager::getInstance();
  if (!configManager) {
    CCLOG("ConfigManager not initialized in GridUtils::findNearestGrassVertex");
    return false;
  }
  auto constantConfig = configManager->getConstantConfig();
  int gridSize = constantConfig.gridSize;

  // 先转换为网格坐标
  int tempRow, tempCol;
  if (!screenToGrid(screenPos, p00, tempRow, tempCol)) {
    return false;
  }

  // 检查周围的几个点，找到最近的
  float minDist = FLT_MAX;
  int bestRow = tempRow, bestCol = tempCol;

  for (int r = tempRow - 1; r <= tempRow + 1; ++r) {
    for (int c = tempCol - 1; c <= tempCol + 1; ++c) {
      if (r >= 0 && r <= gridSize && c >= 0 && c < gridSize) {
        Vec2 gridPos = gridToScene(r, c, p00);
        float dist = screenPos.distance(gridPos);
        if (dist < minDist) {
          minDist = dist;
          bestRow = r;
          bestCol = c;
        }
      }
    }
  }

  row = bestRow;
  col = bestCol;
  nearestPos = gridToScene(row, col, p00);
  return true;
}
