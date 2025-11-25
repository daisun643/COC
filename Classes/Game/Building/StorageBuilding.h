#ifndef __STORAGE_BUILDING_H__
#define __STORAGE_BUILDING_H__

#include "Building.h"
#include "ResourceBuilding.h"  // 需要 ResourceType 枚举

/**
 * 储存建筑类
 * 用于储存资源
 */
class StorageBuilding : public Building {
public:
    static StorageBuilding* create(ResourceType resourceType, int level = 1);
    
    virtual bool init(ResourceType resourceType, int level);
    
    // 储存属性
    CC_SYNTHESIZE(ResourceType, _resourceType, ResourceType);
    CC_SYNTHESIZE(int, _capacity, Capacity);  // 储存容量
    CC_SYNTHESIZE(int, _storedAmount, StoredAmount);  // 当前储存量
    
    /**
     * 添加资源
     * @param amount 资源数量
     * @return 实际添加的数量（可能因为容量限制而小于请求数量）
     */
    int addResource(int amount);
    
    /**
     * 移除资源
     * @param amount 资源数量
     * @return 实际移除的数量
     */
    int removeResource(int amount);
    
    /**
     * 获取可用容量
     */
    int getAvailableCapacity() const;
    
    /**
     * 获取储存建筑信息
     */
    virtual std::string getBuildingInfo() const override;
    
protected:
    StorageBuilding();
    virtual ~StorageBuilding();
    
    /**
     * 根据资源类型初始化属性
     */
    virtual void initStorageProperties();
    
    /**
     * 获取资源名称
     */
    std::string getResourceName() const;
};

#endif // __STORAGE_BUILDING_H__

