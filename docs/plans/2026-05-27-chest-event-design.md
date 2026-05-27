# 宝箱事件 (Chest) 设计文档

## 概述

在地牢探索中，玩家进入宝箱房间。右侧放置着一座发光的魔法水晶宝箱，点击后随机产出一件遗物，玩家可选择拾取或跳过。

## 视觉参考

- **视角**：战斗视角（非越肩），玩家左侧，宝箱右侧
- **宝箱素材**：`resources/images/events/Chest/Chest.png` — 半透明青蓝色水晶宝箱，自带星芒闪光
- **背景素材**：`resources/images/events/Chest/Background.png` — 地下废墟场景，火把暖光，紫色雾气
- **理想效果**：`debug/ideal_effect/chest_sample.jpg` — 杀戮尖塔原版宝箱事件截图

## 架构

```
ChestView : EventBaseView
├── m_chestImage (QGraphicsPixmapItem)     — 宝箱图片，右侧
├── m_playerImage (继承)                    — 玩家立绘，左侧
├── m_sparkleEffects                        — 点击前星芒闪烁粒子
├── m_relicPopup  (RelicPopupWidget)        — 遗物弹窗
│   ├── 遗物图标 + 名称 + 效果描述
│   └── Hover → Tooltip 显示遗物详情
├── m_takeBtn / m_skipBtn (TextButton)      — 拾取/跳过
└── m_leaveBtn (继承 LeaveButton)           — "前进"按钮
```

## 交互流程

```
场景加载 → 宝箱带闪光特效
    ↓
点击宝箱 → 播放开箱动画(组员提供素材)
    ↓
遗物弹窗弹出  ←  RelicFactory::generateRandomRelic()
    ├─ 鼠标悬停遗物 → 显示遗物基本信息 Tooltip
    ├─ 点击"拾取" → 遗物加入 RelicManager → 宝箱变为开启无闪光
    └─ 点击"跳过" → 遗物丢弃 → 宝箱变为开启无闪光
    ↓
显示"前进"按钮 (LeaveButton → GO_ahead.png)
    ↓
点击前进 → emit eventFinished() → EventLauncher 结算
```

## 遗物弹窗 UI

- 居中弹出半透明暗底卡片
- 遗物图标左侧，名称和效果描述右侧
- 悬停时显示 Tooltip（含遗物完整描述）
- 底部并排两个按钮："拾取"（确认）、"跳过"（取消）
- 拾取前允许玩家充分查看遗物信息

## 输入/输出合同

复用 `EventAPI.h` 中的 `EventContext` 和 `EventResult`：
- **输入**：`EventContext.eventType = Campfire`（新增 `Chest` 枚举值）
- **输出**：`EventResult.relicsChanged = true`（若拾取），`resultRelics` 包含新增遗物

## 关键文件

| 文件 | 操作 |
|------|------|
| `src/ui/events/ChestView.h` | 新建 |
| `src/ui/events/ChestView.cpp` | 新建 |
| `src/ui/events/RelicPopupWidget.h` | 新建 — 遗物弹窗组件 |
| `src/ui/events/RelicPopupWidget.cpp` | 新建 |
| `src/api/EventAPI.h` | 修改 — 事件类型中 Chest 已存在，确认可用 |
| `src/main.cpp` | 修改 — 测试入口切换为 Chest |
| `SlayTheQt.pro` | 修改 — 添加新源文件 |

## 依赖

- `EventBaseView` — 基类（TopBar、RelicTray、玩家像、前进引导）
- `RelicFactory::generateRandomRelic()` — 随机遗物生成（已存在）
- `RelicManager` — 遗物管理（添加/移除）
- `Relic::getDescription()` — 悬停 Tooltip 文案来源
