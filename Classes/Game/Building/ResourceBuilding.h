#ifndef __RESOURCE_BUILDING_H__
#define __RESOURCE_BUILDING_H__

#include "Building.h"

/**
 * 资源类型
 */
enum class ResourceType {
    GOLD,       // 金币
    ELIXIR,     // 圣水
    DARK_ELIXIR // 暗黑重油
};

/**
 * 资源建筑基类
 * 用于生产资源
 */
class ResourceBuilding : public Building {
public:
    static ResourceBuilding* create(ResourceType resourceType, int level = 1);
    
    virtual bool init(ResourceType resourceType, int level);
    
    // 资源属性
    CC_SYNTHESIZE(ResourceType, _resourceType, ResourceType);
    CC_SYNTHESIZE(int, _productionRate, ProductionRate);  // 每小时产量
    CC_SYNTHESIZE(int, _capacity, Capacity);              // 储存容量
    
    /**
     * 开始生产资源
     */
    virtual void startProduction();
    
    /**
     * 停止生产资源
     */
    virtual void stopProduction();
    
    /**
     * 收集资源
     * @return 收集的资源数量
     */
    virtual int collectResource();
    
    /**
     * 获取资源建筑信息
     */
    virtual std::string getBuildingInfo() const override;
    
    /**
     * 更新资源生产（每帧调用）
     */
    virtual void update(float delta) override;
    
protected:
    ResourceBuilding();
    virtual ~ResourceBuilding();
    
    float _accumulatedTime;  // 累积时间
    int _storedResource;     // 当前储存的资源
    
    /**
     * 根据资源类型初始化属性
     */
    virtual void initResourceProperties();
    
    /**
     * 获取资源名称
     */
    std::string getResourceName() const;
};

#endif // __RESOURCE_BUILDING_H__

