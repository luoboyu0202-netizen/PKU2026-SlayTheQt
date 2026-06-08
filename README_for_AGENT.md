# SlayTheQt — 项目总览 (Agent 参考)

基于 Qt 6.11 的 Slay the Spire 风格卡牌 Roguelike 游戏，最终整合版本。

---

## 项目结构

```
SlayTheQt.pro              Qt 项目文件
resources.qrc              资源索引
src/
  main.cpp                  入口：TEST_EVENT 宏切换 地图模式 / 沙盒测试
  api/                      API 层
    EventAPI.h              事件数据合同 (EventContext, EventResult)
    BattleAPI.h             战斗数据合同 (BattleContext, BattleResult)
    EventLauncher.h/.cpp    事件启动器：创建沙盒 Player/CardManager/RelicManager
    BattleLauncher.h/.cpp   战斗启动器：生成 BattleView 并绑定引擎
    GameEnums.h             全局枚举 (NodeType, EventType, CardRarity, CardType)
  entities/
    fighter.h/.cpp           战斗者基类 (HP/Block/状态/动画)
    player.h                 玩家：含金币、能量、状态叠加
    enemy.h/.cpp             敌人基类：含意图系统
    statusmanager.h/.cpp     状态效果管理器 (力量/敏捷/虚弱/易伤/缠绕/仪式等)
    cards/                   卡牌定义 (无 Q_OBJECT，头文件 only)
      card.h                 卡牌基类：费用/类型/稀有度/效果逻辑
      DefendCard.h, StrikeCard.h, BashCard.h, ...
      DarkEmbraceCard.h, FireSourceCard.h, ...
      DoubtCard.h (诅咒), RegretCard.h (诅咒)
    enemies/                 敌人定义 (头文件 only)
      SlimeSmall.h, SlimeLarge.h
      JawWorm.h
      FatGremlin.h, MadGremlin.h, ShieldGremlin.h, SneakyGremlin.h, WizardGremlin.h
      GremlinLeader.h
      Cultist.h
      Hexaghost.h (Boss) + HexaFlameItem.h (火焰特效)
    relics/                  遗物定义 (头文件 only)
      relic.h, relicmanager.h
      BurningBloodRelic.h, OrichalcumRelic.h, PenNibRelic.h, SneckoEyeRelic.h
      HappyFlowerRelic.h, IceCreamRelic.h, LanternRelic.h
      MercuryHourglassRelic.h, PrayerWheelRelic.h, ShurikenRelic.h
      SmoothStoneRelic.h, ToriiRelic.h
      MeatOnTheBoneRelic.h, OrnamentalFanRelic.h, TungstenRodRelic.h
  logic/
    GlobalSaveData.h          全局存档：HP/金币/牌组/遗物/事件状态/怪物概率
    CardFactory.h/.cpp        卡牌工厂
    RelicFactory.h/.cpp       遗物工厂
    EnemyFactory.h/.cpp       敌人工厂
    BattleEngine.h/.cpp       战斗引擎核心 (回合/能量/遗物触发/状态结算)
    CardManager.h/.cpp        卡组管理
  map/
    MapManager.h/.cpp          大地图生成 (15层节点 + 贝塞尔连线)
    TitleMenuView.h/.cpp       开始界面 (新游戏/继续游戏)
  ui/
    GameWindow.h/.cpp          主窗口：QStackedWidget 频道切换 + 黑幕转场
    TopBar.h/.cpp              顶栏 (HP/金币/计牌器) — ⚠️ 队友文件，不直接修改
    carditem.h/.cpp            卡牌 UI (渲染/悬停/购买/删牌)
    battleview.h/.cpp          战斗界面
    carddraftoverlay.h/.cpp    选牌界面
    cardbrowseroverlay.h/.cpp  牌库浏览
    enemyitem.h/.cpp           敌人 UI (意图/状态图标)
    playeritem.h/.cpp          玩家 UI (状态/动画)
    RelicItem.h/.cpp           遗物图标
    RelicTray.h/.cpp           遗物托盘 (顶栏下方)
    pileitem.h/.cpp            牌堆图标
    handlayoutmanager.h/.cpp   手牌布局
    statusiconitem.h/.cpp      状态图标
    rewardscreen.h/.cpp        战利品界面 (遗物/金币流星动画)
    topbar.h/.cpp              顶栏数据绑定
    events/                    事件视图
      EventBaseView.h/.cpp      事件基类 → QGraphicsView, 1920x1080 场景
      LeaveButton.h/.cpp        右下角离开按钮 (GO_ahead 图标)
      IconButton.h/.cpp         图标按钮
      TextButton.h/.cpp         文字按钮
      EventOptionButton.h/.cpp  选项按钮
      CampfireView.h/.cpp       火堆 (休息/升级)
      ChestView.h/.cpp          宝箱 (遗物获取)
      MerchantView.h/.cpp       商人 (7卡 + 3遗物 + 删牌服务)
      BigFishView.h/.cpp        大鱼事件
      ClericView.h/.cpp         牧师事件
      DesignerView.h/.cpp       设计师事件
      GoldenWingView.h/.cpp     金神像事件
      SelfNoteView.h/.cpp       留给自己的讯息
      WorldOfGoopView.h/.cpp    黏液世界
      SsssserpentView.h/.cpp    蛇影事件
```

---

## 游戏系统一览

### 地图
- 15 层节点，每层随机 2-5 个节点
- 节点类型：Monster / Elite / Shop / Campfire / Chest / Event / Boss
- 奇偶层交错偏移，贝塞尔曲线连线
- 已访问节点标记 ❌ 且不可重新点击

### 战斗
- 回合制：抽牌 → 出牌 → 敌方行动 → 状态结算
- 能量系统 + 格挡系统
- 状态效果：力量、敏捷、虚弱、易伤、缠绕、仪式、愤怒
- 意图系统：攻击/防御/增益/减益/诅咒/召唤/逃跑

### 敌人 (9 种 + 1 Boss)
Small Slime / Large Slime / Jaw Worm / Fat Gremlin / Mad Gremlin /
Shield Gremlin / Sneaky Gremlin / Wizard Gremlin / Gremlin Leader / Cultist /
**Hexaghost (Boss)** — 含召唤 HexaFlame 机制

### 卡牌 (23 张)
| 类型 | 卡牌 |
|------|------|
| 攻击 | Strike, Bash, PommelStrike, Pummel, Thunderclap, Reaper, Bloodletting, Burn |
| 技能 | Defend, ShrugItOff, SecondWind, Inflame, Metallicize, DarkEmbrace, FireSource, BurningPact, Pour, Barricade |
| 诅咒 | Doubt (虚弱), Regret (扣血) |
| 状态 | Dazed, Wound, Slimed |

### 遗物 (14 件)
BurningBlood / Orichalcum / PenNib / SneckoEye / HappyFlower /
IceCream / Lantern / MercuryHourglass / PrayerWheel / Shuriken /
SmoothStone / Torii / MeatOnTheBone / OrnamentalFan / TungstenRod

### 事件
| 事件 | 选择 |
|------|------|
| 火堆 | 休息(回血) / 升级(选一张牌升级) |
| 宝箱 | 获得随机遗物 (可选拒绝) |
| 商人 | 购买 7 卡 + 3 遗物 + 删牌服务 |
| 大鱼 | 香蕉(回血) / 甜甜圈(提升MaxHP) / 盒子(遗物+悔恨) |
| 牧师 | 治疗 / 净化(删牌) |
| 设计师 | 升级×2 / 移除×1 / 升级+移除 / 一拳(扣血) |
| 金神像 | 祈祷(扣血删牌) / 摧毁(金币) |
| 留讯 | 强制存牌/取牌 |
| 蛇影 | 金币+疑虑 / 拒绝 |
| 黏液世界 | 收集(金币-扣血) / 放手(随机丢失金币) |

---

## 构建与运行

- **编译器**: MinGW 64-bit (Qt 6.11 自带)
- **构建命令**: 用 Qt Creator 打开 `SlayTheQt.pro` → 构建/运行
- **测试开关**: `src/main.cpp` 顶部 `TEST_EVENT` 宏 — `1`=沙盒测试事件, `0`=大地图
- **测试事件选择**: `TEST_WHICH` 1=Campfire, 2=Chest, 3=Merchant, 4=GoldenWing, 5=BigFish, 6=WorldOfGoop, 7=Ssssserpent, 8=SelfNote

---

## 开发注意事项

- **事件范式**: 沙盒模式 — EventLauncher 创建临时 Player/CardManager，事件结束后 EventResult 回写 GlobalSaveData
- **静默结算**: 问号事件获得的卡牌不播放流星动画，计牌器直接更新
- **TopBar**: 队友文件，测试模式下用 `setScale(1.2)` 适配 1920 场景，不对原文件做侵入修改
- **卡牌/遗物/敌人**: 头文件 only 定义，无 `Q_OBJECT` 宏以防链接错误
- **Merchant**: 购买卡牌/遗物保留流星动画 (紫色/蓝色轨迹飞向顶部)
- **存档系统**: `savegame.json` 落盘存档，保存 HP/金币/牌组/遗物/地图进度。新游戏自动清空旧存档
- **地图持久化**: 存档时序列化全部节点（id/类型/坐标/连线/已访问状态），读档后精确重建

---

## 最近更新 (2026-06-08)

- **全屏支持**: `setFixedSize` 改为 `resize` + `setMinimumSize`，`resizeEvent` 同步黑幕和悬浮层尺寸
- **地图存档修复**: 修复"保存并退出后关卡重新生成"问题 — 地图节点数据完整序列化到 `savegame.json`
- **SaveGameView**: 顶栏新增金币和计牌器图标显示 + 退出按钮样式优化
- **CampfireView 数值微调**: 升级/休息按钮位置和特效修正
- **RewardScreen**: 战利品面板自适应窗口尺寸

---

## 已知限制

- 无多存档槽位（仅单存档文件 `savegame.json`）
- 窗口最小尺寸 1280×720，低于此分辨率 UI 可能溢出
