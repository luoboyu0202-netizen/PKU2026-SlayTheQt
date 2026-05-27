# Chest Event Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the Chest event where a crystal chest on the right side of a dungeon scene drops a random relic for the player to take or skip.

**Architecture:** ChestView extends EventBaseView (inheriting TopBar, RelicTray, LeaveButton). A new RelicPopupWidget handles the relic reveal popup with hover tooltip, Take/Skip buttons. RelicFactory::generateRandomRelic() provides the random relic. The view uses battle perspective (player left, chest right) instead of the Campfire over-the-shoulder layout.

**Tech Stack:** Qt 6 C++17, QGraphicsView/Scene, QGraphicsWidget, RelicFactory, RelicManager

**Design doc:** `docs/plans/2026-05-27-chest-event-design.md`

---

### Task 1: Add Chest resources

**Files:**
- Create: `resources/images/events/Chest/Chest.png` (user provides)
- Create: `resources/images/events/Chest/Background.png` (user provides)
- Create: `resources/images/events/Chest/Chest_open.png` (user provides — open chest, no sparkle)
- Modify: `resources.qrc`

- [ ] **Step 1: Create Chest resource directory and add resource entries**

Create the directory:
```bash
mkdir -p resources/images/events/Chest
```

Then add to `resources.qrc` after the Campfire entries:
```xml
<file>resources/images/events/Chest/Chest.png</file>
<file>resources/images/events/Chest/Background.png</file>
<file>resources/images/events/Chest/Chest_open.png</file>
```

- [ ] **Step 2: Commit**

```bash
git add resources.qrc resources/images/events/Chest/
git commit -m "assets: add chest event resource images and qrc entries"
```

---

### Task 2: Create RelicPopupWidget

**Files:**
- Create: `src/ui/events/RelicPopupWidget.h`
- Create: `src/ui/events/RelicPopupWidget.cpp`

- [ ] **Step 1: Write RelicPopupWidget header**

```cpp
#pragma once
#include <QGraphicsWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QString>
#include <QFont>
#include <QTimer>

class Relic;
class TextButton;

class RelicPopupWidget : public QGraphicsWidget {
    Q_OBJECT

public:
    explicit RelicPopupWidget(Relic* relic, QGraphicsItem* parent = nullptr);

signals:
    void takeClicked();
    void skipClicked();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    void setupUI();
    void showTooltip();
    void hideTooltip();

    Relic* m_relic;

    QGraphicsRectItem* m_panelBg = nullptr;
    QGraphicsPixmapItem* m_relicIcon = nullptr;
    QGraphicsTextItem* m_titleText = nullptr;
    QGraphicsTextItem* m_nameText = nullptr;
    QGraphicsTextItem* m_descText = nullptr;

    TextButton* m_takeBtn = nullptr;
    TextButton* m_skipBtn = nullptr;

    QGraphicsRectItem* m_tooltipBg = nullptr;
    QGraphicsTextItem* m_tooltipText = nullptr;
    bool m_hovered = false;
};
```

- [ ] **Step 2: Write RelicPopupWidget implementation**

```cpp
#include "RelicPopupWidget.h"
#include "TextButton.h"
#include "../../entities/relics/Relic.h"
#include <QPainter>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

RelicPopupWidget::RelicPopupWidget(Relic* relic, QGraphicsItem* parent)
    : QGraphicsWidget(parent)
    , m_relic(relic)
{
    setupUI();
    setAcceptHoverEvents(true);
}

void RelicPopupWidget::setupUI() {
    // Full-screen semi-transparent backdrop
    m_panelBg = new QGraphicsRectItem(0, 0, 1920, 1080, this);
    m_panelBg->setBrush(QColor(0, 0, 0, 120));
    m_panelBg->setPen(Qt::NoPen);

    // Central card panel
    auto* cardBg = new QGraphicsRectItem(0, 0, 600, 380, m_panelBg);
    cardBg->setBrush(QColor(30, 25, 20, 240));
    cardBg->setPen(QPen(QColor(180, 150, 100), 3));
    cardBg->setPos(660, 280);
    cardBg->setZValue(1);

    // Title: "发现遗物！"
    m_titleText = new QGraphicsTextItem("发现遗物！", cardBg);
    m_titleText->setDefaultTextColor(QColor(255, 215, 120));
    m_titleText->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    m_titleText->setPos(300 - m_titleText->boundingRect().width() / 2, 18);
    m_titleText->setZValue(2);

    // Relic icon (placeholder - will attempt to load from resources)
    m_relicIcon = new QGraphicsPixmapItem(cardBg);
    QPixmap iconPixmap(":/resources/images/relics/" + m_relic->getId() + ".png");
    if (iconPixmap.isNull()) {
        // Fallback: draw a colored rect as placeholder
        iconPixmap = QPixmap(80, 80);
        iconPixmap.fill(QColor(100, 180, 220));
    }
    m_relicIcon->setPixmap(iconPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_relicIcon->setPos(60, 90);
    m_relicIcon->setZValue(2);

    // Relic name
    m_nameText = new QGraphicsTextItem(m_relic->getName(), cardBg);
    m_nameText->setDefaultTextColor(QColor(255, 250, 240));
    m_nameText->setFont(QFont("Microsoft YaHei", 22, QFont::Bold));
    m_nameText->setPos(160, 95);
    m_nameText->setZValue(2);

    // Relic short description
    m_descText = new QGraphicsTextItem(m_relic->getDescription(), cardBg);
    m_descText->setDefaultTextColor(QColor(200, 195, 185));
    m_descText->setFont(QFont("Microsoft YaHei", 14));
    m_descText->setPos(60, 190);
    m_descText->setTextWidth(480);
    m_descText->setZValue(2);

    // Take button
    m_takeBtn = new TextButton("拾取", 180, 50, cardBg);
    m_takeBtn->setPos(100, 300);
    m_takeBtn->setZValue(2);
    connect(m_takeBtn, &TextButton::clicked, this, &RelicPopupWidget::takeClicked);

    // Skip button
    m_skipBtn = new TextButton("跳过", 180, 50, cardBg);
    m_skipBtn->setPos(320, 300);
    m_skipBtn->setZValue(2);
    connect(m_skipBtn, &TextButton::clicked, this, &RelicPopupWidget::skipClicked);

    setZValue(200);
}

void RelicPopupWidget::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event)
    m_hovered = true;
    showTooltip();
}

void RelicPopupWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event)
    m_hovered = false;
    hideTooltip();
}

void RelicPopupWidget::showTooltip() {
    if (m_tooltipBg) return;

    QString tipText = m_relic->getDescription();
    QFont tipFont("Microsoft YaHei", 13);
    QFontMetrics fm(tipFont);
    QRect textRect = fm.boundingRect(QRect(0, 0, 350, 0), Qt::TextWordWrap, tipText);
    int w = textRect.width() + 24;
    int h = textRect.height() + 24;

    m_tooltipBg = new QGraphicsRectItem(-w/2, -h - 10, w, h, m_relicIcon);
    m_tooltipBg->setBrush(QColor(20, 18, 15, 235));
    m_tooltipBg->setPen(QPen(QColor(180, 150, 100), 1));
    m_tooltipBg->setZValue(5);

    m_tooltipText = new QGraphicsTextItem(tipText, m_tooltipBg);
    m_tooltipText->setDefaultTextColor(QColor(255, 255, 245));
    m_tooltipText->setFont(tipFont);
    m_tooltipText->setPos(-w/2 + 12, -h - 10 + 12);
    m_tooltipText->setTextWidth(340);
    m_tooltipText->setZValue(6);
}

void RelicPopupWidget::hideTooltip() {
    if (m_tooltipBg) {
        delete m_tooltipBg;
        m_tooltipBg = nullptr;
        m_tooltipText = nullptr;
    }
}
```

- [ ] **Step 3: Commit**

```bash
git add src/ui/events/RelicPopupWidget.h src/ui/events/RelicPopupWidget.cpp
git commit -m "feat: add RelicPopupWidget for chest event relic display"
```

---

### Task 3: Create ChestView

**Files:**
- Create: `src/ui/events/ChestView.h`
- Create: `src/ui/events/ChestView.cpp`

- [ ] **Step 1: Write ChestView header**

```cpp
#pragma once
#include "EventBaseView.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QList>

class RelicPopupWidget;
class Relic;

class ChestView : public EventBaseView {
    Q_OBJECT

public:
    explicit ChestView(Player* player, RelicManager* relicManager,
                       QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    void onChestClicked();
    void onTakeRelic(Relic* relic);
    void onSkipRelic();
    void showResult();

    // Chest visual
    QGraphicsPixmapItem* m_chestImage = nullptr;
    QPixmap m_chestClosedPixmap;
    QPixmap m_chestOpenPixmap;
    QPixmap m_backgroundPixmap;

    // Background
    QGraphicsPixmapItem* m_backgroundItem = nullptr;

    // Sparkle particle items (pre-opening)
    QList<QGraphicsEllipseItem*> m_sparkleParticles;

    // Relic popup
    RelicPopupWidget* m_relicPopup = nullptr;

    // Current relic being offered
    Relic* m_offeredRelic = nullptr;

    bool m_chestOpened = false;
};
```

- [ ] **Step 2: Write ChestView implementation — constructor and setupContent**

```cpp
#include "ChestView.h"
#include "RelicPopupWidget.h"
#include "../../entities/relics/Relic.h"
#include "../../entities/relics/RelicManager.h"
#include "../../entities/Player.h"
#include "../../logic/RelicFactory.h"
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <cmath>

ChestView::ChestView(Player* player, RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, nullptr, relicManager, parent)
{
    setupContent();
}

void ChestView::setupContent() {
    // 1. Background image
    m_backgroundPixmap.load(":/resources/images/events/Chest/Background.png");
    if (!m_backgroundPixmap.isNull()) {
        m_backgroundItem = new QGraphicsPixmapItem();
        m_backgroundItem->setPixmap(m_backgroundPixmap.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_backgroundItem->setPos(0, 0);
        m_backgroundItem->setZValue(-10);
        m_scene->addItem(m_backgroundItem);
    }

    // 2. Reposition player for battle perspective (left side, face right)
    // EventBaseView placed player at (-200, 380) scale 800x1200 for over-the-shoulder.
    // For battle view: reposition to left-center at normal scale.
    if (m_playerImage) {
        m_playerImage->setPos(100, 350);
        QPixmap scaled = m_playerPixmap.scaled(450, 675, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_playerImage->setPixmap(scaled);
        m_playerImage->setZValue(10);
    }

    // 3. Chest image (right-center, with sparkle)
    m_chestClosedPixmap.load(":/resources/images/events/Chest/Chest.png");
    m_chestOpenPixmap.load(":/resources/images/events/Chest/Chest_open.png");
    m_chestImage = new QGraphicsPixmapItem();
    if (!m_chestClosedPixmap.isNull()) {
        auto scaled = m_chestClosedPixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_chestImage->setPixmap(scaled);
    }
    m_chestImage->setPos(1300, 580);
    m_chestImage->setZValue(15);
    m_scene->addItem(m_chestImage);

    // 4. Sparkle particles around chest (before opening)
    auto* rng = QRandomGenerator::global();
    QPointF chestCenter(1450, 730);

    for (int i = 0; i < 8; ++i) {
        qreal size = 3 + rng->bounded(5);
        auto* sparkle = new QGraphicsEllipseItem(-size/2, -size/2, size, size, nullptr);
        sparkle->setBrush(QColor(255, 250, 180, 200 + rng->bounded(55)));
        sparkle->setPen(Qt::NoPen);

        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal dist = 60 + rng->bounded(140);
        sparkle->setPos(chestCenter.x() + cos(angle) * dist,
                        chestCenter.y() + sin(angle) * dist - rng->bounded(80));
        sparkle->setZValue(16);
        m_scene->addItem(sparkle);
        m_sparkleParticles.append(sparkle);

        // Twinkle animation
        auto* opacityEff = new QGraphicsOpacityEffect();
        opacityEff->setOpacity(0.3 + rng->bounded(70) / 100.0);
        sparkle->setGraphicsEffect(opacityEff);

        auto* twinkle = new QVariantAnimation(this);
        twinkle->setDuration(600 + rng->bounded(800));
        twinkle->setLoopCount(-1);
        int phase = rng->bounded(100);
        twinkle->setKeyValueAt(0.0, 0.3);
        twinkle->setKeyValueAt(0.5, 1.0);
        twinkle->setKeyValueAt(1.0, 0.3);
        twinkle->setEasingCurve(QEasingCurve::InOutSine);
        connect(twinkle, &QVariantAnimation::valueChanged, this,
                [opacityEff](const QVariant& v) { opacityEff->setOpacity(v.toReal()); });
        twinkle->start();
    }

    // 5. Make chest clickable via scene event
    // We handle click in the view by checking if user clicked near chest area.

    // 6. Hide leave button initially
    setLeaveButtonVisible(false);

    // 7. Instruction text
    auto* instructText = new QGraphicsTextItem("点击宝箱打开...");
    instructText->setDefaultTextColor(QColor(255, 250, 200));
    instructText->setFont(QFont("Microsoft YaHei", 18));
    instructText->setPos(1400, 800);
    instructText->setZValue(17);
    m_scene->addItem(instructText);
}
```

- [ ] **Step 3: Write ChestView — chest click handler**

```cpp
void ChestView::onChestClicked() {
    if (m_chestOpened) return;
    m_chestOpened = true;

    // Stop sparkle animations — remove sparkle particles
    for (auto* p : m_sparkleParticles) {
        m_scene->removeItem(p);
        delete p;
    }
    m_sparkleParticles.clear();

    // Generate random relic
    m_offeredRelic = RelicFactory::generateRandomRelic();
    if (!m_offeredRelic) return;

    // Swap chest image to open state
    if (!m_chestOpenPixmap.isNull()) {
        auto scaled = m_chestOpenPixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_chestImage->setPixmap(scaled);
    }

    // Show relic popup after a short delay for the "open" feel
    QTimer::singleShot(500, this, [this]() {
        m_relicPopup = new RelicPopupWidget(m_offeredRelic);
        m_scene->addItem(m_relicPopup);

        connect(m_relicPopup, &RelicPopupWidget::takeClicked, this, [this]() {
            onTakeRelic(m_offeredRelic);
        });
        connect(m_relicPopup, &RelicPopupWidget::skipClicked, this, [this]() {
            onSkipRelic();
        });
    });
}
```

- [ ] **Step 4: Write ChestView — take/skip handlers and result**

```cpp
void ChestView::onTakeRelic(Relic* relic) {
    // Remove popup
    if (m_relicPopup) {
        m_scene->removeItem(m_relicPopup);
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }

    // Add relic to manager
    m_relicManager->addRelic(relic);

    showResult();
}

void ChestView::onSkipRelic() {
    // Remove popup, discard relic
    if (m_relicPopup) {
        m_scene->removeItem(m_relicPopup);
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }

    if (m_offeredRelic) {
        delete m_offeredRelic;
        m_offeredRelic = nullptr;
    }

    showResult();
}

void ChestView::showResult() {
    // Show "前进" leave button
    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
        m_leaveBtn->show();
    }
}
```

- [ ] **Step 5: Override mousePressEvent to detect chest clicks**

Add to `ChestView.h` protected section:
```cpp
void mousePressEvent(QMouseEvent* event) override;
```

Add to `ChestView.cpp`:
```cpp
void ChestView::mousePressEvent(QMouseEvent* event) {
    // Convert viewport coords to scene coords
    QPointF scenePt = mapToScene(event->pos());

    // Chest hit area (approximate)
    QRectF chestRect(1300, 580, 300, 300);
    if (!m_chestOpened && chestRect.contains(scenePt)) {
        onChestClicked();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}
```

Add include to ChestView.cpp:
```cpp
#include <QMouseEvent>
```

- [ ] **Step 6: Commit**

```bash
git add src/ui/events/ChestView.h src/ui/events/ChestView.cpp
git commit -m "feat: add ChestView with chest click, sparkle effects, and relic popup flow"
```

---

### Task 4: Wire EventLauncher::launchChest

**Files:**
- Modify: `src/api/EventLauncher.cpp:93-99`

- [ ] **Step 1: Replace the stub implementation**

Replace the existing `launchChest` body (lines 93-99) with:
```cpp
void EventLauncher::launchChest(Player* player, RelicManager* relicManager,
                                 const EventContext& context) {
    m_view = new ChestView(player, relicManager);

    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });

    m_view->show();
}
```

Add include at top of EventLauncher.cpp:
```cpp
#include "../ui/events/ChestView.h"
```

- [ ] **Step 2: Commit**

```bash
git add src/api/EventLauncher.cpp
git commit -m "feat: wire ChestView into EventLauncher"
```

---

### Task 5: Update build files

**Files:**
- Modify: `SlayTheQt.pro`

- [ ] **Step 1: Add new source and header files to .pro**

Add to SOURCES:
```
    src/ui/events/ChestView.cpp \
    src/ui/events/RelicPopupWidget.cpp
```

Add to HEADERS:
```
    src/ui/events/ChestView.h \
    src/ui/events/RelicPopupWidget.h
```

- [ ] **Step 2: Commit**

```bash
git add SlayTheQt.pro
git commit -m "build: add ChestView and RelicPopupWidget to project"
```

---

### Task 6: Update main.cpp test entry

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Switch test entry from Campfire to Chest**

Replace the current test setup (lines 29-68) with:
```cpp
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ==========================================
    // 宝箱事件测试入口
    // ==========================================
    EventContext context;
    context.currentHp = 60;
    context.maxHp = 80;
    context.gold = 300;
    context.maxEnergy = 3;
    context.eventType = EventType::Chest;

    // Fill some relics for the RelicTray
    context.relics.append(new PenNibRelic());
    context.relics.append(new OrichalcumRelic());

    EventLauncher* launcher = new EventLauncher();

    QObject::connect(launcher, &EventLauncher::eventConcluded, [](EventResult result) {
        qDebug() << "===========================================";
        qDebug() << "[Test] Chest Event Concluded!";
        qDebug() << "Remaining HP:" << result.remainingHp;
        qDebug() << "Current Gold:" << result.currentGold;
        qDebug() << "Relics count:" << result.resultRelics.size();
        qDebug() << "Relics changed:" << result.relicsChanged;
        qDebug() << "===========================================";
    });

    launcher->launch(context);

    return a.exec();
}
```

Remove the unused StrikeCard, DefendCard, BashCard includes (no longer needed for Chest test).

- [ ] **Step 2: Commit**

```bash
git add src/main.cpp
git commit -m "test: switch main.cpp test entry from Campfire to Chest event"
```

---

### Task 7: Build and verify

- [ ] **Step 1: Build the project**

```bash
cd "E:/Badstuff/Schoolwork/Coding/Programming_Internship/SlayTheQt - 协作副本/PKU2026-SlayTheQt"
qmake && make -j$(nproc)
```

Expected: Build succeeds with zero errors.

- [ ] **Step 2: Run the application to verify visual output**

```bash
./SlayTheQt  # or the actual executable name
```

Verify:
- [ ] Dungeon background visible
- [ ] Player on left side (battle perspective)
- [ ] Crystal chest on right side with sparkle particles
- [ ] TopBar shows player name, HP, gold
- [ ] RelicTray shows existing relics
- [ ] Clicking chest removes sparkles, opens chest, shows relic popup
- [ ] Relic popup shows relic name and description
- [ ] Hover over relic icon shows tooltip with full description
- [ ] "拾取" button adds relic to RelicTray
- [ ] "跳过" button discards relic
- [ ] After take/skip, "前进" button appears at bottom right
- [ ] Clicking "前进" emits eventFinished and closes

- [ ] **Step 3: Fix any issues found, then commit**

```bash
git add -A
git commit -m "fix: address visual and interaction issues from manual testing"
```
