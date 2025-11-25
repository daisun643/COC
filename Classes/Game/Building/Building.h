#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * 建筑类型枚举
 */
enum class BuildingType {
    TOWN_HALL,          // 大本营
    DEFENSE,            // 防御建筑
    RESOURCE,           // 资源建筑
    STORAGE,            // 储存建筑
    BARRACKS            // 兵营
};

/**
 * 建筑基础类
 * 所有建筑的基类，提供通用的建筑功能
 */
class Building : public Sprite {
public:
    /**
     * 创建建筑
     * @param imagePath 建筑图片路径
     * @param type 建筑类型
     * @param level 建筑等级（默认为1）
     */
    static Building* create(const std::string& imagePath, BuildingType type, int level = 1);
    
    /**
     * 初始化建筑
     */
    virtual bool init(const std::string& imagePath, BuildingType type, int level);
    
    // 建筑属性
    CC_SYNTHESIZE(BuildingType, _buildingType, BuildingType);
    CC_SYNTHESIZE(int, _level, Level);
    CC_SYNTHESIZE(int, _maxLevel, MaxLevel);
    
    // 建筑名称（避免与 Node::getName() 冲突）
    inline void setBuildingName(const std::string& name) { _buildingName = name; }
    inline const std::string& getBuildingName() const { return _buildingName; }
    
    /**
     * 升级建筑
     * @return 是否升级成功
     */
    virtual bool upgrade();
    
    /**
     * 获取升级所需资源
     * @return 升级所需资源（可以根据等级计算）
     */
    virtual int getUpgradeCost() const;
    
    /**
     * 获取建筑信息
     */
    virtual std::string getBuildingInfo() const;
    
    /**
     * 显示建筑信息标签
     */
    virtual void showInfo();
    
    /**
     * 隐藏建筑信息标签
     */
    virtual void hideInfo();
    
protected:
    Building();
    virtual ~Building();
    
    Label* _infoLabel;  // 信息显示标签
    std::string _buildingName;  // 建筑名称
    
    /**
     * 创建默认建筑外观（如果图片不存在）
     */
    virtual void createDefaultAppearance();
};

#endif // __BUILDING_H__

