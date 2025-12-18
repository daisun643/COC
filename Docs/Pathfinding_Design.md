# 士兵智能寻路算法设计方案

## 1. 背景与目标
当前项目中，士兵的移动逻辑采用简单的“直线逼近”方式（`BasicSoldier::moveToTarget`）。这种方式在遇到障碍物（如城墙、非目标建筑）时会产生“穿墙”或“卡住”的不真实表现。

为了提升游戏的策略深度和视觉真实感，我们将设计并实现一套基于 **A* (A-Star)** 算法的智能寻路系统。这将是本项目的一个核心技术亮点。

## 2. 核心需求
1.  **智能避障**：士兵能够识别地图上的障碍物（建筑、城墙），并自动规划绕行路径。
2.  **兵种差异化**：
    *   **地面单位**（如野蛮人）：受围墙和建筑阻挡，需要寻路。
    *   **空中单位**（如气球兵）：无视地形，保持直线移动。
    *   **特殊单位**（如野蛮人攻城）：可能优先攻击墙，寻路逻辑需特殊处理。
3.  **动态更新**：当障碍物（如城墙）被摧毁后，地图通行性应实时更新，士兵能利用缺口。

## 3. 算法选择：A* (A-Star)
考虑到游戏地图是基于网格（Grid）的（参考 `GridUtils`），A* 算法是最佳选择。它结合了 Dijkstra 算法的准确性和贪婪最佳优先搜索（BFS）的效率。

### 3.1 核心公式
$$ F = G + H $$
*   **G (Cost)**: 从起点移动到当前格子的实际代价。
*   **H (Heuristic)**: 从当前格子到目标格子的预估代价（启发式）。
    *   我们将使用 **曼哈顿距离 (Manhattan Distance)** 作为启发函数，因为地图只允许四方向或八方向移动。

## 4. 系统架构设计

### 4.1 地图网格数据 (`GridMap`)
我们需要构建一个逻辑网格来表示地图的通行状态。
*   **位置**: 建议集成在 `BuildingManager` 或独立的 `MapManager` 中。
*   **数据结构**: 二维数组 `int _gridMap[ROWS][COLS]`。
    *   `0`: 空地（可通行）。
    *   `1`: 建筑/障碍（不可通行）。
    *   `2`: 城墙（特殊障碍，炸弹人可视为目标）。
*   **接口**:
    *   `bool isWalkable(int row, int col)`
    *   `void updateGrid(int row, int col, int type)`

### 4.2 寻路器工具类 (`PathFinder`)
一个纯算法类，负责计算路径。

```cpp
struct Node {
    int row, col;
    int g, h;
    Node* parent;
    int getF() const { return g + h; }
};

class PathFinder {
public:
    /**
     * 计算路径
     * @param start 起点网格坐标
     * @param end 终点网格坐标
     * @param ignoreWalls 是否无视墙壁（空军）
     * @return 路径点列表（屏幕坐标或网格坐标）
     */
    static std::vector<Vec2> findPath(const Vec2& start, const Vec2& end, bool ignoreWalls);
};
```

### 4.3 士兵类集成 (`BasicSoldier`)
修改 `BasicSoldier` 以支持路径跟随。

*   **新增成员**:
    *   `std::vector<Vec2> _pathQueue`: 存储计算出的路径点序列。
    *   `int _currentPathIndex`: 当前正在前往的路径点索引。
*   **逻辑变更**:
    *   **原逻辑**: `moveToTarget` 直接修改坐标逼近 `_targetPosition`。
    *   **新逻辑**:
        1.  在 `findTarget` 确定目标后，调用 `PathFinder::findPath`。
        2.  将返回的路径存入 `_pathQueue`。
        3.  `moveToTarget` 每一帧让士兵向 `_pathQueue[_currentPathIndex]` 移动。
        4.  到达当前点后，`_currentPathIndex++`。
        5.  如果目标位置发生剧烈变化（极少情况）或障碍物改变，重新寻路。

## 5. 详细实现流程

### 第一阶段：基础 A* 实现
1.  **构建网格**: 在 `BuildingManager` 初始化时，根据所有建筑的位置填充 `_gridMap`。
2.  **算法编写**: 实现标准的 A* 算法，支持 8 方向移动（对角线代价略高，G值 +1.4）。
3.  **初步集成**: 让野蛮人使用寻路移动。

### 第二阶段：优化与平滑
1.  **路径平滑 (Path Smoothing)**: A* 生成的路径通常是“锯齿状”的。可以通过 **Floyd 算法** 或简单的 **视线检查 (Line of Sight)** 来优化路径，去除不必要的中间点。
    *   *原理*: 如果点 A 和点 C 之间没有障碍物，则可以直接从 A 走到 C，跳过 B。
2.  **分帧计算 (Time Slicing)**: 如果一帧内有大量士兵同时寻路，会导致卡顿。建立一个全局的 `PathfindingRequestQueue`，每帧只处理 2-3 个寻路请求。

### 第三阶段：动态避障
1.  **监听建筑销毁**: 当 `Building` 死亡（特别是 Wall），通知 `BuildingManager` 更新网格状态。
2.  **重计算**: 附近的士兵如果原本被阻挡，现在可能需要重新计算路径以利用新缺口。

## 6. 预期效果
*   士兵不再无脑撞墙，而是会寻找城墙的缺口绕行。
*   空军单位依然可以直接飞越障碍。
*   游戏表现更加智能和流畅。
