#ifndef __GRID_UTILS_H__
#define __GRID_UTILS_H__

#include "cocos2d.h"
#include <float.h>

USING_NS_CC;

/**
 * 网格工具类
 * 提供网格坐标和屏幕坐标之间的转换功能
 * p00从ConstantManager获取，deltaX和deltaY从ConfigManager的ConstantConfig获取
 */
class GridUtils {
public:
  /**
   * 将网格坐标转换为屏幕坐标（grass顶点位置）
   * @param row 网格行坐标
   * @param col 网格列坐标
   * @return 屏幕坐标
   */
  static Vec2 gridToScreen(int row, int col);
  
  /**
   * 将屏幕坐标转换为网格坐标(row, col)
   * @param screenPos 屏幕坐标
   * @param row 输出的网格行坐标
   * @param col 输出的网格列坐标
   * @return 是否转换成功（坐标在有效范围内）
   */
  static bool screenToGrid(const Vec2 &screenPos, int &row, int &col);
  
  /**
   * 找到最近的grass顶点
   * @param screenPos 屏幕坐标
   * @param row 输出的网格行坐标
   * @param col 输出的网格列坐标
   * @param nearestPos 输出的最近顶点屏幕坐标
   * @return 是否找到有效顶点
   */
  static bool findNearestGrassVertex(const Vec2 &screenPos, int &row, int &col, Vec2 &nearestPos);
};

#endif // __GRID_UTILS_H__

