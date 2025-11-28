#ifndef __CONSTANT_MANAGER_H__
#define __CONSTANT_MANAGER_H__

#include "cocos2d.h"
#include "Config/ConfigManager.h"

USING_NS_CC;

/**
 * 常量管理器
 * 管理游戏中的常量值，如地图原点p00等
 */
class ConstantManager {
public:
    /**
     * 获取单例实例
     */
    static ConstantManager* getInstance();
    
    /**
     * 初始化常量管理器
     * 在设置分辨率后调用，计算地图原点p00
     */
    bool init();
    
    /**
     * 获取地图原点p[0][0]的位置
     */
    const Vec2& getP00() const { return _p00; }
    
    /**
     * 重新计算p00（当分辨率改变时调用）
     */
    void recalculateP00();

private:
    ConstantManager();
    ~ConstantManager();
    
    /**
     * 计算地图原点p00
     */
    void calculateP00();
    
    Vec2 _p00;           // 地图原点p[0][0]
    static ConstantManager* _instance;
};

#endif // __CONSTANT_MANAGER_H__

