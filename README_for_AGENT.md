# 事件模块实现计划

## 进度概览

| 阶段 | 状态 | 产出与关键改动 |
|------|------|----------------|
| 阶段1：API 合约 + 基础设施 | 完成 | EventAPI.h, EventLauncher.h/.cpp |
| 阶段2：事件基类 + 公共 UI | 完成 | EventBaseView.h/.cpp, TopBar.h/.cpp, RelicTray.h/.cpp, LeaveButton.h/.cpp |
| 阶段 3.1：火堆 Campfire | 完成（深度视觉调优 + 交互闭环 + OTS 视角） | CampfireView.h/.cpp, IconButton.h/.cpp |
| 阶段 3.2：宝箱 Chest | 未开始 | — |
| 阶段 3.3：商人 Merchant | 未开始 | — |
| 阶段 3.4：问号 QuestionMark | 未开始 | — |
| 阶段 4：联调与边界处理 | 持续中 | UI 模块间对齐与资源自动化处理 |


---

## 核心改进总结 (截至 2026-05-27)

### 1. UI 架构与一致性
*   **状态栏 (TopBar) 完全克隆**：重构了 `TopBar.cpp` 的绘制逻辑，包含角色名显示、心形 HP 图标、金币图标，并改用 Arial 16pt Bold 字体以提升清晰度。背景色条调亮，确保在 1080p 下具有高辨识度。
*   **遗物栏同步**：在事件基类 `EventBaseView` 中引入了 `RelicTray`，并实现了与战斗模块 1:1 匹配的排版布局（(450, 6) 槽位）。
*   **缩放方案固定**：采用 `fitInView(0, 0, 1920, 1080)` 替代不稳定的手动 `scale()`，确保场景原点永远精准对齐窗口左上角。
*   **IconButton 架构升级**：重构了图标加载与缩放逻辑，采用 `scaledToHeight(150)` 结合居中绘制算法，彻底解决了不同素材比例导致的按钮高度不统一问题。

### 2. 火堆 (Campfire) 视觉与交互
*   **真实越肩视角 (OTS)**：引入了 `rear_side-removebg-preview.png`，将玩家像大幅放大并下移，营造出极强的近景沉浸感。
*   **素材替换**：火堆核心替换为带透明通道的 `Bonfire.png`，移除了不必要的呼吸动画与冗余的程序火焰，保持静态写实风格，并辅以微弱的火星粒子效果。
*   **选项 UI 优化**：
    *   **高度对齐**：通过底层的 `IconButton` 逻辑，确保“休息”与“锻造”图标高度绝对一致。
    *   **交互蒙版**：在选牌阶段（升级）显著加深背景蒙版（Alpha 200），并将层级置于人物之上，使卡牌成为视觉中心。确认升级后立即撤销蒙版，恢复原始亮度以展示精美的升级动画。
*   **休息动画增强**：
    *   **全屏致盲**：白色遮罩 Z-Order 提升至 300，粒子数增加到 120+，模拟瞬间爆发的烟雾。
    *   **熄火逻辑**：动画中途自动隐藏所有火、光图元，实现“操作完即熄灭”的物理交互。

### 3. 系统稳定性与修复
*   **链接错误修复**：解决了 `OrichalcumRelic` 和 `PenNibRelic` 的 `vtable` 未定义问题，将具体实现移至 `.cpp`。
*   **访问权限调整**：将 `EventBaseView` 的核心成员（`m_scene`, `m_playerImage` 等）改为 `protected`，确保子类能正常操控 UI 元素。
*   **RelicManager 修正**：修复了构造函数声明与定义不匹配以及重定义的问题。


### 3. 资源系统修复
*   **资源清单清理**：移除了 non-existent 的 `.webp` 条目，防止构建系统报错。
*   **类型纠正**：将 `m_choiceCloud` 类型修正为 `QGraphicsEllipseItem*`，解决了 `setBrush/setPen` 导致的编译失败。

### 4. 交付与同步
*   **目标仓库**：`PKU2026-SlayTheQt`
*   **同步状态**：已完成所有新增功能代码、素材及配置文件的复制与提交。
*   **Commit 信息**：`Feat: Complete Campfire event implementation with enhanced visuals, animations, and matching TopBar UI`
*   **遗留项**：此工作区已完成阶段性交付，后续开发可在 `PKU2026-SlayTheQt` 目录下进行。

---

## 实际文件清单

```
src/api/
  EventAPI.h                  ✅ 事件枚举与数据合同
  EventLauncher.h/.cpp        ✅ 调度器（支持结算自销毁）

src/ui/
  TopBar.h/.cpp               ✅ 战斗级状态栏（角色名+HP+金币）
  RelicTray.h/.cpp            ✅ 遗物托盘接入
  events/
    EventBaseView.h/.cpp      ✅ 基类：负责 TopBar/RelicTray/玩家像/前进引导
    CampfireView.h/.cpp       ✅ 火堆：包含熄火逻辑、越肩视角、增强烟雾
    IconButton.h/.cpp         ✅ 统一图标容器：支持强制 280x180 缩放
    LeaveButton.h/.cpp        ✅ 引导按钮：支持动态抠图（去黑背景）算法
    TextButton.h/.cpp         ✅ 选牌确认/取消按钮
```

---

## 下一步计划
1.  **实现宝箱 (Chest)**：开发随机遗物拾取逻辑与打开动画。
2.  **实现商人 (Merchant)**：开发随机商品池（3卡+2遗物）与购买扣款逻辑。
3.  **开发 RelicFactory**：为宝箱和商人提供遗物生成支撑。
