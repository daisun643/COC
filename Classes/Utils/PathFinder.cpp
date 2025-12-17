#include "PathFinder.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

#include "Utils/GridUtils.h"

struct PathNode {
  int row, col;
  int g, h;
  PathNode* parent;

  PathNode(int r, int c) : row(r), col(c), g(0), h(0), parent(nullptr) {}
  int getF() const { return g + h; }

  bool operator==(const PathNode& other) const {
    return row == other.row && col == other.col;
  }
};

// 哈希函数，用于 unordered_set
struct NodeHash {
  std::size_t operator()(const PathNode* node) const {
    return std::hash<int>()(node->row) ^ (std::hash<int>()(node->col) << 1);
  }
};

// 比较函数，用于优先队列
struct NodeCompare {
  bool operator()(const PathNode* a, const PathNode* b) const {
    return a->getF() > b->getF();
  }
};

std::vector<Vec2> PathFinder::findPath(
    const Vec2& startPos, const Vec2& endPos, const Vec2& p00,
    const std::function<bool(int, int)>& isWalkable, int precision) {
  float startRow, startCol, endRow, endCol;

  // 转换坐标
  if (!GridUtils::screenToGrid(startPos, p00, startRow, startCol) ||
      !GridUtils::screenToGrid(endPos, p00, endRow, endCol)) {
    return {};
  }

  // 使用精度倍增坐标
  int sRow = static_cast<int>(std::round(startRow * precision));
  int sCol = static_cast<int>(std::round(startCol * precision));
  int eRow = static_cast<int>(std::round(endRow * precision));
  int eCol = static_cast<int>(std::round(endCol * precision));

  // 如果起点和终点相同，直接返回终点
  if (sRow == eRow && sCol == eCol) {
    return {endPos};
  }

  // 包装 isWalkable 回调，处理精度转换
  auto isWalkablePrecise = [&](int r, int c) -> bool {
    // 将高精度坐标转换回原始网格坐标
    // 使用浮点除法以获得更准确的中心点判断，或者直接整数除法
    // 这里使用整数除法，意味着一个大格子被分为 precision*precision 个小格子
    // 只要大格子不可通行，所有小格子都不可通行
    return isWalkable(r / precision, c / precision);
  };

  // 目标点集合（如果终点不可通行，则这里存储所有可能的替代终点）
  std::unordered_set<long long> validEndPoints;
  auto coordToLong = [](int r, int c) -> long long {
    return (static_cast<long long>(r) << 32) | static_cast<unsigned int>(c);
  };

  // 检查终点是否可通行
  if (isWalkablePrecise(eRow, eCol)) {
    // 如果终点本身可通行，则只有一个目标点
    validEndPoints.insert(coordToLong(eRow, eCol));
  } else {
    // 如果终点不可通行（例如是建筑），寻找周围所有可通行点作为潜在目标
    int radius = 1;
    // 搜索范围
    while (radius <= 5 * precision) {
      for (int r = eRow - radius; r <= eRow + radius; ++r) {
        for (int c = eCol - radius; c <= eCol + radius; ++c) {
          // 只检查边缘
          if (std::abs(r - eRow) == radius || std::abs(c - eCol) == radius) {
            if (isWalkablePrecise(r, c)) {
              validEndPoints.insert(coordToLong(r, c));
            }
          }
        }
      }
      // 如果在当前半径圈内找到了可通行点，我们是否应该停止？
      // 为了找到"最近"的路径，我们应该尽可能多地收集候选点。
      // 但为了性能，我们不能无限搜索。
      // 策略：一旦找到至少一个可通行点，再多搜索一圈（为了覆盖角落），然后停止。
      if (!validEndPoints.empty() &&
          radius > 2 * precision) {  // 至少搜索一定范围
        break;
      }
      radius++;
    }

    // 如果找不到任何可通行的终点，返回空路径
    if (validEndPoints.empty()) {
      return {};
    }
  }

  // A* 算法初始化
  std::priority_queue<PathNode*, std::vector<PathNode*>, NodeCompare> openList;
  std::vector<PathNode*> allNodes;  // 用于内存管理

  // 使用简单的 set 记录已访问坐标
  std::unordered_set<long long> closedSet;

  PathNode* startNode = new PathNode(sRow, sCol);
  startNode->g = 0;
  // 启发式函数：距离目标中心的曼哈顿距离
  startNode->h = std::abs(sRow - eRow) + std::abs(sCol - eCol);

  openList.push(startNode);
  allNodes.push_back(startNode);

  PathNode* destNode = nullptr;

  // 8方向移动偏移
  int dr[] = {0, 0, 1, -1, 1, 1, -1, -1};
  int dc[] = {1, -1, 0, 0, 1, -1, 1, -1};
  int cost[] = {10, 10, 10, 10, 14, 14, 14, 14};  // 直线代价10，对角线14

  int steps = 0;
  const int MAX_STEPS = 10000;  // 限制最大搜索步数，防止卡死

  while (!openList.empty()) {
    if (++steps > MAX_STEPS) {
      break;
    }
    PathNode* current = openList.top();
    openList.pop();

    // 如果已经在 closedSet 中，跳过
    if (closedSet.count(coordToLong(current->row, current->col))) {
      continue;
    }
    closedSet.insert(coordToLong(current->row, current->col));

    // 检查是否到达任何一个有效终点
    if (validEndPoints.count(coordToLong(current->row, current->col))) {
      // [优化] 找到一个有效终点后，不立即停止，而是检查它是否是"最近"的。
      // 在A*中，第一个被从openList取出的终点通常就是代价最小（最近）的。
      // 但由于我们有多个有效终点，且启发式函数是指向目标中心的，
      // 所以第一个到达的边缘点可能不是离起点最近的边缘点（虽然路径代价是最小的）。
      // 不过，对于游戏寻路来说，"路径代价最小"通常就是我们想要的（走最少的路到达任意一个攻击位）。
      // 所以这里直接 break 是合理的。
      destNode = current;
      break;
    }

    // 遍历邻居
    for (int i = 0; i < 8; ++i) {
      int nr = current->row + dr[i];
      int nc = current->col + dc[i];

      // 检查是否可通行
      if (!isWalkablePrecise(nr, nc)) {
        continue;
      }

      if (closedSet.count(coordToLong(nr, nc))) {
        continue;
      }

      int newG = current->g + cost[i];

      PathNode* neighbor = new PathNode(nr, nc);
      neighbor->g = newG;
      // 启发式函数仍然使用到目标中心的距离
      neighbor->h = (std::abs(nr - eRow) + std::abs(nc - eCol)) * 10;
      neighbor->parent = current;

      openList.push(neighbor);
      allNodes.push_back(neighbor);
    }
  }

  std::vector<Vec2> path;
  if (destNode) {
    // 回溯路径
    PathNode* curr = destNode;
    while (curr != nullptr) {
      // 转换为屏幕坐标
      // 注意：这里需要除以 precision 转换回原始网格坐标（浮点数）
      // 修正：加上 0.5f 偏移量，使路径点位于子网格的中心，而不是顶点
      path.push_back(GridUtils::gridToScene(
          (curr->row + 0.5f) / precision, (curr->col + 0.5f) / precision, p00));
      curr = curr->parent;
    }
    // 路径是反向的，需要翻转
    std::reverse(path.begin(), path.end());

    // 移除起点（因为已经在起点了）
    // [修正] 如果路径只包含一个点（起点即终点），不要移除它。
    // 否则返回空路径会被调用者误判为"寻路失败"，导致触发保底逻辑（如攻击墙壁）。
    // 保留这个点可以让调用者知道"路径存在，且我已经到达最佳位置"。
    if (path.size() > 1) {
      path.erase(path.begin());
    }
  }

  // 清理内存
  for (PathNode* node : allNodes) {
    delete node;
  }

  return path;
}
