#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include <functional>
#include <vector>

#include "cocos2d.h"

USING_NS_CC;

/**
 * 寻路器工具类
 * 实现 A* 寻路算法
 */
class PathFinder {
 public:
  /**
   * 寻找路径
   * @param startPos 起始屏幕坐标
   * @param endPos 终点屏幕坐标
   * @param p00 地图原点
   * @param isWalkable 回调函数，判断指定网格是否可通行
   * @return 路径点列表（屏幕坐标），如果找不到路径返回空列表
   */
  static std::vector<Vec2> findPath(const Vec2& startPos, const Vec2& endPos,
                                    const Vec2& p00,
                                    std::function<bool(int, int)> isWalkable);
};

#endif  // __PATH_FINDER_H__
