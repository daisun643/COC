#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include "cocos2d.h"
#include "json/document.h"
#include <string>

USING_NS_CC;

/**
 * 配置管理器
 * 负责读取和解析JSON配置文件
 */
class ConfigManager {
public:
    static const ConfigManager* getInstance();
    
    /**
     * 初始化配置管理器，加载所有配置文件
     * @return 是否初始化成功
     */
    bool init();
    
    /**
     * 获取背景配置
     */
    struct BackgroundConfig {
        int designWidth;
        int designHeight;
        int smallWidth;
        int smallHeight;
        int mediumWidth;
        int mediumHeight;
        int largeWidth;
        int largeHeight;
    };
    
    /**
     * 获取常量配置
     */
    struct ConstantConfig {
        int gridSize;                   // 网格边长
        int gridResolutionWidth;        // 网格分辨率宽度
        int gridResolutionHeight;       // 网格分辨率高度
        std::string grassImagePath;     // 草地图片路径
        float grassWidth;               // 草地图片宽度
        float grassHeight;              // 草地图片高度
        float deltaX;                   // 网格间距X
        float deltaY;                   // 网格间距Y
        float glowDelay;                // 光晕明暗交替间隔
    };
    
    /**
     * 获取建筑配置
     */
    struct BuildingConfig {
        std::string image;
        int gridPositionX;   // 建筑在网格中的位置X
        int gridPositionY;   // 建筑在网格中的位置Y
        float anchorRatioX;  // 锚点X比例
        float anchorRatioY;  // 锚点Y比例
        int gridSize;        // 建筑占用的网格大小（菱形边长）
        int defaultLevel;
        int maxLevel;
        float imageScale;
    };
    
    /**
     * 获取背景配置
     */
    BackgroundConfig getBackgroundConfig() const { return _backgroundConfig; }
    
    /**
     * 获取常量配置
     */
    ConstantConfig getConstantConfig() const { return _constantConfig; }
    
    /**
     * 获取TownHall配置
     */
    BuildingConfig getTownHallConfig() const { return _townHallConfig; }
    
private:
    ConfigManager();
    ~ConfigManager();
    
    /**
     * 加载背景配置文件
     */
    bool loadBackgroundConfig();
    
    /**
     * 加载常量配置文件
     */
    bool loadConstantConfig();
    
    /**
     * 加载建筑配置文件
     */
    bool loadBuildingConfig();
    
    /**
     * 从文件读取JSON文档
     */
    bool loadJsonFromFile(const std::string& filePath, rapidjson::Document& doc);
    
    BackgroundConfig _backgroundConfig;
    ConstantConfig _constantConfig;
    BuildingConfig _townHallConfig;
    
    static ConfigManager* _instance;
};

#endif // __CONFIG_MANAGER_H__

