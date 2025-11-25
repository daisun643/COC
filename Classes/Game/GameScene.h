#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Building/TownHall.h"
#include "Building/DefenseBuilding.h"
#include "Building/ResourceBuilding.h"
#include "Building/StorageBuilding.h"
#include "Building/Barracks.h"

USING_NS_CC;

/**
 * 游戏主场景
 * 展示村庄和所有建筑
 */
class GameScene : public Scene {
public:
    static Scene* createScene();
    
    virtual bool init() override;
    
    CREATE_FUNC(GameScene);
    
private:
    /**
     * 初始化大本营
     */
    void initTownHall();
    
    /**
     * 初始化防御建筑
     */
    void initDefenseBuildings();
    
    /**
     * 初始化资源建筑
     */
    void initResourceBuildings();
    
    /**
     * 初始化储存建筑
     */
    void initStorageBuildings();
    
    /**
     * 初始化兵营
     */
    void initBarracks();
    
    /**
     * 添加建筑到场景
     */
    void addBuilding(Building* building, const Vec2& position);
    
    TownHall* _townHall;  // 大本营
};

#endif // __GAME_SCENE_H__

