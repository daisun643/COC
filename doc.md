# 部落攻防（Clash-like）项目设计与实现文档

------

## 1. 项目简介

- **课程项目**：程序设计范式 / Cocos2d-x 游戏开发期末项目 
- **项目名称**：部落攻防（Clash-like）  
- **团队成员**：
  - 组长：但泰然
  - 成员：孙同德
  - 成员：吴承宪
- **Git 仓库地址**：https://github.com/daisun643/COC  
- **提交日期**：2025-12-28  

------

## 2. 项目背景与目标

本项目为一款仿《部落冲突》的策略类游戏原型，客户端采用`C++`和`cocos2d`引擎，后端采用`flask`框架，进行开发，实现主村庄建设与进攻战斗两大核心玩法。玩家通过建设与升级建筑、训练兵种，在不同地图中发起进攻，由 AI 自动完成战斗过程，并根据战果获得资源与评价。并用面向对象设计、数据驱动、算法与工程化手段体现课程目标：

- 面向对象：建筑/兵种/场景/管理器的职责划分、继承与多态、可扩展的实体体系  
- 算法能力：A* 寻路、索敌优先级、破墙策略等 AI 决策逻辑  
- 工程质量：Git 协作与提交规范、异常处理、单元测试、减少内存泄漏、可维护目录结构  
- 体验完整：基础 UI、合理难度、背景音乐与音效、可运行可答辩  

---

## 3. 需求覆盖说明

### 3.1 基础功能（全部实现）

* [x] 大本营系统
* [x] 三种资源：金币、圣水和人口
* [x] 建筑系统：资源生成建筑、军营、防御塔和存储建筑
* [x] 兵种系统：野蛮人、弓箭手、巨人、炸弹人和飞龙
* [x] AI 自动战斗：路径寻找、攻击判定、建筑优先级和建筑等级系统
* [x] 单局战斗流程：投放兵种、兵种自动战斗和胜负判定
* [x] 多张地图：地图编辑器支持设计并保存地图
* [x] 背景音乐与战斗音效

### 3.2 可选功能（全部实现）

* [x] 多人进攻：两次进攻机会，前人进攻结果保留
* [x] 联盟系统：成员加入、离开部落，首领创建、解散部落
* [x] 空中兵种：飞龙兵种无视城墙
* [x] 陷阱系统：炸弹陷阱，敌人靠近触发
* [x] 战斗回放：回放进攻过程
* [x] 建筑升级加速：加速建筑的升级过程

### 3.3 自行扩展

* [x] 后端云服务：`flask`框架，服务联机需求

### 3.4 加分项

#### 3.4.1 版本控制与协作

* [x] GitHub 使用规范

- 多`branch`同步推进解耦需求
- `master`采用`squash\space merge`，确保`commit`记录清晰
- 使用`code\space diff`进行`code\space review`
- 使用`issues`汇总开发时遇到的`Bug`并解决
- `gitignore`避免上传`build`中间文件
- `GitHub` [仓库](https://github.com/daisun643/COC)

* [x] 合理分工

- 需求逻辑解耦
- 共同开发项目基座 
- 根据兴趣特长分配需求
- 一次分配一周工作量，一周一迭代

* [x] Commit 记录清晰

- `message`记录`commit`中功能添加、`Bug`修复等等

#### 3.4.2 代码质量

* [x] 单元测试

- 工具函数
  使用`Google\space Test`框架为工具函数编写单元测试，参见`Classes\test`。
- 图形界面
  编译后手动点击测试

* [x] 合理异常处理
  当事件与预期不符合，通过`CCLOG`写入输出，再通过`VS`查看输出锁定异常。

```cpp
try {
  outFile << buffer.GetString();
  outFile.close();
  CCLOG("BuildingManager: Map saved to %s", path.c_str());
} catch (const std::exception& e) {
  CCLOG("BuildingManager: Error saving map to %s: %s", path.c_str(),
        e.what());
}
```

* [x] 无内存泄漏

- 单实例模式，避免反复`new`和`delete`

  ```cpp
  Profile* profile =  Profile::getInstance();
  ```

- 析构安全
  `BasicSence`作为`Scene`的基类，其维护不少配置类，为了防止子类错误析构，进而导致重复析构，类似下面在基类中采用`if`判断避免重复析构。

  ```cpp
  BasicSence::~BasicSence(){
    ...
    if(_buildingManager){
      delete _buildingManager;
    }
    ...
  }
  ```

#### 3.4.3 开发特性

* [x] C++11/14/17 特性使用丰富
  auto、lambda、function等等参见3.1
* [x] 优雅的架构设计

- 类的继承和多态运用
  参见 3.1
- 资源管理逻辑解耦
  `Classes/Manager`处理资源加载更新保存逻辑，与`Classes/Game`下游戏逻辑解耦。
- 规避配置硬编码
  配置`Resource`下`json`文件，通过`Classes/Manager`下`Manager`加载更新保存参数配置。

* [x] 目录结构清晰

  ```
  COC/
  ├── CMakeLists.txt        # CMake 主配置文件
  ├── build.bat             # Windows 构建脚本
  ├── .gitignore            # Git 忽略文件配置
  ├── README.md             # 项目说明文档
  ├── Classes/              # 源代码目录
  │   ├── Container/        # 场景容器
  │   │   ├── Layer/        # Layer容器
  │   │   ├── Node/         # Node容器
  │   │   └── Scene/        # Scene容器
  │   ├── Game/             # 游戏逻辑
  │   │   ├── Building/     # 建筑逻辑
  │   │   ├── Soldier/      # 士兵逻辑
  │   │   └── Spell/        # 法术逻辑
  │   ├── Manager/          # 运行时资源管理
  │   │   ├── Building/     # 建筑管理
  │   │   ├── Config/       # 参数管理
  │   │   └── Record/       # 回放管理
  │   ├── Utils/            # 工具函数
  │   │   ├── API/          # 网络请求
  │   │   └── Profile/      # 信息维护
  │   ├── AppDelegate.cpp   # 应用程序委托类
  │   └── main.cpp          # 程序入口
  │── Resources/            # 资源文件目录
  │   ├── config/           # 字体文件
  │   ├── develop/          # 字体文件
  │   ├── fonts/            # 字体文件
  │   ├── images/           # 图片资源
  │   ├── level/            # 字体文件
  │   ├── profile/          # 字体文件
  │   └── record/           # 音频资源
  ├── test/                 # 单元测试
  └── server/               # 服务器逻辑
  
  ```

#### 3.4.4 界面与体验

* [x] 界面精美
  使用网络素材包美化界面。
  ![](./pic/p1.png)

### 3.5 超级加分项

* [x] 成功运行于`Andriod`
  通过`Andriod Studio`进行代码和资源的打包，apk位于`release`中。
  ![运行示例](./pic/p2.jpg)

---

##  4.项目要求覆盖说明

### 4.1 必须符合的要求(C++ 特性使用)

* [x] STL 容器:`vector、map 、set、pair、pair`
* [x] 迭代器
  这里采用逆向迭代器，遍历建筑。

```cpp
for (auto it = _buildings.rbegin(); it != _buildings.rend(); ++it) {
  ...
}
```

* [x] 类与多态

```mermaid
classDiagram
    class Building {
        int _level
        float _maxHP
        float _currentHP
        upgrade()
        completeUpgrade()
        updateHPBar()
    }
    class Wall {
        int _defense
    }
    class StorageBuilding {
        int _capacity
        string _resourceType
    }
    class TrapBuilding {
        float _triggerRange
        int _damage
        bool _isArmed
        checkTrigger()
    }
    class ResourceBuilding {
        int _productionRate
        float _storedResource
        collect()
        update()
    }

    class BasicSoldier {
        float _attackRange
        AttackType _attackType
        isAlive()
        getCurrentHP()
        setMaxHP()
    }
    class Dragon {
    }
    class Barbarian {
    }

    Building <|-- Wall
    Building <|-- StorageBuilding
    Building <|-- TownHall
    Building <|-- TrapBuilding
    Building <|-- ResourceBuilding
    Building <|-- PlaceholderBuilding

    BasicSoldier <|-- Dragon
    BasicSoldier <|-- Barbarian
```

* [x] 异常处理
  当事件与预期不符合，通过`CCLOG`写入输出，再通过`VS`查看输出锁定异常。

```cpp
try {
  outFile << buffer.GetString();
  outFile.close();
  CCLOG("BuildingManager: Map saved to %s", path.c_str());
} catch (const std::exception& e) {
  CCLOG("BuildingManager: Error saving map to %s: %s", path.c_str(),
        e.what());
}
```

* [x] 函数重载&操作符重载
  重载`-=`运算符，实现对建筑生命值的减少。

```cpp
Building& Building::operator-=(float damage) {
  ...
  _currentHP -= damage;
  if (_currentHP < 0) {
    _currentHP = 0;
  }
  ...
  return *this;
}
```

* [x] C++11 或以上功能

- lambda&auto

```cpp
auto loadFromPath = [this](const std::string& path) -> bool {
    ...
    return false;
  };
```

- nullptr

```cpp
struct PathNode {
  ...
  PathNode(...) : ..., parent(nullptr) {}
  ...
};
```

- range-for

```cpp
for (PathNode* node : allNodes) {
    delete node;
  }
```

- unordered_set

```cpp
  std::unordered_set<long long> validEndPoints;
  if (...) {
    validEndPoints.insert(coordToLong(eRow, eCol));
  } else {...}
```

- function

```cpp
static std::vector<Vec2> findPath(
      ...,const std::function<bool(int, int)>& isWalkable, ...);
```

### 4.2 项目必达标准

* [x] 代码格式统一

- `Google formater`统一代码风格

* [x] Google C++ Style

- 类名采用`PascalCase`命名法
- 成员变量采用下划线+`camelCase`命名法
- 函数名采用`camelCase`命名法

* [x] C++ 风格类型转换

- static_cast

```cpp
EventMouse* mouseEvent = static_cast<EventMouse*>(event);
```

- dynamic_cast

```cpp
DefenseBuilding* defenseBuilding = dynamic_cast<DefenseBuilding*>(building);
if (!defenseBuilding) {
  continue;
}
```

* [x] 合理使用 const

- 成员函数

```cpp
bool getIsLogin() const;
int getId() const; 
const std::string& getName() const;
```

- 参数传递

```cpp
void setName(const std::string& name); 
static void leaveClan(const std::string& clan_id, int user_id, JoinClanCallback callback);
```

* [x] 注释规范,设计说明明确

- 服务器逻辑中

```
"""
    获取随机对手接口
    GET 参数: user_id (用户ID)
    返回: JSON 格式 {
        "success": bool,
        "message": str,
        "opponent_id": int or None,
        "opponent_name": str or None,
        "map_data": dict or None
    }
"""
```

### 4.3 核心技术实现细节

#### 4.3.1 面向对象设计范式的实践

针对游戏中种类繁多的建筑与兵种，我们利用 C++ 的继承、多态与封装特性，构建了高内聚、低耦合的实体架构。

**1. 继承体系与抽象**

通过提取共性构建清晰的类层次，最大化代码复用。

- **建筑抽象体系**：
  - **基类封装**：`Building` 类作为所有建筑的根基，封装了核心属性（如 `_level` 等级、`_currentHP` 生命值、`_row/_col` 网格坐标）与通用行为接口（如 `takeDamage()` 受击、`isAlive()` 存活判定）。
  - **派生类特化**：
    - **`DefenseBuilding` (防御塔)**：继承自 `Building`，扩展了 **攻击行为**。新增 `_attackRange` (射程)、`_damage` (伤害) 属性，并实现了 `attackSoldiers()` 索敌逻辑。
    - **`ResourceBuilding` (资源设施)**：继承自 `Building`，扩展了 **生产行为**。新增 `_productionRate` 属性与 `harvest()` 接口，利用 `update()` 钩子实现资源的定时产出。

**2. 多态与运行时行为**

利用虚函数机制实现“同一接口，不同行为”，提升系统的可扩展性。

- 差异化的升级逻辑：

  基类定义了 virtual void upgrade()。

  ```c++
  void DefenseBuilding::upgrade() {
    Building::upgrade(); // 复用基类逻辑（等级+1，纹理刷新）
    // 扩展派生类特有逻辑：数据驱动更新攻击属性
    auto config = ConfigManager::getInstance()->getBuildingConfig(_buildingName, _level);
    this->_damage = config.damage;
  }
  ```

  这种设计符合 **开闭原则 (OCP)**：若未来新增“陷阱”类建筑，只需继承基类并重写 `upgrade`，无需修改核心升级系统的代码。

**3. 设计模式的应用**

- 简单工厂模式：

  为解耦对象创建与使用，兵种生成模块采用工厂方法。UI 层只传入 SoldierType 枚举，工厂负责返回对应派生类实例，符合 依赖倒置原则：

  ```c++
  BasicSoldier* BasicSoldier::create(SoldierType soldierType, int level) {
    switch (soldierType) {
      case SoldierType::BARBARIAN: return Barbarian::create(level);
      case SoldierType::ARCHER:    return Archer::create(level);
      // ... 新增兵种只需在此添加 case，无需修改外部调用逻辑
    }
    return nullptr;
  }
  ```

- 单例模式：

  对于全局唯一的配置管理器 ConfigManager 和音频管理器 AudioManager，我们实现了线程安全的单例模式。通过私有化构造函数并提供静态访问点 getInstance()，确保了全局资源的一致性访问，避免了重复加载配置文件的 IO 开销。

- 有限状态机 (FSM)：

  在 BasicSoldier 的 AI 设计中，我们使用状态模式管理兵种行为。定义 SoldierState 枚举 (IDLE, MOVING, ATTACKING, DEAD)，在 update() 中根据状态切换行为逻辑，保证了复杂战斗逻辑下的状态流转清晰可控。

#### 4.3.2 智能算法与 AI 决策

**1. 高精度 A 寻路算法优化**

为解决士兵寻路问题，PathFinder 类实现了一套优化的 A* 算法：

- **子网格精度细分**：引入 `precision`（默认 4），将一个逻辑网格细分为 16 个 (4x4) 寻路子网格。计算时使用高精度坐标，输出路径时通过 `(curr->row + 0.5f) / precision` 将坐标修正到子网格中心，实现了平滑的移动轨迹。
- **数据结构优化**：使用 `std::priority_queue` 管理开放列表，确保每次以 $O(1)$ 时间复杂度取出 F 值最小的节点。
- **启发式函数优化**：使用曼哈顿距离 (`std::abs(dx) + std::abs(dy)`) 作为启发函数 $H$，在网格地图中相比欧几里得距离计算开销更低且更符合移动规则。
- **策略模式应用**：`findPath` 接口接收 `std::function<bool(int,int)> isWalkable` 回调。普通兵种传入常规碰撞检测，而**炸弹人**传入特殊检测函数（将“墙体”视作可通行目标），从而复用了核心算法实现了完全不同的 AI 行为。

**2. 多级 AI 索敌决策树**

为了还原真实的战斗策略，我们在 findTarget 中实现了一套三级优先级决策系统：

1. **首选目标**：根据兵种特性（如巨人优先攻击防御塔）筛选特定类型建筑。
2. **次选兜底**：若首选目标不存在，自动降级搜索最近的任意非墙建筑。
3. **破墙机制**：当 A 寻路返回空路径（被围墙完全阻挡）时，AI 智能识别并锁定最近的城墙作为临时攻击目标。

#### 4.3.3 全异步网络通信与数据驱动架构

**1. 异步非阻塞通信**

为了避免网络交互阻塞主线程导致游戏卡顿，客户端采用了全异步设计：

- **Lambda 与闭包**：使用 C++11 Lambda 表达式封装 `HttpRequest` 回调，通过值捕获 `[callback]` 传递上下文，避免了传统函数指针导致的上下文丢失问题。

  ```c++
  request->setResponseCallback([callback](HttpClient* client, HttpResponse* response) {
    // 在回调中解析数据并通过 callback 回传，不阻塞主线程
    if (callback) callback(success, message, ...);
  });
  ```

**2. 数据驱动设计**

游戏的核心数值（如兵种血量、建筑造价）并未硬编码在 C++ 代码中，而是完全分离至 JSON 配置文件。ConfigManager 利用 rapidjson 在运行时加载 config/building.json 等文件。这种设计使得数值策划可以在不重新编译程序的情况下调整游戏平衡性，极大地提高了开发效率。同时，在解析时通过 IsObject、IsString 等严格类型检查，提高了面对异常网络数据时的健壮性。

------

## 5. 运行环境与快速开始

### 5.1 运行环境

- **客户端**：Windows 10/11（Visual Studio + CMake），或 Android（Android Studio）  
- **引擎**：Cocos2d-x 3.x
- **语言标准**：C++11 或以上  
- **后端**：Python 3.8+ Flask  

### 5.2 客户端（Windows）快速启动

1. 安装依赖  
   - Visual Studio 2022  
   - CMake（建议 3.2x+）  
   - Git（可选）  

2. 编译  
   - 双击运行 `build.bat`（如仓库提供）  
   - 或手动：
     - `mkdir build && cd build`
     - `cmake ..`
     - `cmake --build . --config Release`

3. 运行  
   - 在生成目录中找到可执行文件并运行（例如 `Release/COC.exe`）

### 5.3 客户端（Android）快速启动

- Android Studio 打开工程：`proj.android `
- 构建方式：Build APK / Gradle Task：`assembleRelease`
- APK 路径：`release/app-release.apk`   

### 5.4 服务器（Flask）快速启动

1. 进入服务器目录：`cd server`  
2. 安装依赖：`pip install -r requirements.txt`  
3. 启动服务：`python app.py`（或 `flask run --host 0.0.0.0 --port 80`）  
4. 配置客户端连接（如需要）：  
   - 服务器地址：`http://127.0.0.1:80`  
   - 端口：80

------

## 6. 玩家使用说明 

### 6.1 界面清单

- 登录界面 
  - 开始菜单：进入游戏 / 设置 / 退出  
- 主村庄界面  
  - 资源显示：金币/圣水/人口  
  - 建筑列表：大本营、资源建筑、军营、防御塔、存储、墙体/装饰等  
- 商店/建筑放置与升级界面  
  - 放置提示、无效放置提示、升级按钮与耗时/加速  
- 进攻准备界面（选择地图/对手、布置兵种）  
  - 兵种选择与投放  
- 战斗界面（AI 自动战斗）  
  - 战斗音效、特效、血条/攻击范围表现  
- 结算界面  
  - 胜负判定、星级/摧毁率、资源奖励  
- 回放界面  
  - 回放列表、播放控制、快进/暂停（如实现）  
- 联盟/部落界面 
  - 加入/离开/创建/解散部落  

### 6.2 核心玩法流程

1. 进入主村庄：查看资源与人口  
2. 建设或升级建筑：提升产出、防御与容量  
3. 军营训练兵种：消耗资源与人口容量  
4. 进入进攻：选择地图/对手，投放兵种  
5. AI 自动战斗：寻路 + 索敌 + 攻击  
6. 战斗结算：资源奖励与评价  
7. 可查看回放：复盘进攻过程  
8. （可选）加入联盟：多人互动与进攻机会保留  

### 6.3 难度设计说明

- **地图难度差异**：至少两张地图，建筑布局/墙体/陷阱/防御塔分布不同  
- **建筑等级差异**：至少 3 级，防御塔伤害/射程、资源建筑产量等随等级变化  
- **AI 决策差异**：不同兵种（巨人/炸弹人/飞龙）具有不同目标偏好与移动规则  
- **数据驱动可调参**：兵种 HP/伤害/速度、建筑造价/等级成长、陷阱伤害等可在 JSON 中调整  

