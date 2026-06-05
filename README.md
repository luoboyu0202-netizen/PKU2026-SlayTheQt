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
遗物的架构与卡牌非常相似，但它有一个核心区别：卡牌是“主动触发”的，而遗物绝大多数是“被动监听”的（Hooks）。不仅如此，遗物的状态（比如钢笔尖的层数）需要跨越整个大地图进行持久化存档。

为你呈上这份架构师级别的《SlayTheQt：全新遗物制作终极指南》！🏺✨

第一步：构思与物料准备 (The Blueprint)
在敲代码之前，你需要为你的新遗物准备三样东西：

唯一的身份证 (ID)： 例如 relic_vajra (金刚杵)。

文本描述： 名称（金刚杵）、描述（战斗开始时，获得 1 点力量）。

高清贴图： 一张背景透明的 PNG 图片。

⚠️ 血的教训： 把图片放进项目后，立刻添加到 .qrc 文件里，绝对不要在路径里留空格，并且确认大小写完全匹配！

第二步：编写遗物逻辑类 (The Brain)
在你的 src/entities/relics/ 目录下，创建一个继承自基类 Relic 的新类。
遗物的核心魔法在于重写（Override）生命周期函数（Hooks）。你需要根据遗物的功能，选择在什么时机介入战斗。

常见钩子函数 (Hooks) 示例：

onBattleStart()：战斗开始时触发（适合：金刚杵、锚）。

onCardPlayed(Card* card)：每次打出牌时触发（适合：钢笔尖、死枝）。

onTurnStart() / onTurnEnd()：回合交替时触发（适合：水银沙漏、奥利哈钢）。

onEnemyDied(Enemy* enemy)：怪物死亡时触发（适合：肉食）。

💡 案例 A：一次性触发类（金刚杵）
C++
// Vajra.h
#pragma once
#include "Relic.h"
#include "../Player.h"

class Vajra : public Relic {
public:
    Vajra() : Relic("relic_vajra", "金刚杵", "战斗开始时，获得 1 点力量。", ":/resources/images/relics/relic_vajra.png") {}

    // 监听：战斗开始
    void onBattleStart(Player* player) override {
        // 赋予玩家 1 层力量 Buff
        player->addBuff(StatusType::Strength, 1);
        
        // 播放遗物闪烁动画（如果有的话）
        flash(); 
    }
};
💡 案例 B：跨战斗计数类（钢笔尖）
还记得咱们刚刚修好的计数器 Bug 吗？这就是计数遗物的标准写法！

C++
// PenNib.h
#pragma once
#include "Relic.h"

class PenNib : public Relic {
public:
    PenNib() : Relic("relic_pen_nib", "钢笔尖", "每打出第10张攻击牌，造成双倍伤害。", ":/resources/images/relics/relic_pen_nib.png") {
        // 如果存档里没有进度，默认从 0 开始
    }

    // 监听：玩家出牌
    void onCardPlayed(Card* card, Player* player) override {
        if (card->getType() == CardType::Attack) {
            int current = getCounter();
            setCounter(current + 1); // 计数器 +1

            if (getCounter() == 10) {
                // TODO: 赋予玩家【下一次攻击双倍】的特殊 Buff
                player->addBuff(StatusType::DoubleDamage, 1);
                setCounter(0); // 触发后清零
                flash();
            }
        }
    }
};
第三步：注册到遗物工厂 (The Factory)
遗物写好了，需要把它挂牌上市，让大地图的宝箱、Boss掉落和商人（Merchant）能够找到它。打开你的 RelicFactory.h。

1. 加入生成名单：

C++
static inline Relic* createRelic(const QString& id, QObject* parent = nullptr) {
    if (id == "relic_vajra") return new Vajra();
    if (id == "relic_pen_nib") return new PenNib();
    // ... 其他遗物 ...
    return nullptr;
}
2. 扔进随机池（供商人进货和盲盒掉落）：

C++
static inline QList<QString> getAllAvailableRelicIds() {
    return {
        "relic_burning_blood",
        "relic_vajra",
        "relic_pen_nib",
        "relic_anchor"
        // 将新 ID 加到这里！
    };
}
第四步：数据持久化收尾 (The Memory)
绝大多数新遗物到第三步就已经大功告成了！
但如果你的遗物是带有计数器的（像钢笔尖、日晷、开心果），你必须确保在两处地方做好了对接：

从全局读取（入场）： BattleLauncher 生成遗物时，需要把全局的计数器喂给它：

C++
// 伪代码参考
Relic* r = RelicFactory::createRelic(id);
r->setCounter(save->relicCounters[id]); 
写回全局（结算）： 战斗结束时，必须把最新层数存回全局（我们之前在 BattleLauncher 里的结算逻辑已经完美涵盖了这一点！）。

🎉 检查清单 (Checklist)
每次新建遗物，在点下编译按钮前，在脑子里过一遍这 4 个问题：

[ ] 图片有没有放进 .qrc？路径有没有死角（空格、乱码）？

[ ] 是否在 RelicFactory 里的 createRelic 注册了？

[ ] 是否在 RelicFactory 里的 getAllAvailableRelicIds 加入了奖池？

[ ] 钩子函数（Hook）里操作 Player 数据时，有没有判断空指针防闪退？

按照这份指南，你就像有了一台《杀戮尖塔》的 3D 打印机，想要什么神仙遗物都能顺滑地产出喵！接下来打算去造哪个遗物练练手？🐾✨
