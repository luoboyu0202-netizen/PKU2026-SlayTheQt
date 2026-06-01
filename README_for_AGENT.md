# 事件模块实现计划

## 进度概览

| 阶段 | 状态 | 产出与关键改动 |
|------|------|----------------|
| 阶段1：API 合约 + 基础设施 | 完成 | EventAPI.h, EventLauncher.h/.cpp |
| 阶段2：事件基类 + 公共 UI | 完成 | EventBaseView.h/.cpp, TopBar.h/.cpp, RelicTray.h/.cpp, LeaveButton.h/.cpp |
| 阶段 3.1：火堆 Campfire | 完成 | CampfireView.h/.cpp, IconButton.h/.cpp |
| 阶段 3.2：宝箱 Chest | 完成 | ChestView.h/.cpp, RelicPopupWidget.h/.cpp |
| 阶段 3.3：商人 Merchant | 完成 | MerchantView.h/.cpp, 遗物 Tooltip, 购买动画, 删牌服务 |
| **阶段 3.4：问号 QuestionMark** | **完成** | GenericChoiceEventView, BigFish, Cleric, Designer, SelfNote, GoldenWing |
| 阶段 4：卡牌组件统一 | 完成 | CardItem 替代 ShopCardItem + SelectableCardItem |
| 阶段 5：联调与边界处理 | 持续中 | 编译修复、资源格式转换、UI 对齐 |

---

## 问号事件 (Question Mark) 当前状态 (截至 2026-05-31)

### 已完成事件集
- **遭遇战 (Monster Encounter)**：通过 `BattleLauncher` 桥接，直接进入普通怪物战斗，战后无缝同步血量与金币。
- **大鱼 (Big Fish)**：
    - [香蕉] 回血 (MaxHp / 3)；[甜甜圈] 提升 MaxHP 并回血；[盒子] 获得随机遗物并塞入占位诅咒(Strike)。
- **牧师 (The Cleric)**：
    - [治疗] 扣35金，回复25%血；[净化] 扣50金，启动卡牌移除（模态复用商店逻辑）。
- **尖端设计师 (The Designer)**：
    - 复杂的动态判定：[小修] 扣50金，随机升级2张；[清洁] 扣75金，移除1张；[全套] 扣110金，移除1并随机升级1张；[一拳] 扣3血，动态切换插图为 `Punched.png`。
- **留给自己的讯息 (Note For Yourself)**：
    - **跨局联动**：在 `GlobalSaveData` 中持久化记录 `storedCardId` 和 `isStoredCardUpgraded`。
    - **两阶段交换**：点击后先获得存款卡牌，再强制开启选牌界面存入一张新卡牌。
- **翅膀雕像 (Golden Wing)**：
    - [祈祷] 扣7血，移除1张卡牌；[摧毁] 获得 50-80 随机金币。

### 核心技术沉淀
- **`GenericChoiceEventView` 框架**：
    - 标准化“左侧 650x650 插图 + 右侧 HTML 滑动文本 + 底部悬停按钮”的选项类事件范式。
    - **UI 隔离**：使用 `setOptionsEnabled` 实现进入删牌/选牌网格时的模态独占，杜绝套娃点击。
    - **富文本渲染**：引入 `QGraphicsProxyWidget` + `QTextBrowser` 组合，支持文字滚动和关键字 **加粗**（如 `<b>遗物</b>`）。
- **流程规范**：所有涉及“服务”（删牌、换牌）的交互已统一采用“选中 -> 确认/返回”二次操作流，避免误触。

---

## 商人 (Merchant) 当前状态 (截至 2026-05-30)

### 已完成
- **两阶段架构**：阶段一（相遇：玩家+商人+前进箭头）→ 阶段二（购买：地毯+商品+手臂光标）
- **商品生成**：7张随机战士卡牌（5+2布局）+ 3件遗物（最右=商店专属）
- **定价系统**：卡牌价格按稀有度（普通45-55g / 罕见68-82g / 稀有135-165g），1张随机打折50%
- **卡牌购买**：点击→金币扣减→卡牌熔融为光点飞向右上角（径向渐变光球+粒子尾迹+模糊光晕）
- **遗物购买**：支持图标渲染、价格逻辑、Tooltip 显示（金色标题+白色描述）以及购买时的光点特效
- **卡牌移除服务**：点击删牌→暗幕+牌组选择UI→确认移除→扣费→售罄占位。每商店限1次，跨商店递增75→100→125...
- **UI 微调**：
    - 遗物位置上移，与商品区视觉更协调
    - 修复了 `CardItem` 在选中态下金色边框压住能量球数字的层级问题
    - 修复了 `MerchantView` 编译错误（缺失 tooltip 声明）
- **资源补全**：将遗物图标正式注册至 `resources.qrc`，修复了 `PenNibRelic` 路径缺失问题
- **手臂光标**：仅悬停商品时从屏幕上方移入商品边缘；离开时连续动画移回上方消失
- **离开飘带**：第二排左侧，点击返回阶段一（保留购物状态）
- **前进箭头**：阶段一使用 GO_ahead.png，退出事件
- **价格标识**：价格标签内置在卡牌中，金色=可购买，红色=余额不足

---

## 卡牌组件统一 (2026-05-29)

原来的三个并行卡牌UI组件已统一为队友的 `CardItem`：

| 组件 | 统一前 | 统一后 |
|------|--------|--------|
| 战斗手牌 | CardItem | 不变 |
| 营火升级选牌 | SelectableCardItem | **CardItem** |
| 营火升级动画 | SelectableCardItem | **CardItem** |
| 商店购买展示 | ShopCardItem | **CardItem** |
| 商店删牌选择 | SelectableCardItem | **CardItem** |

**CardItem 扩展 (carditem.h/.cpp):**
- `setSelectionEnabled(bool)` + `cardClicked(CardItem*)` 信号 — 非战斗选牌模式
- `setHighlighted(bool)` + `isHighlighted()` — 金色边框选中态
- `setPrice(int)` + `setOnSale(bool)` + `setAffordable(bool)` — 商店价格渲染
- BattleEngine 空指针保护 — 事件模式无 BattleEngine 时不崩溃

**移除的组件：**
- `ShopCardItem.h/.cpp` — .pro中已注释
- `SelectableCardItem.h/.cpp` — .pro中已注释

---

## 核心改进总结

### 1. UI 架构与一致性
- **状态栏 (TopBar)**: 克隆战斗模块绘制逻辑，包含角色名、心形HP、金币图标
- **遗物栏**: `RelicTray` 已接入 `EventBaseView`，匹配战斗模块排版
- **缩放方案**: `fitInView(0, 0, 1920, 1080)` 固定
- **卡牌组件统一**: 全事件使用单一 `CardItem`，消除三套重复渲染代码

### 2. 火堆 (Campfire)
- OTS视角、静态写实火堆、休息烟雾动画、升级选牌/确认/动画流程
- 已迁移至 CardItem

### 3. 宝箱 (Chest)
- 水晶宝箱+星芒闪烁→点击开箱→随机遗物弹窗→拾取/跳过
- RelicPopupWidget 独立弹窗组件

### 4. 系统稳定性
- 同伴文件保护规则：缺失文件注释而非删除
- WebP文件自动检测与PNG转换
- BattleEngine空指针防护

---

## 文件清单

```
src/api/
  EventAPI.h                  ✅ 事件枚举与数据合同
  EventLauncher.h/.cpp        ✅ 调度器（全事件已接入）

src/ui/
  TopBar.h/.cpp               ✅ 状态栏
  RelicTray.h/.cpp            ✅ 遗物托盘
  carditem.h/.cpp             ✅ 统一卡牌组件（战斗+事件）
  cardbrowseroverlay.h/.cpp   ✅ 牌堆浏览
  events/
    EventBaseView.h/.cpp      ✅ 基类
    CampfireView.h/.cpp       ✅ 火堆（已用CardItem）
    ChestView.h/.cpp          ✅ 宝箱
    MerchantView.h/.cpp       🚧 商人（进行中）
    IconButton.h/.cpp         ✅ 图标按钮
    LeaveButton.h/.cpp        ✅ 前进引导按钮
    TextButton.h/.cpp         ✅ 文本按钮
    RelicPopupWidget.h/.cpp   ✅ 遗物弹窗
    # ShopCardItem.h/.cpp     ❌ 已废弃（CardItem替代）
    # SelectableCardItem.h/.cpp ❌ 已废弃（CardItem替代）

src/entities/relics/
    relic.h                   ✅ 遗物基类（含getId/getDescription）
    relicmanager.h/.cpp       ✅ 遗物管理
    RelicImplementations.cpp  ✅ 具体遗物实现

src/logic/
    cardfactory.h/.cpp        ✅ 卡牌工厂（20张可玩+4状态牌）
    relicfactory.h/.cpp       ✅ 遗物工厂
    cardmanager.h/.cpp        ✅ 卡牌管理（含removeCardPermanently）
    globalsavedata.h          ✅ 全局存档（含cardRemovalCost）
```

---

## 构建注意事项
- 编译器：`E:\Badstuff\Software\Qt\Tools\mingw1310_64\bin\g++.exe`
- Qt路径：`E:\Badstuff\Software\Qt\6.11.0\mingw_64\`
- PATH: `E:\Badstuff\Software\Qt\Tools\mingw1310_64\bin;E:\Badstuff\Software\Qt\6.11.0\mingw_64\bin`
- 编译命令：`qmake ../SlayTheQt.pro && mingw32-make -j4`
- 同伴文件规则：缺失文件在.pro中 `# 注释`，不要删除
- WebP文件需转为PNG后在qrc中引用.png
