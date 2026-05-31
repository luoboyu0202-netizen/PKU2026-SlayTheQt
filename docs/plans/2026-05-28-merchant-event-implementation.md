# Merchant Event Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use ultrapower:executing-plans to implement this plan task-by-task.

**Goal:** Implement the MerchantView with two-phase flow (encounter → shopping), card/relic purchasing with glow-trail animation, hand cursor that only appears when hovering purchasable items, scale-emphasis instead of highlight, and card removal service.

**Architecture:** Create MerchantView inheriting EventBaseView + ShopCardItem widget. Phase 1: player left, merchant right, click to transition. Phase 2: carpet backdrop, 7 cards (5+2 grid), 3 relics (rightmost = shop-exclusive), card removal (one-shot, escalating cost across shops), exit banner. Key interactions: hand cursor animates to hovered item edge then retreats off-screen; hovered items scale up (not highlight); purchased items turn into glowing orb with trail flying to top-right corner; post-purchase slots are completely empty (except remove→soldout).

**Tech Stack:** Qt 6 C++17, QGraphicsView/QGraphicsScene, qmake

---

### Task 1: Add Merchant resources to resources.qrc

**Files:**
- Modify: `resources.qrc`

**Step 1: Add all merchant images**

Add these lines before `</qresource>` in `resources.qrc`:

```xml
<file>resources/images/events/Merchant/Merchant.png</file>
<file>resources/images/events/Merchant/arm.jpeg</file>
<file>resources/images/events/Merchant/carpet.jpg</file>
<file>resources/images/events/Merchant/exit.jpg</file>
<file>resources/images/events/Merchant/label.jpg</file>
<file>resources/images/events/Merchant/remove.jpg</file>
<file>resources/images/events/Merchant/soldout.jpg</file>
```

**Step 2: Commit**

```bash
git add resources.qrc
git commit -m "feat: add merchant event resources to qrc"
```

---

### Task 2: Add cardRemovalCost to GlobalSaveData

**Files:**
- Modify: `src/logic/globalsavedata.h`

**Step 1: Add the field**

In the public section, after `int maxEnergy = 3;`:

```cpp
int cardRemovalCost = 75;
```

In `initNewGame()`, after `gold = 99;`:

```cpp
cardRemovalCost = 75;
```

**Step 2: Commit**

```bash
git add src/logic/globalsavedata.h
git commit -m "feat: add cardRemovalCost to GlobalSaveData for merchant"
```

---

### Task 3: Create ShopCardItem widget (scale-emphasis, no highlight)

**Files:**
- Create: `src/ui/events/ShopCardItem.h`
- Create: `src/ui/events/ShopCardItem.cpp`

**Step 1: Write ShopCardItem.h**

```cpp
#pragma once
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include "../../entities/cards/card.h"

class ShopCardItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
public:
    explicit ShopCardItem(Card* card, int price, QGraphicsItem* parent = nullptr);

    Card* card() const { return m_card; }
    int price() const { return m_price; }
    int originalPrice() const { return m_originalPrice; }
    void setAffordable(bool canAfford);
    void setOnSale(bool onSale);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // For hand cursor positioning
    QPointF topEdgeCenter() const;

signals:
    void clicked(Card* card, int price);
    void hovered(ShopCardItem* item);   // emitted when mouse enters
    void unhovered(ShopCardItem* item); // emitted when mouse leaves

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Card* m_card;
    int m_price;
    int m_originalPrice;
    bool m_affordable = true;
    bool m_onSale = false;
    QPropertyAnimation* m_scaleAnim = nullptr;
};
```

**Step 2: Write ShopCardItem.cpp**

```cpp
#include "ShopCardItem.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

ShopCardItem::ShopCardItem(Card* card, int price, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_card(card), m_price(price), m_originalPrice(price) {
    setAcceptHoverEvents(true);
}

void ShopCardItem::setAffordable(bool canAfford) {
    m_affordable = canAfford;
    update();
}

void ShopCardItem::setOnSale(bool onSale) {
    m_onSale = onSale;
    if (onSale) m_price = m_originalPrice / 2;
    update();
}

QPointF ShopCardItem::topEdgeCenter() const {
    // Return center of top edge in scene coordinates
    return mapToScene(QPointF(0, boundingRect().top()));
}

QRectF ShopCardItem::boundingRect() const {
    return QRectF(-85, -120, 170, 260);
}

void ShopCardItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Card background (type-colored, NO highlight tint — emphasis is via scale)
    QColor bg;
    switch (m_card->getType()) {
    case CardType::Attack: bg = QColor(140, 30, 30); break;
    case CardType::Skill:  bg = QColor(30, 100, 30); break;
    case CardType::Power:  bg = QColor(30, 30, 140); break;
    default: bg = QColor(60, 60, 60); break;
    }
    painter->setBrush(bg);
    QColor borderColor = m_affordable ? Qt::white : QColor(100, 100, 100);
    painter->setPen(QPen(borderColor, 2));
    painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1), 10, 10);

    // Card image
    QString imgPath = m_card->getImagePath();
    if (!imgPath.isEmpty()) {
        QPixmap cardImg(imgPath);
        if (!cardImg.isNull()) {
            painter->drawPixmap(QRect(-70, -105, 140, 100),
                cardImg.scaled(140, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    // Name
    QFont nameFont("Microsoft YaHei", 11, QFont::Bold);
    painter->setFont(nameFont);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(-70, -15, 140, 25), Qt::AlignCenter, m_card->getName());

    // Cost
    QFont costFont("Microsoft YaHei", 13);
    painter->setFont(costFont);
    painter->setPen(QColor(255, 220, 100));
    painter->drawText(QRectF(-70, 10, 140, 20), Qt::AlignCenter,
                      QString::number(m_card->getCost()) + " 费");

    // Description (truncated)
    QFont descFont("Microsoft YaHei", 8);
    painter->setFont(descFont);
    painter->setPen(QColor(180, 180, 180));
    QString desc = m_card->getDescription();
    if (desc.length() > 40) desc = desc.left(38) + "...";
    painter->drawText(QRectF(-70, 35, 140, 50), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, desc);

    // Price tag background
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(30, 30, 30, 220));
    painter->drawRoundedRect(QRectF(-50, 95, 100, 22), 4, 4);

    // Price text
    QFont priceFont("Microsoft YaHei", 12, QFont::Bold);
    painter->setFont(priceFont);
    painter->setPen(m_affordable ? QColor(255, 215, 0) : QColor(150, 150, 150));

    if (m_onSale) {
        QFont smallFont("Microsoft YaHei", 9);
        painter->setFont(smallFont);
        painter->setPen(QColor(200, 80, 80));
        painter->drawText(QRectF(-50, 97, 100, 10), Qt::AlignCenter,
                          QString::number(m_originalPrice) + "g");
        painter->setFont(priceFont);
        painter->setPen(QColor(255, 215, 0));
        painter->drawText(QRectF(-50, 107, 100, 12), Qt::AlignCenter,
                          QString::number(m_price) + "g");
    } else {
        painter->drawText(QRectF(-50, 97, 100, 19), Qt::AlignCenter,
                          QString::number(m_price) + "g");
    }
}

void ShopCardItem::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    // Scale up animation (1.0 → 1.15) — emphasis via size, not highlight color
    if (m_scaleAnim) { m_scaleAnim->stop(); delete m_scaleAnim; }
    m_scaleAnim = new QPropertyAnimation(this, "scale", this);
    m_scaleAnim->setDuration(150);
    m_scaleAnim->setStartValue(scale());
    m_scaleAnim->setEndValue(1.15);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

    emit hovered(this);
}

void ShopCardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    // Scale back down (1.15 → 1.0)
    if (m_scaleAnim) { m_scaleAnim->stop(); delete m_scaleAnim; }
    m_scaleAnim = new QPropertyAnimation(this, "scale", this);
    m_scaleAnim->setDuration(150);
    m_scaleAnim->setStartValue(scale());
    m_scaleAnim->setEndValue(1.0);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

    emit unhovered(this);
}

void ShopCardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_affordable)
        emit clicked(m_card, m_price);
}
```

**Step 3: Commit**

```bash
git add src/ui/events/ShopCardItem.h src/ui/events/ShopCardItem.cpp
git commit -m "feat: add ShopCardItem with scale-emphasis and hover signals"
```

---

### Task 4: Create MerchantView header

**Files:**
- Create: `src/ui/events/MerchantView.h`

**Step 1: Write MerchantView.h**

```cpp
#pragma once
#include "EventBaseView.h"
#include <QGraphicsPixmapItem>
#include <QList>
#include <QPropertyAnimation>

class Card;
class Relic;
class ShopCardItem;

class MerchantView : public EventBaseView {
    Q_OBJECT

public:
    explicit MerchantView(Player* player, CardManager* cardManager,
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    // Phase 1
    void setupPhaseOne();
    void onMerchantClicked();

    // Phase 2
    void setupPhaseTwo();
    void generateShopItems();
    void layoutShopItems();
    void refreshAffordability();

    // Hand cursor
    void updateHandCursor(const QPointF& mouseScenePos);
    void moveHandToItem(ShopCardItem* item);
    void moveHandOffScreen();
    ShopCardItem* findHoveredCardItem(const QPointF& scenePos);

    // Purchases
    void onCardClicked(Card* card, int price);
    void onRemoveClicked();
    void onExitClicked();

    // Purchase animation
    void playPurchaseEffect(QGraphicsObject* item);

    // Phase state
    enum class Phase { Encounter, Shopping };
    Phase m_phase = Phase::Encounter;

    // Phase 1
    QGraphicsPixmapItem* m_bgItem = nullptr;
    QGraphicsPixmapItem* m_merchantImage = nullptr;

    // Phase 2
    QGraphicsPixmapItem* m_carpet = nullptr;
    QGraphicsPixmapItem* m_handCursor = nullptr;
    QGraphicsPixmapItem* m_exitBanner = nullptr;
    QGraphicsPixmapItem* m_removeButton = nullptr;
    QGraphicsPixmapItem* m_saleTag = nullptr;

    QPropertyAnimation* m_handAnimX = nullptr;
    QPropertyAnimation* m_handAnimY = nullptr;

    // Shop items
    ShopCardItem* m_cardSlots[7] = {};
    Card* m_shopCards[7] = {};
    int m_cardPrices[7] = {};
    int m_saleIndex = -1;

    QGraphicsPixmapItem* m_relicIcons[3] = {};
    QGraphicsTextItem* m_relicPriceTexts[3] = {};
    QGraphicsTextItem* m_relicNameTexts[3] = {};
    Relic* m_shopRelics[3] = {};
    int m_relicPrices[3] = {};

    bool m_cardRemoved = false;
    ShopCardItem* m_currentHoveredCard = nullptr;
};
```

**Step 2: Commit**

```bash
git add src/ui/events/MerchantView.h
git commit -m "feat: add MerchantView header with hand cursor and animation state"
```

---

### Task 5: Create MerchantView - Phase 1

**Files:**
- Create: `src/ui/events/MerchantView.cpp`

**Step 1: Write constructor, setupContent, Phase 1, and click handling**

```cpp
#include "MerchantView.h"
#include "ShopCardItem.h"
#include "../../entities/Player.h"
#include "../../entities/cards/card.h"
#include "../../entities/relics/Relic.h"
#include "../../entities/relics/RelicManager.h"
#include "../../logic/CardManager.h"
#include "../../logic/CardFactory.h"
#include "../../logic/RelicFactory.h"
#include "../../logic/globalsavedata.h"
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QDebug>
#include <cmath>

MerchantView::MerchantView(Player* player, CardManager* cardManager,
                           RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void MerchantView::setupContent() {
    setupPhaseOne();
}

void MerchantView::setupPhaseOne() {
    // Background (reuse Chest background)
    QPixmap bg(":/resources/images/events/Chest/Background.png");
    if (!bg.isNull()) {
        m_bgItem = new QGraphicsPixmapItem();
        m_bgItem->setPixmap(bg.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_bgItem->setPos(0, 0);
        m_bgItem->setZValue(-10);
        m_scene->addItem(m_bgItem);
    }

    // Player on left
    if (m_playerImage) {
        m_playerImage->setPixmap(m_playerPixmap.scaled(450, 675, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_playerImage->setPos(100, 450);
        m_playerImage->setZValue(10);
    }

    // Merchant on right
    QPixmap merchantPix(":/resources/images/events/Merchant/Merchant.png");
    if (!merchantPix.isNull()) {
        m_merchantImage = new QGraphicsPixmapItem();
        m_merchantImage->setPixmap(merchantPix.scaled(500, 650, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_merchantImage->setPos(1200, 420);
        m_merchantImage->setZValue(15);
        m_scene->addItem(m_merchantImage);
    }

    if (m_leaveBtn) m_leaveBtn->hide();
}

void MerchantView::mousePressEvent(QMouseEvent* event) {
    QPointF scenePt = mapToScene(event->pos());

    if (m_phase == Phase::Encounter) {
        QRectF merchantRect(1200, 420, 500, 650);
        if (merchantRect.contains(scenePt)) {
            onMerchantClicked();
            return;
        }
    }

    if (m_phase == Phase::Shopping) {
        // Exit banner
        if (m_exitBanner && m_exitBanner->isVisible()) {
            QRectF exitRect = m_exitBanner->sceneBoundingRect();
            exitRect.adjust(-15, -15, 15, 15);
            if (exitRect.contains(scenePt)) {
                onExitClicked();
                return;
            }
        }
        // Remove button
        if (m_removeButton && m_removeButton->isVisible()) {
            QRectF removeRect = m_removeButton->sceneBoundingRect();
            removeRect.adjust(-10, -10, 10, 10);
            if (removeRect.contains(scenePt)) {
                onRemoveClicked();
                return;
            }
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void MerchantView::onMerchantClicked() {
    qDebug() << "[MerchantView] Transitioning to shop phase";
    m_phase = Phase::Shopping;
    setupPhaseTwo();
}
```

**Step 2: Commit**

```bash
git add src/ui/events/MerchantView.cpp
git commit -m "feat: add MerchantView Phase 1 - encounter scene"
```

---

### Task 6: Create MerchantView - Phase 2 scene + hand cursor

**Files:**
- Modify: `src/ui/events/MerchantView.cpp` (append)

**Step 1: Add setupPhaseTwo() — scene elements**

```cpp
void MerchantView::setupPhaseTwo() {
    if (m_playerImage) m_playerImage->hide();
    if (m_merchantImage) m_merchantImage->hide();
    if (m_bgItem) m_bgItem->hide();

    // Carpet
    QPixmap carpetPix(":/resources/images/events/Merchant/carpet.jpg");
    if (!carpetPix.isNull()) {
        m_carpet = new QGraphicsPixmapItem();
        m_carpet->setPixmap(carpetPix.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_carpet->setPos(0, 0);
        m_carpet->setZValue(0);
        m_scene->addItem(m_carpet);
    }

    // Hand cursor (arm from above) — starts off-screen
    QPixmap armPix(":/resources/images/events/Merchant/arm.jpeg");
    m_handCursor = new QGraphicsPixmapItem();
    if (!armPix.isNull()) {
        // Scale arm to reasonable width (~120px), keep aspect ratio (image is elongated)
        m_handCursor->setPixmap(armPix.scaled(120, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_handCursor->setZValue(100);
    m_handCursor->setPos(-200, -500); // off-screen
    m_scene->addItem(m_handCursor);
    setMouseTracking(true);

    // Generate items
    generateShopItems();
    layoutShopItems();

    // Exit banner (left of second row)
    QPixmap exitPix(":/resources/images/events/Merchant/exit.jpg");
    if (!exitPix.isNull()) {
        m_exitBanner = new QGraphicsPixmapItem();
        m_exitBanner->setPixmap(exitPix.scaled(220, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_exitBanner->setPos(30, 680);
        m_exitBanner->setZValue(50);
        m_scene->addItem(m_exitBanner);
    }
}
```

**Step 2: Add mouseMoveEvent + hand cursor logic**

```cpp
void MerchantView::mouseMoveEvent(QMouseEvent* event) {
    if (m_phase != Phase::Shopping) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    QPointF scenePt = mapToScene(event->pos());
    updateHandCursor(scenePt);
    QGraphicsView::mouseMoveEvent(event);
}

void MerchantView::updateHandCursor(const QPointF& mouseScenePos) {
    ShopCardItem* hovered = findHoveredCardItem(mouseScenePos);

    if (hovered && hovered != m_currentHoveredCard) {
        m_currentHoveredCard = hovered;
        moveHandToItem(hovered);
    } else if (!hovered && m_currentHoveredCard) {
        m_currentHoveredCard = nullptr;
        moveHandOffScreen();
    }
}

ShopCardItem* MerchantView::findHoveredCardItem(const QPointF& scenePos) {
    for (int i = 0; i < 7; ++i) {
        if (m_cardSlots[i] && m_cardSlots[i]->isVisible()) {
            QRectF r = m_cardSlots[i]->sceneBoundingRect();
            if (r.contains(scenePos))
                return m_cardSlots[i];
        }
    }
    return nullptr;
}

void MerchantView::moveHandToItem(ShopCardItem* item) {
    if (!m_handCursor) return;

    if (m_handAnimX) { m_handAnimX->stop(); delete m_handAnimX; m_handAnimX = nullptr; }
    if (m_handAnimY) { m_handAnimY->stop(); delete m_handAnimY; m_handAnimY = nullptr; }

    // Arm enters from above; fingertip is at bottom of pixmap
    // Position: fingertip (bottom edge) = item's top edge, X = centered on item
    QPointF itemTop = item->topEdgeCenter();
    QPixmap armPm = m_handCursor->pixmap();
    qreal armH = armPm.isNull() ? 400 : armPm.height();
    qreal armW = armPm.isNull() ? 120 : armPm.width();

    qreal targetX = itemTop.x() - armW / 2;
    qreal targetY = itemTop.y() - armH; // bottom of arm at item's top

    m_handAnimX = new QPropertyAnimation(m_handCursor, "x", this);
    m_handAnimX->setDuration(200);
    m_handAnimX->setStartValue(m_handCursor->x());
    m_handAnimX->setEndValue(targetX);
    m_handAnimX->setEasingCurve(QEasingCurve::OutCubic);

    m_handAnimY = new QPropertyAnimation(m_handCursor, "y", this);
    m_handAnimY->setDuration(200);
    m_handAnimY->setStartValue(m_handCursor->y());
    m_handAnimY->setEndValue(targetY);
    m_handAnimY->setEasingCurve(QEasingCurve::OutCubic);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(m_handAnimX);
    group->addAnimation(m_handAnimY);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    m_handCursor->show();
}

void MerchantView::moveHandOffScreen() {
    if (!m_handCursor) return;

    if (m_handAnimX) { m_handAnimX->stop(); delete m_handAnimX; m_handAnimX = nullptr; }
    if (m_handAnimY) { m_handAnimY->stop(); delete m_handAnimY; m_handAnimY = nullptr; }

    QPixmap armPm = m_handCursor->pixmap();
    qreal armH = armPm.isNull() ? 400 : armPm.height();

    m_handAnimY = new QPropertyAnimation(m_handCursor, "y", this);
    m_handAnimY->setDuration(300);
    m_handAnimY->setStartValue(m_handCursor->y());
    m_handAnimY->setEndValue(-armH - 50); // fully above screen, sleeve out of view
    m_handAnimY->setEasingCurve(QEasingCurve::InCubic);

    connect(m_handAnimY, &QPropertyAnimation::finished, this, [this]() {
        if (m_handCursor && m_currentHoveredCard == nullptr)
            m_handCursor->hide();
    });
    m_handAnimY->start(QAbstractAnimation::DeleteWhenStopped);
}
```

**Step 3: Commit**

```bash
git add src/ui/events/MerchantView.cpp
git commit -m "feat: add MerchantView Phase 2 with contextual hand cursor"
```

---

### Task 7: Create MerchantView - Shop item generation & layout

**Files:**
- Modify: `src/ui/events/MerchantView.cpp` (append)

**Step 1: Add generateShopItems()**

```cpp
void MerchantView::generateShopItems() {
    auto* rng = QRandomGenerator::global();

    // Generate 7 random cards
    QList<QString> allCardIds = CardFactory::getAllAvailableCardIds();
    for (int i = 0; i < allCardIds.size(); ++i)
        allCardIds.swapItemsAt(i, rng->bounded(allCardIds.size()));
    int cardCount = qMin(7, allCardIds.size());

    for (int i = 0; i < cardCount; ++i) {
        m_shopCards[i] = CardFactory::createCard(allCardIds[i], this);
        int basePrice = 50 + rng->bounded(120);
        double jitter = 0.9 + rng->bounded(21) / 100.0;
        m_cardPrices[i] = qRound(basePrice * jitter);
    }
    if (cardCount > 0)
        m_saleIndex = rng->bounded(cardCount);

    // Generate 3 relics
    QList<QString> allRelicIds = RelicFactory::getAllAvailableRelicIds();
    for (int i = 0; i < allRelicIds.size(); ++i)
        allRelicIds.swapItemsAt(i, rng->bounded(allRelicIds.size()));
    int relicCount = qMin(3, allRelicIds.size());

    for (int i = 0; i < relicCount; ++i) {
        m_shopRelics[i] = RelicFactory::createRelic(allRelicIds[i], this);
        m_relicPrices[i] = 143 + rng->bounded(15);
    }
}
```

**Step 2: Add layoutShopItems()**

```cpp
void MerchantView::layoutShopItems() {
    const qreal row1Y = 250, row2Y = 520;
    const qreal startX1 = 160, cardSpacingX = 195;
    const qreal relicStartX = startX1 + 2 * cardSpacingX + 30;
    const qreal relicSpacing = 160, relicY = row2Y + 30;

    // Row 1: cards 0-4
    for (int i = 0; i < 5 && m_shopCards[i]; ++i) {
        m_cardSlots[i] = new ShopCardItem(m_shopCards[i], m_cardPrices[i]);
        m_cardSlots[i]->setPos(startX1 + i * cardSpacingX, row1Y);
        m_cardSlots[i]->setZValue(10);
        m_scene->addItem(m_cardSlots[i]);
        connect(m_cardSlots[i], &ShopCardItem::clicked, this, &MerchantView::onCardClicked);
    }

    // Row 2: cards 5-6
    for (int i = 5; i < 7 && m_shopCards[i]; ++i) {
        int col = i - 5;
        m_cardSlots[i] = new ShopCardItem(m_shopCards[i], m_cardPrices[i]);
        m_cardSlots[i]->setPos(startX1 + col * cardSpacingX, row2Y);
        m_cardSlots[i]->setZValue(10);
        m_scene->addItem(m_cardSlots[i]);
        connect(m_cardSlots[i], &ShopCardItem::clicked, this, &MerchantView::onCardClicked);
    }

    // Sale tag
    if (m_saleIndex >= 0 && m_cardSlots[m_saleIndex]) {
        m_cardSlots[m_saleIndex]->setOnSale(true);
        QPixmap labelPix(":/resources/images/events/Merchant/label.jpg");
        if (!labelPix.isNull()) {
            m_saleTag = new QGraphicsPixmapItem(m_cardSlots[m_saleIndex]);
            m_saleTag->setPixmap(labelPix.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_saleTag->setPos(50, -110);
            m_saleTag->setZValue(1);
        }
    }

    // Relics
    for (int i = 0; i < 3 && m_shopRelics[i]; ++i) {
        // Icon
        QPixmap relicIcon(QString(":/resources/images/relics/%1.png").arg(m_shopRelics[i]->getId()));
        m_relicIcons[i] = new QGraphicsPixmapItem();
        if (!relicIcon.isNull())
            m_relicIcons[i]->setPixmap(relicIcon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_relicIcons[i]->setPos(relicStartX + i * relicSpacing, relicY);
        m_relicIcons[i]->setZValue(10);
        m_scene->addItem(m_relicIcons[i]);

        // Price
        m_relicPriceTexts[i] = m_scene->addText(QString::number(m_relicPrices[i]) + "g",
            QFont("Microsoft YaHei", 12, QFont::Bold));
        m_relicPriceTexts[i]->setDefaultTextColor(QColor(255, 215, 0));
        m_relicPriceTexts[i]->setPos(relicStartX + i * relicSpacing + 10, relicY + 70);
        m_relicPriceTexts[i]->setZValue(10);

        // Name
        m_relicNameTexts[i] = m_scene->addText(m_shopRelics[i]->getName(),
            QFont("Microsoft YaHei", 9));
        m_relicNameTexts[i]->setDefaultTextColor(QColor(200, 200, 200));
        m_relicNameTexts[i]->setPos(relicStartX + i * relicSpacing - 5, relicY + 95);
        m_relicNameTexts[i]->setZValue(10);
    }

    // Remove button
    QPixmap removePix(":/resources/images/events/Merchant/remove.jpg");
    if (!removePix.isNull()) {
        m_removeButton = new QGraphicsPixmapItem();
        m_removeButton->setPixmap(removePix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_removeButton->setPos(relicStartX + 3 * relicSpacing + 50, relicY - 10);
        m_removeButton->setZValue(10);
        m_scene->addItem(m_removeButton);
    }

    refreshAffordability();
}
```

**Step 3: Commit**

```bash
git add src/ui/events/MerchantView.cpp
git commit -m "feat: add MerchantView shop item generation and grid layout"
```

---

### Task 8: Create MerchantView - Purchase + animation + cleanup

**Files:**
- Modify: `src/ui/events/MerchantView.cpp` (append)

**Step 1: Add purchase flow with glow-trail animation**

```cpp
void MerchantView::refreshAffordability() {
    int gold = m_player->getGold();
    for (int i = 0; i < 7; ++i)
        if (m_cardSlots[i])
            m_cardSlots[i]->setAffordable(gold >= m_cardPrices[i]);
}

void MerchantView::onCardClicked(Card* card, int price) {
    if (m_player->getGold() < price) return;

    // Find which slot
    int slot = -1;
    for (int i = 0; i < 7; ++i) {
        if (m_cardSlots[i] && m_cardSlots[i]->card() == card) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    m_player->modifyGold(-price);
    m_cardManager->getDrawPile().append(card);

    // Play glow-trail fly-away animation, then remove from scene
    playPurchaseEffect(m_cardSlots[slot]);
    // Remove immediately (effect is visual-only, slot cleared)
    QTimer::singleShot(800, this, [this, slot]() {
        if (m_cardSlots[slot]) {
            m_scene->removeItem(m_cardSlots[slot]);
            delete m_cardSlots[slot];
            m_cardSlots[slot] = nullptr;
        }
        refreshAffordability();
    });

    qDebug() << "[MerchantView] Purchased card:" << card->getName() << "for" << price;
}

void MerchantView::playPurchaseEffect(QGraphicsObject* item) {
    // Create a glowing orb that flies to top-right corner with trail effect
    QPointF startPos = item->scenePos();
    QPointF endPos(2000, -200);

    // Glow orb
    auto* glowOrb = new QGraphicsEllipseItem(-10, -10, 20, 20);
    glowOrb->setBrush(QColor(255, 255, 200, 240));
    glowOrb->setPen(Qt::NoPen);
    glowOrb->setPos(startPos);
    glowOrb->setZValue(150);
    m_scene->addItem(glowOrb);

    // Trail particles (3-5 smaller fading orbs that follow)
    QList<QGraphicsEllipseItem*> trails;
    for (int t = 0; t < 4; ++t) {
        auto* trail = new QGraphicsEllipseItem(-5, -5, 10, 10);
        trail->setBrush(QColor(200, 200, 150, 180 - t * 40));
        trail->setPen(Qt::NoPen);
        trail->setPos(startPos);
        trail->setZValue(148 - t);
        m_scene->addItem(trail);
        trails.append(trail);
    }

    // Position animation for glow orb
    auto* orbAnimX = new QPropertyAnimation(glowOrb, "x", this);
    orbAnimX->setDuration(600);
    orbAnimX->setStartValue(startPos.x());
    orbAnimX->setEndValue(endPos.x());
    orbAnimX->setEasingCurve(QEasingCurve::InQuad);

    auto* orbAnimY = new QPropertyAnimation(glowOrb, "y", this);
    orbAnimY->setDuration(600);
    orbAnimY->setStartValue(startPos.y());
    orbAnimY->setEndValue(endPos.y());
    orbAnimY->setEasingCurve(QEasingCurve::InQuad);

    // Scale down as it flies away
    auto* orbAnimScale = new QPropertyAnimation(glowOrb, "scale", this);
    orbAnimScale->setDuration(600);
    orbAnimScale->setStartValue(1.0);
    orbAnimScale->setEndValue(0.3);

    auto* group = new QParallelAnimationGroup(this);
    group->addAnimation(orbAnimX);
    group->addAnimation(orbAnimY);
    group->addAnimation(orbAnimScale);

    // Trail follow with staggered delay
    for (int t = 0; t < trails.size(); ++t) {
        QTimer::singleShot(50 + t * 60, this, [this, trails, t, startPos, endPos]() {
            auto* tx = new QPropertyAnimation(trails[t], "x", this);
            tx->setDuration(500);
            tx->setStartValue(startPos.x());
            tx->setEndValue(endPos.x());
            tx->setEasingCurve(QEasingCurve::InQuad);

            auto* ty = new QPropertyAnimation(trails[t], "y", this);
            ty->setDuration(500);
            ty->setStartValue(startPos.y());
            ty->setEndValue(endPos.y());
            ty->setEasingCurve(QEasingCurve::InQuad);

            auto* tg = new QParallelAnimationGroup(this);
            tg->addAnimation(tx);
            tg->addAnimation(ty);
            connect(tg, &QAnimationGroup::finished, this, [this, trails, t]() {
                m_scene->removeItem(trails[t]);
                delete trails[t];
            });
            tg->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    connect(group, &QAnimationGroup::finished, this, [this, glowOrb]() {
        m_scene->removeItem(glowOrb);
        delete glowOrb;
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MerchantView::onRemoveClicked() {
    if (m_cardRemoved) return;

    int cost = GlobalSaveData::getInstance()->cardRemovalCost;
    if (m_player->getGold() < cost) return;

    // Remove first card from draw/hand/discard (simplified; future: selection UI)
    QList<Card*> removable;
    removable.append(m_cardManager->getDrawPile());
    removable.append(m_cardManager->getHand());
    removable.append(m_cardManager->getDiscardPile());
    if (removable.isEmpty()) return;

    Card* toRemove = removable.first();
    if (m_cardManager->getDrawPile().contains(toRemove))
        m_cardManager->getDrawPile().removeOne(toRemove);
    else if (m_cardManager->getHand().contains(toRemove))
        m_cardManager->getHand().removeOne(toRemove);
    else if (m_cardManager->getDiscardPile().contains(toRemove))
        m_cardManager->getDiscardPile().removeOne(toRemove);

    m_player->modifyGold(-cost);
    GlobalSaveData::getInstance()->cardRemovalCost += 25;

    // Replace remove button with soldout (ONLY placeholder in the shop)
    if (m_removeButton) {
        m_removeButton->hide();
        QPixmap soldoutPix(":/resources/images/events/Merchant/soldout.jpg");
        if (!soldoutPix.isNull()) {
            auto* soldout = new QGraphicsPixmapItem();
            soldout->setPixmap(soldoutPix.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            soldout->setPos(m_removeButton->pos());
            soldout->setZValue(10);
            m_scene->addItem(soldout);
        }
    }
    m_cardRemoved = true;
    refreshAffordability();
    qDebug() << "[MerchantView] Removed card for" << cost << "gold";
}

void MerchantView::onExitClicked() {
    qDebug() << "[MerchantView] Exit banner clicked";
    emit eventFinished();
}
```

**Step 2: Commit**

```bash
git add src/ui/events/MerchantView.cpp
git commit -m "feat: add purchase animation, card removal, and exit logic"
```

---

### Task 9: Wire MerchantView into EventLauncher

**Files:**
- Modify: `src/api/EventLauncher.cpp`

**Step 1: Add include and replace stub**

Add at top:
```cpp
#include "../ui/events/MerchantView.h"
```

Replace `launchMerchant` body with:
```cpp
void EventLauncher::launchMerchant(Player* player, CardManager* cardManager,
                                    RelicManager* relicManager, const EventContext& context) {
    m_view = new MerchantView(player, cardManager, relicManager);

    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        result.deckChanged = true;
        result.goldChanged = true;
        result.relicsChanged = true;
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });

    m_view->show();
}
```

**Step 2: Commit**

```bash
git add src/api/EventLauncher.cpp
git commit -m "feat: wire MerchantView into EventLauncher"
```

---

### Task 10: Update SlayTheQt.pro

**Files:**
- Modify: `SlayTheQt.pro`

**Step 1: Register new files**

In `SOURCES`, add:
```
src/ui/events/MerchantView.cpp \
src/ui/events/ShopCardItem.cpp
```

In `HEADERS`, add:
```
src/ui/events/MerchantView.h \
src/ui/events/ShopCardItem.h
```

**Step 2: Commit**

```bash
git add SlayTheQt.pro
git commit -m "feat: register MerchantView and ShopCardItem in build"
```

---

### Task 11: Update main.cpp for testing

**Files:**
- Modify: `src/main.cpp`

**Step 1: Add Merchant test block**

Copy the existing Chest/Campfire test block pattern and replace with:
```cpp
#if 1  // Test: Merchant
    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->initNewGame();
    save->gold = 200;

    EventContext ctx;
    ctx.eventType = EventType::Merchant;
    ctx.currentHp = save->currentHp;
    ctx.maxHp = save->maxHp;
    ctx.gold = save->gold;
    ctx.maxEnergy = save->maxEnergy;
    for (const QString& id : save->deckIds) {
        Card* c = CardFactory::createCard(id);
        if (c) ctx.currentDeck.append(c);
    }
    for (const QString& id : save->relicIds) {
        Relic* r = RelicFactory::createRelic(id);
        if (r) ctx.relics.append(r);
    }
    EventLauncher* launcher = new EventLauncher();
    QObject::connect(launcher, &EventLauncher::eventConcluded, [&app](EventResult result) {
        qDebug() << "Merchant done. Gold:" << result.currentGold
                 << "Cards:" << result.resultDeck.size();
        app.quit();
    });
    launcher->launch(ctx);
#endif
```

**Step 2: Commit**

```bash
git add src/main.cpp
git commit -m "feat: add Merchant event test entry in main.cpp"
```

---

### Task 12: Build and verify

**Step 1: Build**

```bash
cd build && qmake ../SlayTheQt.pro && make -j$(nproc) 2>&1
```
Expected: Compilation succeeds, no errors.

**Step 2: Visual smoke test**

Run the binary and verify:
- [ ] Phase 1: Merchant shown on right, player on left
- [ ] Click merchant → Phase 2: carpet, 7 cards (5+2), 3 relics, remove, exit banner
- [ ] Hover card → card **scales up** (not color highlight)
- [ ] Hover card → hand cursor animates to card's top edge
- [ ] Mouse leaves card → hand cursor animates back off-screen
- [ ] One card has sale tag with strikethrough price
- [ ] Click card → glow orb flies to top-right with trail → card removed, slot empty
- [ ] Click remove → soldout placeholder replaces remove button
- [ ] Exit banner → event finishes

**Step 3: Commit any build fixes**

---

## Summary

| # | Task | Key Change |
|---|------|------------|
| 1 | resources.qrc | Add 6 merchant images |
| 2 | globalsavedata.h | cardRemovalCost field |
| 3 | ShopCardItem | Scale-emphasis (1.0→1.15), hover/unhover signals |
| 4 | MerchantView.h | Two-phase, hand cursor animation state |
| 5 | Phase 1 | Encounter: player + merchant + background |
| 6 | Phase 2 + Hand | Carpet, contextual hand cursor (in→edge / out→offscreen) |
| 7 | Generation | 7 random cards + 3 relics, 5+2 grid layout |
| 8 | Interactions | Glow-orb fly-away purchase effect, remove→soldout, exit |
| 9 | EventLauncher | Replace stub with MerchantView |
| 10 | SlayTheQt.pro | Register new files |
| 11 | main.cpp | Test entry |
| 12 | Build | Compile and smoke test |

**Pending:** Hand cursor image asset (placeholder logic is implemented, ready to plug in).
