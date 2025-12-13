#ifndef __GRID_UTILS_H__
#define __GRID_UTILS_H__

#include <float.h>

#include "cocos2d.h"

USING_NS_CC;

/**
 * 网格工具类
 * 提供网格坐标和屏幕坐标之间的转换功能
 * p00作为参数传入，deltaX和deltaY从ConfigManager的ConstantConfig获取
 */
class GridUtils {
 public:
  /**
   * 将网格坐标转换为屏幕坐标（grass顶点位置）
   * @param row 网格行坐标
   * @param col 网格列坐标
   * @param p00 地图原点p[0][0]的位置
   * @return 屏幕坐标
   */
  static Vec2 gridToScene(float row, float col, const Vec2& p00);

  /**
   * 将屏幕坐标转换为网格坐标(row, col)
   * @param screenPos 屏幕坐标
   * @param p00 地图原点p[0][0]的位置
   * @param row 输出的网格行坐标
   * @param col 输出的网格列坐标
   * @return 是否转换成功（坐标在有效范围内）
   */
  static bool screenToGrid(const Vec2& screenPos, const Vec2& p00, float& row,
                           float& col);

  /**
   * 找到最近的grass顶点
   * @param screenPos 屏幕坐标
   * @param p00 地图原点p[0][0]的位置
   * @param row 输出的网格行坐标
   * @param col 输出的网格列坐标
   * @param nearestPos 输出的最近顶点屏幕坐标
   * @return 是否找到有效顶点
   */
  static bool findNearestGrassVertex(const Vec2& screenPos, const Vec2& p00,
                                     float& row, float& col, Vec2& nearestPos);
};

#endif  // __GRID_UTILS_H__
