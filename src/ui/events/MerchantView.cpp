#include "MerchantView.h"
#include "../carditem.h"
#include "TextButton.h"
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
#include <QVariantAnimation>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include <QDir>
#include <QImage>
#include <QDebug>
#include <cmath>

namespace {
constexpr qreal kShopCardW = 170.0;
constexpr qreal kShopCardH = 230.0;
constexpr qreal kShopCardGap = 30.0;

QPixmap trimTransparentPadding(const QPixmap& pixmap) {
    if (pixmap.isNull() || !pixmap.hasAlphaChannel())
        return pixmap;

    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    QRect contentRect;
    for (int y = 0; y < image.height(); ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 8) {
                QRect px(x, y, 1, 1);
                contentRect = contentRect.isNull() ? px : contentRect.united(px);
            }
        }
    }

    if (contentRect.isNull())
        return pixmap;
    return QPixmap::fromImage(image.copy(contentRect));
}
}

// Helper: load pixmap from Qt resource path, fall back to filesystem
static QPixmap loadPixmap(const QString& resPath, const QString& fsPath) {
    QPixmap pm(resPath);
    if (!pm.isNull()) return pm;
    qDebug() << "[MerchantView] Resource load failed for" << resPath << ", trying filesystem:" << fsPath;
    pm.load(fsPath);
    if (!pm.isNull()) qDebug() << "[MerchantView] Filesystem load succeeded for" << fsPath;
    return pm;
}

MerchantView::MerchantView(Player* player, CardManager* cardManager,
                           RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void MerchantView::setupContent() {
    setupPhaseOne();
}

// ============================================================
// Phase 1: Encounter
// ============================================================

void MerchantView::setupPhaseOne() {
    QPixmap bg(":/resources/images/events/Chest/Background.png");
    if (!bg.isNull()) {
        m_bgItem = new QGraphicsPixmapItem();
        m_bgItem->setPixmap(bg.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_bgItem->setPos(0, 0);
        m_bgItem->setZValue(-10);
        m_scene->addItem(m_bgItem);
    }

    if (m_playerImage) {
        m_playerImage->setPixmap(m_playerPixmap.scaled(450, 675, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_playerImage->setPos(100, 450);
        m_playerImage->setZValue(10);
    }

    QPixmap merchantPix = loadPixmap(":/resources/images/events/Merchant/merchant.png",
                                      "resources/images/events/Merchant/merchant.png");
    if (!merchantPix.isNull()) {
        m_merchantImage = new QGraphicsPixmapItem();
        m_merchantImage->setPixmap(merchantPix.scaled(500, 650, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_merchantImage->setPos(1200, 420);
        m_merchantImage->setZValue(15);
        m_scene->addItem(m_merchantImage);
    }

    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Merchant/GO_ahead.png");
        m_leaveBtn->show();
        // NOTE: EventBaseView already connects LeaveButton::clicked → eventFinished
    }
}

void MerchantView::onMerchantClicked() {
    qDebug() << "[MerchantView] Transitioning to shop phase";
    m_phase = Phase::Shopping;
    if (!m_shopGenerated) {
        setupPhaseTwo();
        m_shopGenerated = true;
    } else {
        // Re-show existing Phase 2 elements
        setupPhaseTwoReenter();
    }
}

// ============================================================
// Phase 2: Shopping
// ============================================================

void MerchantView::setupPhaseTwo() {
    if (m_playerImage) m_playerImage->hide();
    if (m_merchantImage) m_merchantImage->hide();
    if (m_bgItem) m_bgItem->hide();
    if (m_leaveBtn) m_leaveBtn->hide();

    QPixmap carpetPix = loadPixmap(":/resources/images/events/Merchant/carpet.png",
                                     "resources/images/events/Merchant/carpet.png");
    if (!carpetPix.isNull()) {
        m_carpet = new QGraphicsPixmapItem();
        m_carpet->setPixmap(carpetPix.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_carpet->setPos(0, 0);
        m_carpet->setZValue(-5);
        m_scene->addItem(m_carpet);
    }

    QPixmap armPix = loadPixmap(":/resources/images/events/Merchant/arm.png",
                                  "resources/images/events/Merchant/arm.png");
    m_handCursor = new QGraphicsPixmapItem();
    if (!armPix.isNull()) {
        m_handCursor->setPixmap(armPix.scaled(255, 750, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_handCursor->setZValue(120); // 🔴 手臂层级：刚好压过悬停卡牌 (110)
    m_handCursor->setPos(-200, -500);
    m_scene->addItem(m_handCursor);
    setMouseTracking(true);

    generateShopItems();
    layoutShopItems();

    QPixmap exitPix = loadPixmap(":/resources/images/events/Merchant/exit.png",
                                   "resources/images/events/Merchant/exit.png");
    if (!exitPix.isNull()) {
        m_exitBanner = new QGraphicsPixmapItem();
        m_exitBanner->setPixmap(exitPix.scaled(300, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_exitBanner->setPos(10, 670);
        m_exitBanner->setZValue(50);
        m_scene->addItem(m_exitBanner);
    }
}

void MerchantView::setupPhaseTwoReenter() {
    if (m_playerImage) m_playerImage->hide();
    if (m_merchantImage) m_merchantImage->hide();
    if (m_bgItem) m_bgItem->hide();
    if (m_leaveBtn) m_leaveBtn->hide();

    // Show existing Phase 2 elements (preserve shop state)
    if (m_carpet) m_carpet->show();
    if (m_exitBanner) m_exitBanner->show();
    if (m_removeButton && !m_cardRemoved) m_removeButton->show();
    if (m_soldoutItem && m_cardRemoved) m_soldoutItem->show();
    if (m_saleTag) m_saleTag->show();
    for (int i = 0; i < 7; ++i)
        if (m_cardSlots[i]) m_cardSlots[i]->show();
    for (int i = 0; i < 3; ++i) {
        if (m_relicIcons[i]) m_relicIcons[i]->show();
        if (m_relicPriceTexts[i]) m_relicPriceTexts[i]->show();
        if (m_relicNameTexts[i]) m_relicNameTexts[i]->show();
    }
    setMouseTracking(true);
    refreshAffordability();
}

// ============================================================
// Mouse events
// ============================================================

void MerchantView::mousePressEvent(QMouseEvent* event) {
    if (m_inRemovalMode) { QGraphicsView::mousePressEvent(event); return; }
    QPointF scenePt = mapToScene(event->pos());

    if (m_phase == Phase::Encounter) {
        // Check merchant click only if NOT on LeaveButton (they overlap)
        bool onLeaveBtn = m_leaveBtn && m_leaveBtn->isVisible()
                          && m_leaveBtn->sceneBoundingRect().contains(scenePt);
        if (!onLeaveBtn) {
            QRectF merchantRect(1200, 420, 500, 650);
            if (merchantRect.contains(scenePt)) {
                onMerchantClicked();
                return;
            }
        }
    }

    if (m_phase == Phase::Shopping) {
        if (m_exitBanner && m_exitBanner->isVisible()) {
            QRectF exitRect = m_exitBanner->sceneBoundingRect();
            exitRect.adjust(-15, -15, 15, 15);
            if (exitRect.contains(scenePt)) {
                onExitClicked();
                return;
            }
        }
        if (m_removeButton && m_removeButton->isVisible()) {
            QRectF removeRect = m_removeButton->sceneBoundingRect();
            removeRect.adjust(-10, -10, 10, 10);
            if (removeRect.contains(scenePt)) {
                onRemoveClicked();
                return;
            }
        }
        // Check relics
        for (int i = 0; i < 3; ++i) {
            if (m_relicIcons[i] && m_relicIcons[i]->isVisible()) {
                QRectF r = m_relicIcons[i]->sceneBoundingRect();
                r.adjust(-10, -10, 10, 80); // include price area
                if (r.contains(scenePt)) {
                    onRelicClicked(i);
                    return;
                }
            }
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void MerchantView::mouseMoveEvent(QMouseEvent* event) {
    if (m_inRemovalMode) { QGraphicsView::mouseMoveEvent(event); return; }
    if (m_phase == Phase::Shopping) {
        QPointF scenePt = mapToScene(event->pos());
        updateHandCursor(scenePt);
    }
    QGraphicsView::mouseMoveEvent(event);
}

// ============================================================
// Hand cursor
// ============================================================

void MerchantView::updateHandCursor(const QPointF& mouseScenePos) {
    QGraphicsItem* hovered = findHoveredItem(mouseScenePos);
    if (hovered && hovered != m_currentHoveredItem) {
        m_currentHoveredItem = hovered;
        moveHandToItem(hovered);

        // 🔴 如果悬停的是遗物，显示 Tooltip
        bool isRelic = false;
        for (int i = 0; i < 3; ++i) {
            if (hovered == m_relicIcons[i]) {
                showRelicTooltip(i);
                isRelic = true;
                break;
            }
        }
        if (!isRelic) hideRelicTooltip();

    } else if (!hovered && m_currentHoveredItem) {
        m_currentHoveredItem = nullptr;
        moveHandOffScreen();
        hideRelicTooltip();
    }
}

void MerchantView::showRelicTooltip(int index) {
    if (index < 0 || index >= 3 || !m_shopRelics[index]) return;
    Relic* r = m_shopRelics[index];

    hideRelicTooltip(); // 先清理旧的

    m_relicTooltipBg = new QGraphicsRectItem();
    m_relicTooltipText = new QGraphicsTextItem(m_relicTooltipBg);
    
    QString html = QString("<b style='color:#FFD700; font-size:16px;'>%1</b><br><br><span style='color:#FFFFFF; font-size:13px;'>%2</span>")
                   .arg(r->getName(), r->getDescription());
    m_relicTooltipText->setHtml(html);
    m_relicTooltipText->setTextWidth(250);

    qreal w = m_relicTooltipText->boundingRect().width() + 20;
    qreal h = m_relicTooltipText->boundingRect().height() + 20;
    
    m_relicTooltipBg->setRect(0, 0, w, h);
    m_relicTooltipBg->setBrush(QColor(30, 30, 35, 240));
    m_relicTooltipBg->setPen(QPen(QColor(180, 150, 100), 2));
    m_relicTooltipBg->setZValue(160); // 压过一切商店元素

    m_relicTooltipText->setPos(10, 10);

    // 定位在遗物上方
    QPointF relicPos = m_relicIcons[index]->scenePos();
    m_relicTooltipBg->setPos(relicPos.x() + 32 - w / 2, relicPos.y() - h - 20);

    m_scene->addItem(m_relicTooltipBg);
}

void MerchantView::hideRelicTooltip() {
    if (m_relicTooltipBg) {
        m_scene->removeItem(m_relicTooltipBg);
        delete m_relicTooltipBg;
        m_relicTooltipBg = nullptr;
        m_relicTooltipText = nullptr;
    }
}

QGraphicsItem* MerchantView::findHoveredItem(const QPointF& scenePos) {
    // Check cards
    for (int i = 0; i < 7; ++i) {
        // 🔴 增加 scene() 校验，确保它还在场景内
        if (m_cardSlots[i] && m_cardSlots[i]->isVisible() && m_cardSlots[i]->scene()) {
            QRectF r = m_cardSlots[i]->sceneBoundingRect();
            if (r.contains(scenePos))
                return m_cardSlots[i];
        }
    }
    // Check relics
    for (int i = 0; i < 3; ++i) {
        if (m_relicIcons[i] && m_relicIcons[i]->isVisible() && m_relicIcons[i]->scene()) {
            QRectF r = m_relicIcons[i]->sceneBoundingRect();
            r.adjust(-10, -10, 10, 10); // slightly larger hit area for relics
            if (r.contains(scenePos))
                return m_relicIcons[i];
        }
    }
    // Check removal button
    if (m_removeButton && m_removeButton->isVisible()) {
        QRectF r = m_removeButton->sceneBoundingRect();
        if (r.contains(scenePos))
            return m_removeButton;
    }
    return nullptr;
}

void MerchantView::moveHandToItem(QGraphicsItem* item) {
    if (!m_handCursor) return;

    // Stop old animations
    if (m_handAnimX) m_handAnimX->stop();
    if (m_handAnimY) m_handAnimY->stop();

    QRectF itemRect = item->sceneBoundingRect();
    QPointF itemTop(itemRect.center().x(), itemRect.top());

    QPixmap armPm = m_handCursor->pixmap();
    qreal armH = armPm.isNull() ? 750 : armPm.height();
    qreal armW = armPm.isNull() ? 255 : armPm.width();

    qreal startX = m_handCursor->x();
    qreal startY = m_handCursor->y();
    // Position arm so its tip is near the top center of the item
    qreal targetX = itemTop.x() - armW / 2;
    qreal targetY = itemTop.y() - armH + 10;

    m_handAnimX = new QVariantAnimation(this);
    m_handAnimX->setDuration(350); // 🔴 减缓至 350ms，动作更自然
    m_handAnimX->setStartValue(startX);
    m_handAnimX->setEndValue(targetX);
    m_handAnimX->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_handAnimX, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) { if (m_handCursor) m_handCursor->setX(v.toReal()); });
    connect(m_handAnimX, &QVariantAnimation::destroyed, this, [this]() { m_handAnimX = nullptr; });

    m_handAnimY = new QVariantAnimation(this);
    m_handAnimY->setDuration(350); // 🔴 同步减缓
    m_handAnimY->setStartValue(startY);
    m_handAnimY->setEndValue(targetY);
    m_handAnimY->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_handAnimY, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) { if (m_handCursor) m_handCursor->setY(v.toReal()); });
    connect(m_handAnimY, &QVariantAnimation::destroyed, this, [this]() { m_handAnimY = nullptr; });

    m_handAnimX->start(QAbstractAnimation::DeleteWhenStopped);
    m_handAnimY->start(QAbstractAnimation::DeleteWhenStopped);

    m_handCursor->show();
}

void MerchantView::moveHandOffScreen() {
    if (!m_handCursor) return;

    if (m_handAnimX) m_handAnimX->stop();
    if (m_handAnimY) m_handAnimY->stop();

    QPixmap armPm = m_handCursor->pixmap();
    qreal armH = armPm.isNull() ? 750 : armPm.height();

    m_handAnimY = new QVariantAnimation(this);
    m_handAnimY->setDuration(200); // 🔴 加速到 200ms
    m_handAnimY->setStartValue(m_handCursor->y());
    m_handAnimY->setEndValue(-armH - 100);
    m_handAnimY->setEasingCurve(QEasingCurve::OutQuad); // 🔴 使用更快的曲线立即起步
    connect(m_handAnimY, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) { if (m_handCursor) m_handCursor->setY(v.toReal()); });
    connect(m_handAnimY, &QVariantAnimation::destroyed, this, [this]() { m_handAnimY = nullptr; });
    connect(m_handAnimY, &QVariantAnimation::finished, this, [this]() {
        if (m_handCursor && m_currentHoveredItem == nullptr)
            m_handCursor->hide();
    });
    m_handAnimY->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================
// Shop generation
// ============================================================

void MerchantView::generateShopItems() {
    auto* rng = QRandomGenerator::global();

    QList<QString> allCardIds = CardFactory::getAllAvailableCardIds();
    for (int i = 0; i < allCardIds.size(); ++i)
        allCardIds.swapItemsAt(i, rng->bounded(allCardIds.size()));
    int cardCount = qMin(7, allCardIds.size());

    for (int i = 0; i < cardCount; ++i) {
        m_shopCards[i] = CardFactory::createCard(allCardIds[i], this);
        int minP = 45, maxP = 55;
        switch (m_shopCards[i]->getRarity()) {
            case CardRarity::Starter:
            case CardRarity::Common:   minP = 45;  maxP = 55;  break;
            case CardRarity::Uncommon: minP = 68;  maxP = 82;  break;
            case CardRarity::Rare:     minP = 135; maxP = 165; break;
            default:                   minP = 50;  maxP = 60;  break;
        }
        m_cardPrices[i] = minP + rng->bounded(maxP - minP + 1);
    }
    if (cardCount > 0) {
        m_saleIndex = rng->bounded(cardCount);
        m_cardPrices[m_saleIndex] /= 2;
    }

    QList<QString> allRelicIds = RelicFactory::getAllAvailableRelicIds();
    for (int i = 0; i < allRelicIds.size(); ++i)
        allRelicIds.swapItemsAt(i, rng->bounded(allRelicIds.size()));
    int relicCount = qMin(3, allRelicIds.size());

    for (int i = 0; i < relicCount; ++i) {
        m_shopRelics[i] = RelicFactory::createRelic(allRelicIds[i], this);
        m_relicPrices[i] = 143 + rng->bounded(15);
    }
}

void MerchantView::layoutShopItems() {
    // Centered layout: 5 cards ×185px + 30px gaps = 1045px → start at (1920-1045)/2
    const qreal row1Y = 380, row2Y = row1Y + kShopCardH + 90;
    const qreal startX1 = (1920.0 - (kShopCardW * 5 + kShopCardGap * 4)) / 2.0;
    const qreal cardSpacingX = kShopCardW + kShopCardGap;
    // Relics: 3 items to the right of 2nd-row cards
    const qreal relicStartX = startX1 + 2 * cardSpacingX + 60;
    const qreal relicSpacing = 140, relicY = row2Y - 10; // Moved up by 40px (was row2Y + 30)
    const qreal serviceX = relicStartX + 3 * relicSpacing + 60;
    const qreal serviceY = row2Y - kShopCardH / 2.0;
    const qreal removePriceX = serviceX + kShopCardH / 2 - 35;

    auto setupCard = [this](int i, qreal x, qreal y) {
        m_cardSlots[i] = new CardItem(m_shopCards[i]);
        m_cardSlots[i]->setPrice(m_cardPrices[i]);
        m_cardSlots[i]->setSelectionEnabled(true);
        int price = m_cardPrices[i];
        connect(m_cardSlots[i], &CardItem::cardClicked, this, [this, price](CardItem* item) {
            onCardClicked(item->getLogicCard(), price);
        });
        m_cardSlots[i]->setPos(x, y);
        m_cardSlots[i]->setHomeState(QPointF(x, y), 0.0);
        m_cardSlots[i]->setZValue(10);
        m_scene->addItem(m_cardSlots[i]);
    };

    for (int i = 0; i < 5 && m_shopCards[i]; ++i)
        setupCard(i, startX1 + i * cardSpacingX, row1Y);

    for (int i = 5; i < 7 && m_shopCards[i]; ++i)
        setupCard(i, startX1 + (i - 5) * cardSpacingX, row2Y);

    if (m_saleIndex >= 0 && m_cardSlots[m_saleIndex]) {
        m_cardSlots[m_saleIndex]->setOnSale(true);
        QPixmap labelPix = loadPixmap(":/resources/images/events/Merchant/label.png",
                                         "resources/images/events/Merchant/label.png");
        if (!labelPix.isNull()) {
            labelPix = trimTransparentPadding(labelPix);
            m_saleTag = new QGraphicsPixmapItem(m_cardSlots[m_saleIndex]);
            // 🔴 调小尺寸并往左下微调
            m_saleTag->setPixmap(labelPix.scaled(75, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_saleTag->setPos(25, -125);
            m_saleTag->setZValue(20);
        }
    }

    for (int i = 0; i < 3 && m_shopRelics[i]; ++i) {
        QPixmap icon;
        QString iconPath = m_shopRelics[i]->getImagePath();
        if (!iconPath.isEmpty()) icon.load(iconPath);

        m_relicIcons[i] = new QGraphicsPixmapItem();
        if (!icon.isNull())
            m_relicIcons[i]->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else {
            // 如果没图，画个占位圆形
            QPixmap placeholder(64, 64);
            placeholder.fill(Qt::transparent);
            QPainter p(&placeholder);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(QColor(60, 60, 65));
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(2, 2, 60, 60);
            p.drawText(placeholder.rect(), Qt::AlignCenter, m_shopRelics[i]->getName().left(1));
            p.end();
            m_relicIcons[i]->setPixmap(placeholder);
        }
        m_relicIcons[i]->setPos(relicStartX + i * relicSpacing, relicY);
        m_relicIcons[i]->setZValue(10);
        m_scene->addItem(m_relicIcons[i]);

        m_relicPriceTexts[i] = m_scene->addText(
            QString::number(m_relicPrices[i]) + "g",
            QFont("Microsoft YaHei", 12, QFont::Bold));
        m_relicPriceTexts[i]->setDefaultTextColor(QColor(255, 215, 0));
        // 居中对齐价格
        qreal px = relicStartX + i * relicSpacing + 32 - m_relicPriceTexts[i]->boundingRect().width() / 2;
        m_relicPriceTexts[i]->setPos(px, relicY + 70);
        m_relicPriceTexts[i]->setZValue(10);

        m_relicNameTexts[i] = m_scene->addText(
            m_shopRelics[i]->getName(),
            QFont("Microsoft YaHei", 9));
        m_relicNameTexts[i]->setDefaultTextColor(QColor(200, 200, 200));
        // 居中对齐名字
        qreal nx = relicStartX + i * relicSpacing + 32 - m_relicNameTexts[i]->boundingRect().width() / 2;
        m_relicNameTexts[i]->setPos(nx, relicY + 95);
        m_relicNameTexts[i]->setZValue(10);
    }

    QPixmap removePix = loadPixmap(":/resources/images/events/Merchant/remove.png",
                                     "resources/images/events/Merchant/remove.png");
    if (!removePix.isNull()) {
        m_removeButton = new QGraphicsPixmapItem();
        removePix = trimTransparentPadding(removePix);
        m_removeButton->setPixmap(removePix.scaledToHeight(kShopCardH, Qt::SmoothTransformation));
        m_removeButton->setPos(serviceX, serviceY);
        m_removeButton->setZValue(10);
        m_scene->addItem(m_removeButton);

        // Price below remove
        m_removePriceText = m_scene->addText(
            QString::number(GlobalSaveData::getInstance()->cardRemovalCost) + "g",
            QFont("Microsoft YaHei", 12, QFont::Bold));
        m_removePriceText->setDefaultTextColor(QColor(255, 215, 0));
        m_removePriceText->setPos(removePriceX, serviceY + kShopCardH + 5);
        m_removePriceText->setZValue(10);
    }

    refreshAffordability();
}

// ============================================================
// Shopping interactions
// ============================================================

void MerchantView::refreshAffordability() {
    int gold = m_player->getGold();
    for (int i = 0; i < 7; ++i)
        if (m_cardSlots[i])
            m_cardSlots[i]->setAffordable(gold >= m_cardPrices[i]);

    for (int i = 0; i < 3; ++i) {
        if (m_relicPriceTexts[i] && m_relicIcons[i] && m_relicIcons[i]->isVisible()) {
            m_relicPriceTexts[i]->setDefaultTextColor(gold >= m_relicPrices[i] ? QColor(255, 215, 0) : QColor(220, 50, 50));
        }
    }

    if (m_removePriceText && !m_cardRemoved) {
        int cost = GlobalSaveData::getInstance()->cardRemovalCost;
        m_removePriceText->setDefaultTextColor(gold >= cost ? QColor(255, 215, 0) : QColor(220, 50, 50));
    }
}

void MerchantView::onCardClicked(Card* card, int price) {
    if (m_player->getGold() < price) return;

    int slot = -1;
    for (int i = 0; i < 7; ++i) {
        if (m_cardSlots[i] && m_cardSlots[i]->getLogicCard() == card) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    m_player->modifyGold(-price);
    m_cardManager->addCardToDiscardPile(card);

    if (m_cardSlots[slot] == m_currentHoveredItem) {
        m_currentHoveredItem = nullptr;
        moveHandOffScreen();
    }

    // Start melt-into-orb animation, then clean up slot
    CardItem* purchased = m_cardSlots[slot];
    m_cardSlots[slot] = nullptr; // 🔴 立即断开物理关联，防止手臂追踪残影！

    // 🔴 关键修复：如果买的是打折牌，必须立刻断开 saleTag 指针！
    // 因为 saleTag 是 CardItem 的子图元，当 CardItem 被销毁时，saleTag 也会随之消失。
    // 如果不置空，onExitClicked 里的 m_saleTag->hide() 会导致野指针崩溃！
    if (slot == m_saleIndex && m_saleTag) {
        m_saleTag = nullptr; 
    }

    QPointF cardCenter = purchased->sceneBoundingRect().center();
    playPurchaseEffect(purchased, cardCenter);

    // Remove card from scene immediately (animation handles the visual)
    m_scene->removeItem(purchased);
    // Schedule deferred delete after animation completes
    QTimer::singleShot(900, this, [this, purchased]() {
        delete purchased;
        refreshAffordability();
    });

    qDebug() << "[MerchantView] Purchased card:" << card->getName() << "for" << price;
}

void MerchantView::onRelicClicked(int index) {
    if (index < 0 || index >= 3 || !m_shopRelics[index]) return;
    int price = m_relicPrices[index];
    if (m_player->getGold() < price) return;

    m_player->modifyGold(-price);
    m_relicManager->addRelic(m_shopRelics[index]);

    if (m_relicIcons[index] == m_currentHoveredItem) {
        m_currentHoveredItem = nullptr;
        moveHandOffScreen();
    }

    QGraphicsPixmapItem* icon = m_relicIcons[index];
    m_relicIcons[index] = nullptr; // 🔴 立即断开，防止手臂追踪残影！

    QPointF center = icon->sceneBoundingRect().center();
    playPurchaseEffect(icon, center);

    m_scene->removeItem(icon);
    m_shopRelics[index] = nullptr;

    if (m_relicPriceTexts[index]) { m_scene->removeItem(m_relicPriceTexts[index]); delete m_relicPriceTexts[index]; m_relicPriceTexts[index] = nullptr; }
    if (m_relicNameTexts[index]) { m_scene->removeItem(m_relicNameTexts[index]); delete m_relicNameTexts[index]; m_relicNameTexts[index] = nullptr; }

    QTimer::singleShot(900, this, [this, icon]() {
        delete icon;
        refreshAffordability();
    });

    qDebug() << "[MerchantView] Purchased relic at index" << index << "for" << price;
}

void MerchantView::playPurchaseEffect(QGraphicsItem* item, const QPointF& center) {
    Q_UNUSED(item)
    QPointF startPos = center;
    QPointF endPos(2000, -200);
    const int meltDuration = 400;
    const int flyDuration = 500;
    const int totalSteps = 45; // melt(18) + fly(27)
    const int interval = (meltDuration + flyDuration) / totalSteps;
    const int meltSteps = 18;

    // Orb with radial gradient for 3D glow feel
    auto* glowOrb = new QGraphicsEllipseItem(-22, -22, 44, 44);
    QRadialGradient gradient(0, 0, 22);
    gradient.setColorAt(0.0, QColor(255, 255, 240, 255));   // white-hot core
    gradient.setColorAt(0.3, QColor(255, 230, 120, 240));    // golden mid
    gradient.setColorAt(0.6, QColor(255, 180, 40, 160));     // orange edge
    gradient.setColorAt(1.0, QColor(255, 140, 0, 0));        // transparent fringe
    glowOrb->setBrush(gradient);
    glowOrb->setPen(Qt::NoPen);
    glowOrb->setPos(startPos);
    glowOrb->setScale(0.3);
    glowOrb->setZValue(150);
    m_scene->addItem(glowOrb);

    // Blur effect for bloom
    auto* blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(8);
    blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    glowOrb->setGraphicsEffect(blur);

    // Particle trail storage
    struct TrailParticle { QGraphicsEllipseItem* dot; int age; };
    QList<TrailParticle>* trails = new QList<TrailParticle>();

    int* pStep = new int(0);
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [glowOrb, startPos, endPos, totalSteps, meltSteps,
                                             pStep, timer, trails, this]() {
        (*pStep)++;
        int s = *pStep;

        if (s <= meltSteps) {
            // Melt phase: OutBack easing for "pop" effect
            qreal t = qreal(s) / meltSteps;
            const qreal c1 = 1.70158;
            const qreal c3 = c1 + 1;
            qreal eased = 1 + c3 * pow(t - 1, 3) + c1 * pow(t - 1, 2); // easeOutBack
            glowOrb->setScale(0.3 + eased * 1.0); // 0.3 → 1.3
        } else {
            // Fly phase: InQuart easing for acceleration
            qreal t = qreal(s - meltSteps) / (totalSteps - meltSteps);
            qreal eased = t * t * t * t; // easeInQuart
            QPointF pos(
                startPos.x() + (endPos.x() - startPos.x()) * eased,
                startPos.y() + (endPos.y() - startPos.y()) * eased
            );
            glowOrb->setPos(pos);
            glowOrb->setScale(1.3 - eased * 1.0);
            glowOrb->setOpacity(1.0 - eased * 0.8);

            // Spawn trail particle every 2 steps during fly
            if (s % 2 == 0) {
                auto* dot = new QGraphicsEllipseItem(-5, -5, 10, 10);
                dot->setBrush(QColor(255, 200, 60, 180));
                dot->setPen(Qt::NoPen);
                dot->setPos(glowOrb->pos());
                dot->setZValue(148);
                dot->setScale(0.6);
                m_scene->addItem(dot);
                trails->append({dot, 0});
            }

            // Age and fade trail particles
            for (int i = trails->size() - 1; i >= 0; --i) {
                (*trails)[i].age++;
                int age = (*trails)[i].age;
                qreal life = qMin(qreal(age) / 12.0, 1.0);
                (*trails)[i].dot->setOpacity(1.0 - life);
                (*trails)[i].dot->setScale(0.6 - life * 0.5);
                if (age >= 12) {
                    m_scene->removeItem((*trails)[i].dot);
                    delete (*trails)[i].dot;
                    trails->removeAt(i);
                }
            }
        }

        if (s >= totalSteps) {
            timer->stop();
            // Cleanup remaining trails
            for (auto& tr : *trails) {
                m_scene->removeItem(tr.dot);
                delete tr.dot;
            }
            delete trails;
            if (glowOrb->scene())
                glowOrb->scene()->removeItem(glowOrb);
            delete glowOrb;
            delete pStep;
            delete timer;
        }
    });
    timer->start(interval);
}

void MerchantView::onRemoveClicked() {
    if (m_cardRemoved || m_inRemovalMode) return;

    int cost = GlobalSaveData::getInstance()->cardRemovalCost;
    if (m_player->getGold() < cost) return;

    startCardRemoval();
}

void MerchantView::startCardRemoval() {
    m_inRemovalMode = true;

    // 🔴 移出手臂并重置状态
    m_currentHoveredItem = nullptr;
    moveHandOffScreen();
    if (m_handCursor) m_handCursor->hide();

    // 🔴 物理禁用背景商店元素，防止一切悬停和点击穿透
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(false);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(false);
    if (m_exitBanner) m_exitBanner->setEnabled(false);
    if (m_removeButton) m_removeButton->setEnabled(false);

    // 🔴 调暗背景
    showDarkOverlay();
    if (m_darkOverlay) {
        m_darkOverlay->setBrush(QColor(0, 0, 0, 200)); 
        m_darkOverlay->setZValue(130); // 🔴 提升至 130，绝对压过手臂 (120)
    }

    QList<Card*> removable;
    {
        QList<Card*> draw = m_cardManager->getDrawPile();
        QList<Card*> hand = m_cardManager->getHand();
        QList<Card*> discard = m_cardManager->getDiscardPile();
        removable.append(draw);
        removable.append(hand);
        removable.append(discard);
    }
    if (removable.isEmpty()) { cancelRemoval(); return; }

    showDarkOverlay();

    const int cols = 5;
    const qreal cardW = 150, cardH = 220;
    const qreal startX = 260, startY = 250;
    for (int i = 0; i < removable.size(); ++i) {
        auto* item = new CardItem(removable[i]);
        item->setSelectionEnabled(true);
        int col = i % cols;
        int row = i / cols;
        QPointF targetPos(startX + col * (cardW + 20), startY + row * (cardH + 20));
        item->setPos(targetPos);
        item->setHomeState(targetPos, 0.0);
        item->setZValue(150); // 🔴 设置为 150，压过蒙版 (105)，确保它是亮的！
        m_scene->addItem(item);
        m_removalCardItems.append(item);
        connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
            for (auto* other : m_removalCardItems)
                static_cast<CardItem*>(other)->setHighlighted(false);
            item->setHighlighted(true);
            if (m_confirmRemoveBtn) m_confirmRemoveBtn->show();
        });
    }

    auto* confirmBtn = new TextButton("确认移除", 200, 55);
    confirmBtn->setPos(960 - 120, 900);
    confirmBtn->setZValue(150); // 🔴 高层级
    confirmBtn->hide();
    m_scene->addItem(confirmBtn);
    m_confirmRemoveBtn = confirmBtn;
    connect(confirmBtn, &TextButton::clicked, this, [this]() {
        Card* selected = nullptr;
        for (auto* item : m_removalCardItems) {
            auto* ci = static_cast<CardItem*>(item);
            if (ci->isHighlighted()) { selected = ci->getLogicCard(); break; }
        }
        if (selected) confirmRemoval(selected);
    });

    auto* cancelBtn = new TextButton("取消", 200, 55);
    cancelBtn->setPos(960 + 120, 900);
    cancelBtn->setZValue(150); // 🔴 高层级
    m_scene->addItem(cancelBtn);
    m_cancelRemoveBtn = cancelBtn;
    connect(cancelBtn, &TextButton::clicked, this, &MerchantView::cancelRemoval);
}

void MerchantView::confirmRemoval(Card* card) {
    int cost = GlobalSaveData::getInstance()->cardRemovalCost;
    m_cardManager->removeCardPermanently(card);
    m_player->modifyGold(-cost);
    GlobalSaveData::getInstance()->cardRemovalCost += 25;

    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    m_inRemovalMode = false;
    // 恢复背景元素可用性
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(true);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(true);
    if (m_exitBanner) m_exitBanner->setEnabled(true);
    if (m_removeButton) m_removeButton->setEnabled(true);

    if (m_handCursor) m_handCursor->show();

    if (m_removeButton) {
        m_removeButton->hide();
        QPixmap soldoutPix = loadPixmap(":/resources/images/events/Merchant/soldout-removebg-preview.png",
                                           "resources/images/events/Merchant/soldout-removebg-preview.png");
        if (!soldoutPix.isNull()) {
            soldoutPix = trimTransparentPadding(soldoutPix);
            m_soldoutItem = new QGraphicsPixmapItem();
            // 精确对齐高度
            m_soldoutItem->setPixmap(soldoutPix.scaledToHeight(m_removeButton->pixmap().height(), Qt::SmoothTransformation));
            m_soldoutItem->setPos(m_removeButton->pos());
            m_soldoutItem->setZValue(10);
            m_scene->addItem(m_soldoutItem);
        }
    }
    m_cardRemoved = true;
    refreshAffordability();
    qDebug() << "[MerchantView] Removed card:" << card->getName() << "for" << cost;
}

void MerchantView::cancelRemoval() {
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    m_inRemovalMode = false;
    // 恢复背景元素可用性
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(true);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(true);
    if (m_exitBanner) m_exitBanner->setEnabled(true);
    if (m_removeButton) m_removeButton->setEnabled(true);

    if (m_handCursor) m_handCursor->show();
}

void MerchantView::onExitClicked() {
    qDebug() << "[MerchantView] Exit banner clicked, returning to Phase 1";

    // Hide Phase 2 elements
    if (m_carpet) m_carpet->hide();
    if (m_handCursor) m_handCursor->hide();
    if (m_exitBanner) m_exitBanner->hide();
    if (m_removeButton) m_removeButton->hide();
    if (m_soldoutItem) m_soldoutItem->hide();
    if (m_saleTag) m_saleTag->hide();
    if (m_removePriceText) m_removePriceText->hide();
    for (int i = 0; i < 7; ++i)
        if (m_cardSlots[i]) m_cardSlots[i]->hide();

    for (int i = 0; i < 3; ++i) {
        if (m_relicIcons[i]) m_relicIcons[i]->hide();
        if (m_relicPriceTexts[i]) m_relicPriceTexts[i]->hide();
        if (m_relicNameTexts[i]) m_relicNameTexts[i]->hide();
    }
    setMouseTracking(false);
    m_currentHoveredItem = nullptr;

    // Show Phase 1 elements
    if (m_bgItem) m_bgItem->show();
    if (m_playerImage) m_playerImage->show();
    if (m_merchantImage) m_merchantImage->show();
    if (m_leaveBtn) m_leaveBtn->show();

    m_phase = Phase::Encounter;
}
