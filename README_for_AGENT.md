# 事件模块实现计划

## 进度概览

| 阶段 | 状态 | 产出与关键改动 |
|------|------|----------------|
| 阶段1：API 合约 + 基础设施 | 完成 | EventAPI.h, EventLauncher.h/.cpp |
| 阶段2：事件基类 + 公共 UI | 完成 | EventBaseView.h/.cpp, TopBar.h/.cpp, RelicTray.h/.cpp, LeaveButton.h/.cpp |
| 阶段 3.1：火堆 Campfire | 完成 | CampfireView.h/.cpp, IconButton.h/.cpp |
| 阶段 3.2：宝箱 Chest | 完成 | ChestView.h/.cpp, RelicPopupWidget.h/.cpp |
| 阶段 3.3：商人 Merchant | 完成 | MerchantView.h/.cpp, 遗物 Tooltip, 购买动画, 删牌服务 |
| **阶段 3.4：问号 QuestionMark** | **完成** | BigFish, Cleric, Designer, SelfNote, GoldenWing, WorldOfGoop, Ssssserpent |
| 阶段 4：卡牌组件统一与扩展 | 完成 | CardItem 适配诅咒/状态牌；新增 Doubt, Regret 诅咒牌 |
| 阶段 5：联调与边界处理 | 持续中 | 问号事件静默加牌规范、SelfNote 刷牌 Bug 修复 |

---

## 问号事件 (Question Mark) 当前状态 (截至 2026-06-05)

### 核心开发范式：沙盒与合同 (Sandbox & Contract)
为了确保事件逻辑不污染全局状态且易于联调，所有随机事件必须遵循以下范式：
- **数据沙盒**：`EventLauncher` 为每个事件创建临时的 `Player`、`CardManager` 指针。事件内所有变更（扣钱、扣血、加牌）仅作用于这些副本。
- **静默结算**：所有通过问号事件获得的卡牌，在 UI 上**均不播放飞向右上角的流星动画**。计牌器数字直接更新，确保节奏紧凑。
- **结算合同**：事件结束触发 `eventFinished()` 信号，由 `GameWindow` 接收 `EventResult` 统一更新 `GlobalSaveData`。

### 已完成事件集
- **遭遇战 (Monster Encounter)**：通过 `BattleLauncher` 桥接，直接进入普通怪物战斗，战后无缝同步血量与金币。
- **大鱼 (Big Fish)**：
    - [香蕉] 回血 (MaxHp / 3)；[甜甜圈] 提升 MaxHP 并回血；[盒子] 获得随机遗物并塞入**悔恨**诅咒。
- **蛇影 (The Ssssserpent)**：
    - [同意] 获得 175 金币，塞入**疑虑**诅咒；[反对] 无事发生。
- **黏液世界 (World of Goop)**：
    - [收集] 获得 75 金币，失去 11 HP；[放手] 随机失去 20~50 金币。
- **牧师 (The Cleric)**：
    - [治疗] 扣35金，回复25%血；[净化] 扣50金，启动卡牌移除。
- **尖端设计师 (The Designer)**：
    - [小修] 扣50金，随机升级2张；[清洁] 扣75金，移除1张；[全套] 扣110金，移除1并随机升级1张；[一拳] 扣3血。
- **留给自己的讯息 (Note For Yourself)**：
    - **逻辑修正**：改为**强制存牌模式**。点击后立即获得存款卡牌并强制开启存牌界面，必须选一张存入（移除返回按钮），防止无限刷牌。
- **翅膀雕像 (Golden Wing)**：
    - [祈祷] 扣7血，移除1张卡牌；[摧毁] 获得 50-80 随机金币。

---

## 卡牌系统增强 (2026-06-05)

### 1. 诅咒牌实现
- **疑虑 (Doubt)**: 诅咒类型。回合结束获得 1 层虚弱。深紫色边框，费用显示为不可打出。
- **悔恨 (Regret)**: 诅咒类型。回合结束手牌中每有一张牌，失去 1 点生命值（直接掉血）。

### 2. CardItem 渲染优化
- **类型适配**：`CardItem::paint` 现在支持 `CardType::Curse` (深紫色边框) 和 `CardType::Status` (灰色边框)。
- **费用球逻辑**：费用为 `-1` 的卡牌（不可打出）会自动隐藏费用球，呈现标准诅咒感。
- **文本渲染**：支持 `m_secondaryValue` 的动态描述展示，与战斗引擎深度挂钩。

---

## 商人 (Merchant) 系统
- **商品生成**：7张随机战士卡牌 + 3件遗物。
- **特效流星**：购买卡牌时播放“飞向右上角”动画（注意：仅商店购买保留此动画，问号事件已统一为静默处理）。
- **Tooltip 系统**：遗物悬停显示金色标题与描述。

---

## 核心架构清单

```
src/api/
  EventAPI.h                  ✅ 数据合同
  EventLauncher.h/.cpp        ✅ 暴露 getCardManager() 支持 UI 监听

src/ui/
  TopBar.h/.cpp               ✅ 队友文件，禁止直接修改。测试模式需用 setScale(1.2) 适配 1920 场景。
  carditem.h/.cpp             ✅ 统一卡牌渲染引擎，支持诅咒边框

src/entities/cards/
    DoubtCard.h               ✅ 疑虑实现 (无 Q_OBJECT 以防链接错误)
    RegretCard.h              ✅ 悔恨实现 (无 Q_OBJECT 以防链接错误)

src/ui/events/
    WorldOfGoopView.h/.cpp    ✅ 黏液世界
    SsssserpentView.h/.cpp    ✅ 蛇影
    SelfNoteView.h/.cpp       ✅ 逻辑修正后的留讯事件
```

---

## 测试与调试环境
- **main.cpp 测试宏**：
    - `TEST_EVENT 1`: 启用沙盒测试模式。
    - `TEST_WHICH`: 1=Campfire, 5=BigFish, 6=Goop, 7=Serpent, 8=SelfNote。
- **状态栏适配**：在测试模式下，`TopBar` 和 `RelicTray` 会自动缩放适配 `EventView` 的 1920 场景，且不对队友的原始 `.cpp` 文件做任何侵入性修改。
