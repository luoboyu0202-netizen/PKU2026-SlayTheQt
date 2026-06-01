# SlayTheQt — 队友接入指南

## 快速开始：测试当前事件

`src/main.cpp` 内含开关式测试入口：

```cpp
#define TEST_EVENT 0     // 0=大地图模式, 1=独立测试事件
#define TEST_WHICH 5     // 1=CamoFire, 2=Chest, 3=Merchant, 4=QM_GoldenWing, 5=QM_BigFish
```

改为 `TEST_EVENT 1` 即可跳过地图直接进入对应事件。

---

## 事件系统架构

```
EventLauncher::launch(EventContext)
  ├─ EventType::Campfire   → CampfireView
  ├─ EventType::Chest      → ChestView
  ├─ EventType::Merchant   → MerchantView
  └─ EventType::QuestionMark → 根据 context.eventSubtype 路由:
       ├─ "BigFish"          → BigFishView
       ├─ "Cleric"           → ClericView
       ├─ "Designer"         → DesignerView
       ├─ "SelfNote"         → SelfNoteView
       ├─ "GoldenWing"       → GoldenWingView
       └─ "MonsterEncounter" → BattleLauncher (默认)
```

所有事件 View 继承 `EventBaseView`，统一拥有 TopBar、RelicTray、LeaveButton。

---

## 队友接入教程

### 添加新的事件子类型（以 QuestionMark 为例）

**Step 1：创建 View 类**

在 `src/ui/events/` 下新建 `.h` 和 `.cpp`：

```cpp
// MyNewEventView.h
#pragma once
#include "EventBaseView.h"

class MyNewEventView : public EventBaseView {
    Q_OBJECT
public:
    explicit MyNewEventView(Player* player, CardManager* cardManager,
                            RelicManager* relicManager, QWidget* parent = nullptr);
protected:
    void setupContent() override;
signals:
    void eventFinished(); // 继承自 EventBaseView，结束事件时 emit
};
```

```cpp
// MyNewEventView.cpp
#include "MyNewEventView.h"

MyNewEventView::MyNewEventView(Player* player, CardManager* cardManager,
                               RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, cardManager, relicManager, parent) {
    setupContent();
}

void MyNewEventView::setupContent() {
    // 1. 设置背景、插图、文本
    // 2. 添加交互按钮
    // 3. 处理完成后 emit eventFinished()
}
```

**Step 2：在 EventLauncher 中注册**

编辑 `src/api/EventLauncher.cpp`：

```cpp
// 顶部添加 include
#include "../ui/events/MyNewEventView.h"

// 在 launchQuestionMark() 的 else-if 链中添加:
else if (context.eventSubtype == "MyNewEvent") {
    m_view = new MyNewEventView(player, cardManager, relicManager);
    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        // 根据需要设置 result.hpChanged / goldChanged / deckChanged / relicsChanged
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });
    m_view->show();
}
```

**Step 3：注册到构建系统**

编辑 `SlayTheQt.pro`，在 `SOURCES` 和 `HEADERS` 中添加新文件：

```
SOURCES += \
    src/ui/events/MyNewEventView.cpp

HEADERS += \
    src/ui/events/MyNewEventView.h
```

> ⚠️ **队友文件保护规则**：如果某个队友的文件在你的本地不存在（如 `src/core/gamerng.cpp`），**使用 `#` 注释掉而非删除**，避免合并冲突。

**Step 4：添加资源**

将图片放入 `resources/images/events/` 对应目录，在 `resources.qrc` 的 `<qresource>` 中添加：

```xml
<file>resources/images/events/YourEvent/image.png</file>
```

> ⚠️ 确保文件是真正的 PNG/JPEG 格式，**WebP 伪装成 .jpg 的文件会导致 QPixmap 加载失败**。

### 使用 CardFactory / RelicFactory

```cpp
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"

// 获取全部可用 ID
QList<QString> cardIds = CardFactory::getAllAvailableCardIds();

// 按 ID 创建（父对象负责生命周期）
Card* card = CardFactory::createCard("card_strike", this);

// 随机创建
Card* randomCard = CardFactory::generateRandomCard(this);
Relic* randomRelic = RelicFactory::generateRandomRelic(this);
```

### CardItem 使用说明

所有事件中展示卡牌统一使用 `CardItem`（`src/ui/carditem.h`）：

```cpp
auto* item = new CardItem(card);            // 构造
item->setSelectionEnabled(true);             // 启用点击选中（事件模式）
item->setHighlighted(true);                  // 金色高亮边框
item->setPrice(75);                          // 商店价格
item->setOnSale(true);                       // 打折标识
item->setAffordable(gold >= price);          // 更新价格颜色
// 信号: cardClicked(CardItem*)              // 点击时发射
```

### GlobalSaveData 扩展

如需跨事件持久化数据，在 `src/logic/globalsavedata.h` 的 `GlobalSaveData` 单例中添加字段，并在 `initNewGame()` 中初始化。

---

## 数据合同

**EventContext**（输入）：`src/api/EventAPI.h`
```cpp
struct EventContext {
    int currentHp, maxHp, gold, maxEnergy;
    QList<Card*> currentDeck;
    QList<Relic*> relics;
    EventType eventType;
    QString eventSubtype;    // 问号事件的子类型
};
```

**EventResult**（输出）：同一文件
```cpp
struct EventResult {
    int remainingHp, currentGold;
    QList<Card*> resultDeck;
    QList<Relic*> resultRelics;
    bool hpChanged, deckChanged, relicsChanged, goldChanged;
    bool playerDead;
};
```

**EventType 枚举**：`Campfire, Merchant, Chest, QuestionMark`

---

## 构建

```
编译器: E:\Badstuff\Software\Qt\Tools\mingw1310_64\bin\g++.exe
Qt:     E:\Badstuff\Software\Qt\6.11.0\mingw_64\
PATH:   Qt\Tools\mingw1310_64\bin;Qt\6.11.0\mingw_64\bin
命令:   qmake ../SlayTheQt.pro && mingw32-make -j4
```

---

## 战斗系统测试示例

```cpp
#include <QApplication>
#include "api/BattleLauncher.h"
#include "api/BattleAPI.h"
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    BattleContext context;
    context.currentHp = 75;
    context.maxHp = 80;
    context.gold = 120;
    context.maxEnergy = 3;
    context.enemySeedOrId = "Slime_Squad";

    QList<QString> testDeckIds = {
        "card_strike", "card_strike", "card_strike",
        "card_defend", "card_defend", "card_defend",
        "card_bash", "card_thunderclap", "card_dark_embrace"
    };

    for (const QString& id : testDeckIds) {
        Card* newCard = CardFactory::createCard(id, &a);
        if (newCard) { newCard->upgrade(); context.currentDeck.append(newCard); }
    }

    QList<QString> testRelicIds = {
        "relic_pen_nib", "relic_orichalcum",
        "relic_bag_of_preparation", "relic_anchor"
    };

    for (const QString& id : testRelicIds) {
        Relic* newRelic = RelicFactory::createRelic(id, &a);
        if (newRelic) context.relics.append(newRelic);
    }

    BattleLauncher launcher;
    QObject::connect(&launcher, &BattleLauncher::battleConcluded, [](BattleResult result) {
        qDebug() << "Battle Finished! Victory:" << result.isVictory;
    });

    launcher.launch(context);
    return a.exec();
}
```
