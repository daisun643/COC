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
    std::function<bool(int, int)> isWalkable) {
  float startRow, startCol, endRow, endCol;

  // 转换坐标
  if (!GridUtils::screenToGrid(startPos, p00, startRow, startCol) ||
      !GridUtils::screenToGrid(endPos, p00, endRow, endCol)) {
    return {};
  }

  int sRow = static_cast<int>(std::round(startRow));
  int sCol = static_cast<int>(std::round(startCol));
  int eRow = static_cast<int>(std::round(endRow));
  int eCol = static_cast<int>(std::round(endCol));

  // 如果起点和终点相同，直接返回终点
  if (sRow == eRow && sCol == eCol) {
    return {endPos};
  }

  // 检查终点是否可通行，如果不可通行，寻找最近的可通行点
  if (!isWalkable(eRow, eCol)) {
    bool found = false;
    int radius = 1;
    // 限制搜索范围，避免搜索全图
    while (radius <= 5 && !found) {
      for (int r = eRow - radius; r <= eRow + radius; ++r) {
        for (int c = eCol - radius; c <= eCol + radius; ++c) {
          // 只检查边缘
          if (std::abs(r - eRow) == radius || std::abs(c - eCol) == radius) {
            if (isWalkable(r, c)) {
              eRow = r;
              eCol = c;
              found = true;
              break;
            }
          }
        }
        if (found) break;
      }
      radius++;
    }
    // 如果找不到可通行的终点，返回空路径
    if (!found) {
      return {};
    }
  }

  // A* 算法初始化
  std::priority_queue<PathNode*, std::vector<PathNode*>, NodeCompare> openList;
  std::vector<PathNode*> allNodes;  // 用于内存管理

  // 使用简单的二维数组标记已访问，假设地图最大 100x100
  // 或者使用 set
  // 为了性能，这里使用简单的 set 记录坐标
  std::unordered_set<long long> closedSet;
  auto coordToLong = [](int r, int c) -> long long {
    return (static_cast<long long>(r) << 32) | static_cast<unsigned int>(c);
  };

  PathNode* startNode = new PathNode(sRow, sCol);
  startNode->g = 0;
  startNode->h = std::abs(sRow - eRow) + std::abs(sCol - eCol);  // 曼哈顿距离

  openList.push(startNode);
  allNodes.push_back(startNode);

  PathNode* destNode = nullptr;

  // 8方向移动偏移
  int dr[] = {0, 0, 1, -1, 1, 1, -1, -1};
  int dc[] = {1, -1, 0, 0, 1, -1, 1, -1};
  int cost[] = {10, 10, 10, 10, 14, 14, 14, 14};  // 直线代价10，对角线14

  while (!openList.empty()) {
    PathNode* current = openList.top();
    openList.pop();

    // 如果已经在 closedSet 中，跳过
    if (closedSet.count(coordToLong(current->row, current->col))) {
      continue;
    }
    closedSet.insert(coordToLong(current->row, current->col));

    // 到达终点（或者终点不可达时到达终点附近）
    // 注意：如果终点是建筑，通常是不可通行的。
    // 所以我们需要找到终点或者终点相邻的格子。
    // 这里假设 isWalkable 会对终点返回 true (如果它是目标)
    // 或者我们需要特殊处理。
    // 实际上，士兵是攻击建筑，所以目标点本身是不可通行的（被建筑占据）。
    // 我们应该寻找目标周围的可通行点。
    // 但为了简化，我们假设 findPath 的 endPos 是目标建筑边缘的一个可通行点。
    // 或者，我们在判断 isWalkable 时，如果坐标等于 eRow, eCol，则视为可通行。

    if (current->row == eRow && current->col == eCol) {
      destNode = current;
      break;
    }

    // 遍历邻居
    for (int i = 0; i < 8; ++i) {
      int nr = current->row + dr[i];
      int nc = current->col + dc[i];

      // 检查是否可通行
      if (!isWalkable(nr, nc)) {
        continue;
      }

      if (closedSet.count(coordToLong(nr, nc))) {
        continue;
      }

      int newG = current->g + cost[i];

      PathNode* neighbor = new PathNode(nr, nc);
      neighbor->g = newG;
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
      path.push_back(GridUtils::gridToScene(curr->row, curr->col, p00));
      curr = curr->parent;
    }
    // 路径是反向的，需要翻转
    std::reverse(path.begin(), path.end());

    // 移除起点（因为已经在起点了）
    if (!path.empty()) {
      path.erase(path.begin());
    }
  }

  // 清理内存
  for (PathNode* node : allNodes) {
    delete node;
  }

  return path;
}
