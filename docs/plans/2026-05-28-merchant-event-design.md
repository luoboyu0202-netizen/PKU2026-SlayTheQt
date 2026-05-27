# 商店事件 (Merchant) 设计文档

## 概述

在地牢探索中，玩家选择"Shop"节点进入商店。事件分两个阶段：阶段一为主角与商人的相遇场景，点击商人后过渡到阶段二——地毯购买界面。玩家可使用金币购买卡牌和遗物，或付费移除牌组中的一张卡牌。

## 视觉参考

- **阶段一**：战斗视角（主角左侧，商人右侧），复用宝箱背景 `resources/images/events/Chest/Background.png`
- **阶段二**：地毯全幅展开，手指跟随鼠标，卡牌和遗物陈列其上
- **理想效果**：`debug/ideal_effect/merchant_carpet_sample.jpg` — 杀戮尖塔原版商店截图

## 素材清单

| 文件 | 用途 | 状态 |
|------|------|------|
| `Merchant.png` | 商人角色（去背景） | 已有 |
| `merchant.jpg` | 商人角色（带背景参考） | 已有 |
| `carpet.jpg` | 商店地毯 | 已有 |
| `exit.jpg` | "离开"飘带按钮 | 已有 |
| `label.jpg` | 打折标签 | 已有 |
| `remove.jpg` | 卡牌移除入口图标 | 已有 |
| `soldout.jpg` | 移除后"卖光了"占位 | 已有 |
| 手指素材 | 跟随鼠标的手指图标 | 待提供 |
| 宝箱背景 | 阶段一背景（复用 Chest/Background.png） | 已有 |

## 架构

```
MerchantView : EventBaseView
├── 阶段一 (Phase 1: Encounter)
│   ├── m_playerImage (继承)                  — 玩家立绘，左侧（复用父类）
│   ├── m_merchantImage (QGraphicsPixmapItem)  — 商人，右侧
│   └── 点击商人 → onMerchantClicked() → 过渡到阶段二
│
├── 阶段二 (Phase 2: Shopping)
│   ├── m_carpet (QGraphicsPixmapItem)         — 地毯全幅背景
│   ├── m_handCursor (QGraphicsPixmapItem)     — 手指，跟随鼠标
│   ├── m_cardSlots[7]                          — 7个卡牌槽位
│   │   ├── 第一排5张，第二排2张
│   │   ├── 每张显示：费用/插画/名称/效果/价格
│   │   ├── 随机1张附打折标签 (label.jpg)
│   │   └── 点击 → 金币足够则购买，不足则灰显
│   ├── m_relicSlots[3]                         — 3个遗物槽位
│   │   ├── 位于第二排卡牌右侧
│   │   ├── 最右(m_relicSlots[2])必为商店专属遗物
│   │   └── 点击 → 购买确认
│   ├── m_removeButton (QGraphicsPixmapItem)    — 删牌入口(remove.jpg)
│   │   ├── 位于遗物右侧
│   │   ├── 点击 → 弹牌组选择 → 确认删除 → 扣费
│   │   └── 使用后替换为 m_soldoutPlaceholder (soldout.jpg)
│   ├── m_exitBanner (QGraphicsPixmapItem)      — 离开飘带(exit.jpg)
│   │   └── 位于第二排左侧，右半叠加地毯，点击即离开商店
│   └── TopBar (继承)                          — HP/金币实时更新
```

## 交互流程

```
阶段一：场景加载 → 玩家左侧，商人右侧（宝箱背景）
    ↓
点击商人 → 过渡动画 → 阶段二
    ↓
阶段二：地毯展开，卡牌/遗物/删牌/离开飘带依次出现
    ↓
┌────────────────── 购物循环 ──────────────────┐
│ 鼠标移动 → 手指跟随                           │
│ 悬停卡牌 → 高亮 + 显示价格/效果 Tooltip        │
│ 点击卡牌 → 金币够 → 购买，金币够 → 灰显无响应   │
│ 悬停遗物 → 高亮 + 显示描述 Tooltip             │
│ 点击遗物 → 金币够 → 购买                      │
│ 点击删牌 → 弹牌组选择 → 确认删除 → 扣费        │
│          → remove.jpg 变为 soldout.jpg        │
│ 悬停离开飘带 → 高亮                           │
│ 点击离开飘带 → emit eventFinished()            │
└──────────────────────────────────────────────┘
```

## 定价规则

### 卡牌
| 稀有度 | 价格范围 |
|--------|----------|
| 普通 | 45 - 55 金币 (50 ± 10%) |
| 罕见 | 68 - 82 金币 (75 ± 10%) |
| 稀有 | 135 - 165 金币 (150 ± 10%) |

- 7张随机战士卡牌，类型随机分配
- 其中1张随机打折50%，显示打折标签（label.jpg）
- 第一阶段仅战士职业，后续扩展可调整

### 遗物
| 稀有度 | 价格范围 |
|--------|----------|
| 普通 | 143 - 157 金币 (150 ± 5%) |
| 罕见 | 238 - 262 金币 (250 ± 5%) |
| 稀有 | 285 - 315 金币 (300 ± 5%) |
| 商店专属 | 143 - 157 金币 (150 ± 5%) |

- 共3个遗物槽位
- 最右侧(m_relicSlots[2])必为商店专属遗物
- 其余2个随机普通/罕见/稀有遗物
- **商店专属遗物仅在此来源可获得**

### 卡牌移除
- 每个商店限移除1张
- 跨商店递增：第1次75金币 → 第2次100金币 → 第3次125金币...
- 费用由 `GlobalSaveData::cardRemovalCost` 持久化追踪
- 使用后 `remove.jpg` 替换为 `soldout.jpg`

## 阶段二场景布局（1920×1080）

```
┌──────────────────────────────────────────────────┐
│  TopBar (HP / 金币 / 层数)                         │
│                                                    │
│          地毯 (carpet.jpg)                          │
│  ┌─────────────────────────────────────────┐      │
│  │ 卡1    卡2    卡3    卡4    卡5        │      │
│  │              第1排                      │      │
│  ├─────────────────────────────────────────┤      │
│  │ 卡6    卡7   遗物1   遗物2   遗物3     │ 删牌 │
│  │             第2排                       │      │
│  └─────────────────────────────────────────┘      │
│  ┌─────┐                                          │
│  │离开 │← 右半叠加地毯边缘，点击即离开商店            │
│  └─────┘                                          │
│                                                    │
│          手指跟随鼠标                                │
└──────────────────────────────────────────────────┘
```

## 输入/输出合同

复用 `EventAPI.h` 中的 `EventContext` 和 `EventResult`：

- **输入**：`EventContext.eventType = Merchant`
  - `currentHp, maxHp` → 创建 Player
  - `gold` → 购买力
  - `currentDeck` → 卡牌移除时的选择范围
  - `relics` → 已有遗物

- **输出**：`EventResult`
  - `goldChanged = true` → 购买/移除扣费后
  - `currentGold` → 剩余金币
  - `deckChanged = true` → 购买卡牌或移除卡牌后
  - `resultDeck` → 更新后的牌组
  - `relicsChanged = true` → 购买遗物后
  - `resultRelics` → 更新后的遗物列表

## 关键文件

| 文件 | 操作 |
|------|------|
| `src/ui/events/MerchantView.h` | 新建 — 两阶段场景管理 |
| `src/ui/events/MerchantView.cpp` | 新建 — 交互逻辑实现 |
| `src/api/EventLauncher.h` | 修改 — 实现 `launchMerchant()` |
| `src/api/EventLauncher.cpp` | 修改 — 从桩位替换为实际创建 MerchantView |
| `src/logic/globalsavedata.h` | 修改 — 新增 `cardRemovalCost` 字段 |
| `src/logic/globalsavedata.cpp` | 修改 — 初始化 `cardRemovalCost = 75` |
| `SlayTheQt.pro` | 修改 — 添加新源文件 |
| `resources.qrc` | 修改 — 添加 Merchant 资源 |
| `src/main.cpp` | 修改 — 测试入口 |

## 依赖

- `EventBaseView` — 基类（TopBar、RelicTray、玩家像、LeaveButton）
- `CardFactory::createCard(id)` — 按ID生成卡牌
- `CardFactory::getAllAvailableCardIds()` — 获取可用卡牌ID列表
- `RelicFactory::createRelic(id)` / `generateRandomRelic()` — 遗物生成
- `Player::modifyGold()` + `goldChanged` 信号 — 金币变动
- `GlobalSaveData` — 持久化 `cardRemovalCost`
