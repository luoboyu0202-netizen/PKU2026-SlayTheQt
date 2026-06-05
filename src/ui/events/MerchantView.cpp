#include "MerchantView.h"
#include "../Carditem.h"
#include "TextButton.h"
#include "../../entities/Player.h"
#include "../../entities/relics/Relic.h"
#include "../../entities/relics/RelicManager.h"
#include "../../logic/CardManager.h"
#include "../../logic/CardFactory.h"
#include "../../logic/RelicFactory.h"
#include "../../logic/GlobalSaveData.h" // 统一大小写规范
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include <QDir>
#include <QImage>
#include <QDebug>
#include <cmath>
#include "ui/RelicItem.h" // 🔴 必须引入我们的 3A 级遗物类！（请确认路径是否正确喵）
#include <QPropertyAnimation>

namespace {
constexpr qreal kShopCardW = 170.0;
constexpr qreal kShopCardH = 230.0;
constexpr qreal kShopCardGap = 30.0;

// ========================================================
// 🌟 喵娘的黑魔法：极简呼吸图元！让死贴图瞬间活过来！
// ========================================================
class HoverImageItem : public QGraphicsObject {
public:
    explicit HoverImageItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent), m_pixmap(pixmap) {
        setAcceptHoverEvents(true);
        // 🔴 关键：把物理锚点设在中心，这样放大的时候就是原地呼吸！
        setTransformOriginPoint(boundingRect().center());
    }
    QRectF boundingRect() const override { return m_pixmap.rect(); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPixmap(0, 0, m_pixmap);
    }
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
        anim->setDuration(150);
        anim->setEndValue(1.08); // 悬停时放大 8%
        anim->setEasingCurve(QEasingCurve::OutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
        anim->setDuration(150);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
private:
    QPixmap m_pixmap;
};

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

static QPixmap loadPixmap(const QString& resPath, const QString& fsPath) {
    QPixmap pm(resPath);
    if (!pm.isNull()) return pm;
    qDebug() << "[MerchantView] Resource load failed for" << resPath << ", trying filesystem:" << fsPath;
    pm.load(fsPath);
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
        // 🔴 换成注入灵魂的呼吸实体！
        m_merchantImage = new HoverImageItem(merchantPix.scaled(500, 650, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_merchantImage->setPos(1200, 420);
        m_merchantImage->setZValue(15);
        m_scene->addItem(m_merchantImage);
    }

    // 🔴 祖宗之法不可变：阶段一必须有真正的离开按钮！
    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Merchant/GO_ahead.png");
        m_leaveBtn->show();
    }
}

void MerchantView::onMerchantClicked() {
    qDebug() << "[MerchantView] Transitioning to shop phase";
    m_phase = Phase::Shopping;

    if (!m_carpet) {
        setupPhaseTwo();
    } else {
        if (m_playerImage) m_playerImage->hide();
        if (m_merchantImage) m_merchantImage->hide();
        if (m_bgItem) m_bgItem->hide();
        if (m_leaveBtn) m_leaveBtn->hide();

        if (m_carpet) m_carpet->show();
        if (m_exitBanner) {
            // ========================================================
            // 🔴 核心修复 1：每次飘带重新出现前，强行拍平！解除膨胀死锁！
            // ========================================================
            m_exitBanner->setScale(1.0);
            m_exitBanner->show();
        }
        // 🔴 换成这个完整的括号包块，让价签和按钮同生共死！
        if (m_removeButton && !m_cardRemoved) {
            m_removeButton->show();
            if (m_removePriceText) m_removePriceText->show(); // 🌟 就是漏了这一句！
        }
        if (m_soldoutItem && m_cardRemoved) m_soldoutItem->show();
        if (m_saleTag) m_saleTag->show();
        for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->show();
        for (int i = 0; i < 3; ++i) {
            if (m_relicIcons[i]) m_relicIcons[i]->show();
            if (m_relicPriceTexts[i]) m_relicPriceTexts[i]->show();
            if (m_relicNameTexts[i]) m_relicNameTexts[i]->show();
        }
        setMouseTracking(true);
        refreshAffordability();
    }
}

void MerchantView::setupPhaseTwo() {
    if (m_playerImage) m_playerImage->hide();
    if (m_merchantImage) m_merchantImage->hide();
    if (m_bgItem) m_bgItem->hide();
    if (m_leaveBtn) m_leaveBtn->hide(); // 🔴 进商店地毯时，隐藏顶栏的离开按钮

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
    m_handCursor->setZValue(250);
    m_handCursor->setPos(-200, -500);
    m_scene->addItem(m_handCursor);
    setMouseTracking(true);

    generateShopItems();
    layoutShopItems();

    QPixmap exitPix = loadPixmap(":/resources/images/events/Merchant/exit.png",
                                 "resources/images/events/Merchant/exit.png");
    if (!exitPix.isNull()) {
        // 🔴 换成注入灵魂的呼吸实体！
        m_exitBanner = new HoverImageItem(exitPix.scaled(300, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_exitBanner->setPos(10, 670);
        m_exitBanner->setZValue(50);
        m_scene->addItem(m_exitBanner);
    }
}

void MerchantView::onExitClicked() {
    qDebug() << "[MerchantView] Exit banner clicked, returning to Phase 1";

    // 🔴 隐藏所有阶段 2 的购物物品
    if (m_carpet) m_carpet->hide();
    if (m_handCursor) m_handCursor->hide();
    if (m_exitBanner) m_exitBanner->hide();
    if (m_removeButton) m_removeButton->hide();
    if (m_soldoutItem) m_soldoutItem->hide();
    if (m_saleTag) m_saleTag->hide();
    if (m_removePriceText) m_removePriceText->hide();
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->hide();
    for (int i = 0; i < 3; ++i) {
        if (m_relicIcons[i]) m_relicIcons[i]->hide();
        if (m_relicPriceTexts[i]) m_relicPriceTexts[i]->hide();
        if (m_relicNameTexts[i]) m_relicNameTexts[i]->hide();
    }

    // ========================================================
    // 🔴 致命修复 2：删除了这里原有的 setMouseTracking(false)！
    // 绝对不能让界面窒息！
    // ========================================================
    m_currentHoveredItem = nullptr;

    // 🔴 时光倒流，重新显示阶段 1 的老面孔！
    if (m_bgItem) m_bgItem->show();
    if (m_playerImage) m_playerImage->show();
    if (m_merchantImage) {
        // ========================================================
        // 🔴 核心修复 3：商人重新露脸前，一巴掌拍平！
        // ========================================================
        m_merchantImage->setScale(1.0);
        m_merchantImage->show();
    }
    if (m_leaveBtn) m_leaveBtn->show();

    m_phase = Phase::Encounter;
}

// ============================================================
// Mouse events
// ============================================================

void MerchantView::mousePressEvent(QMouseEvent* event) {
    if (m_inRemovalMode) { QGraphicsView::mousePressEvent(event); return; }
    QPointF scenePt = mapToScene(event->pos());

    if (m_phase == Phase::Encounter) {
        // ========================================================
        // 🎯 商人判定框：避免与离开按钮重叠。
        // 离开按钮的命中检测已由 LeaveButton::shape() 收紧。
        // ========================================================
        QRectF merchantRect(1280, 470, 350, 250);
        if (merchantRect.contains(scenePt)) {
            onMerchantClicked();
            return;
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
        for (int i = 0; i < 3; ++i) {
            if (m_relicIcons[i] && m_relicIcons[i]->isVisible()) {
                QRectF r = m_relicIcons[i]->sceneBoundingRect();
                r.adjust(-10, -10, 10, 80);
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

    hideRelicTooltip();

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
    m_relicTooltipBg->setZValue(260);

    m_relicTooltipText->setPos(10, 10);

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
    for (int i = 0; i < 7; ++i) {
        if (m_cardSlots[i] && m_cardSlots[i]->isVisible() && m_cardSlots[i]->scene()) {
            QRectF r = m_cardSlots[i]->sceneBoundingRect();
            if (r.contains(scenePos))
                return m_cardSlots[i];
        }
    }
    for (int i = 0; i < 3; ++i) {
        if (m_relicIcons[i] && m_relicIcons[i]->isVisible() && m_relicIcons[i]->scene()) {
            QRectF r = m_relicIcons[i]->sceneBoundingRect();
            r.adjust(-10, -10, 10, 10);
            if (r.contains(scenePos))
                return m_relicIcons[i];
        }
    }
    if (m_removeButton && m_removeButton->isVisible()) {
        QRectF r = m_removeButton->sceneBoundingRect();
        if (r.contains(scenePos))
            return m_removeButton;
    }
    return nullptr;
}

void MerchantView::moveHandToItem(QGraphicsItem* item) {
    if (!m_handCursor) return;

    if (m_handAnimX) m_handAnimX->stop();
    if (m_handAnimY) m_handAnimY->stop();

    QRectF itemRect = item->sceneBoundingRect();
    QPointF itemTop(itemRect.center().x(), itemRect.top());

    QPixmap armPm = m_handCursor->pixmap();
    qreal armH = armPm.isNull() ? 750 : armPm.height();
    qreal armW = armPm.isNull() ? 255 : armPm.width();

    qreal startX = m_handCursor->x();
    qreal startY = m_handCursor->y();

    qreal targetX = itemTop.x() - armW / 2;
    qreal targetY = itemTop.y() - armH + 10;

    m_handAnimX = new QVariantAnimation(this);
    m_handAnimX->setDuration(350);
    m_handAnimX->setStartValue(startX);
    m_handAnimX->setEndValue(targetX);
    m_handAnimX->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_handAnimX, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) { if (m_handCursor) m_handCursor->setX(v.toReal()); });
    connect(m_handAnimX, &QVariantAnimation::destroyed, this, [this]() { m_handAnimX = nullptr; });

    m_handAnimY = new QVariantAnimation(this);
    m_handAnimY->setDuration(350);
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
    m_handAnimY->setDuration(200);
    m_handAnimY->setStartValue(m_handCursor->y());
    m_handAnimY->setEndValue(-armH - 100);
    m_handAnimY->setEasingCurve(QEasingCurve::OutQuad);
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

    // 🔴 致命修復 3：讀取玩家存檔，把已經有的遺物從進貨單裡劃掉！
    QList<QString> allRelicIds = RelicFactory::getAllAvailableRelicIds();
    QList<QString> ownedRelics = GlobalSaveData::getInstance()->relicIds;

    QList<QString> availableRelics;
    for (const QString& id : allRelicIds) {
        // 🌟 只有沒買過的遺物，才允許進貨！
        if (!ownedRelics.contains(id)) {
            availableRelics.append(id);
        }
    }
    rng = QRandomGenerator::global();
    for (int i = 0; i < availableRelics.size(); ++i) {
        availableRelics.swapItemsAt(i, rng->bounded(availableRelics.size()));
    }

    int relicCount = qMin(3, availableRelics.size());
    for (int i = 0; i < relicCount; ++i) {
        m_shopRelics[i] = RelicFactory::createRelic(availableRelics[i], this);
        m_relicPrices[i] = 143 + rng->bounded(15);
    }
}

void MerchantView::layoutShopItems() {
    const qreal row1Y = 380, row2Y = row1Y + kShopCardH + 90;
    const qreal startX1 = (1920.0 - (kShopCardW * 5 + kShopCardGap * 4)) / 2.0;
    const qreal cardSpacingX = kShopCardW + kShopCardGap;

    const qreal relicStartX = startX1 + 2 * cardSpacingX + 60;
    const qreal relicSpacing = 180, relicY = row2Y - 10;
    const qreal serviceX = relicStartX + 3 * relicSpacing + 60;
    const qreal serviceY = row2Y - kShopCardH / 2.0;
    const qreal removePriceX = serviceX + kShopCardH / 2 - 35;

    auto setupCard = [this](int i, qreal x, qreal y) {
        m_cardSlots[i] = new CardItem(m_shopCards[i]);
        // 🔴 使用纯展示模式，不让卡牌触发战斗机制
        m_cardSlots[i]->setDisplayOnly(true);
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
            m_saleTag->setPixmap(labelPix.scaled(75, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_saleTag->setPos(25, -125);
            m_saleTag->setZValue(20);
        }
    }

    for (int i = 0; i < 3 && m_shopRelics[i]; ++i) {
        m_relicIcons[i] = new RelicItem(m_shopRelics[i]);

        // ========================================================
        // 🔴 视觉升级 1：遗物图标巨物化！(从 1.25 倍暴力拉升到 2.0 倍！)
        // ========================================================
        qreal relicScale = 2.0;
        m_relicIcons[i]->setScale(relicScale);
        m_relicIcons[i]->setPos(relicStartX + i * relicSpacing, relicY);
        m_relicIcons[i]->setZValue(10);
        m_scene->addItem(m_relicIcons[i]);

        // 🧠 魔法算术：获取放大后图标的真实物理宽度，用于后续文本的绝对居中！
        qreal actualIconWidth = m_relicIcons[i]->boundingRect().width() * relicScale;

        // ========================================================
        // 🔴 视觉升级 2：价格标签大字号化！(字号 12 -> 18)
        // ========================================================
        m_relicPriceTexts[i] = m_scene->addText(
            QString::number(m_relicPrices[i]) + "g",
            QFont("Microsoft YaHei", 18, QFont::Bold));
        m_relicPriceTexts[i]->setDefaultTextColor(QColor(255, 215, 0));

        // 🎯 动态绝对居中：图标左上角 X + 图标一半宽 - 文本一半宽
        qreal px = relicStartX + i * relicSpacing + (actualIconWidth / 2.0) - (m_relicPriceTexts[i]->boundingRect().width() / 2.0);
        // Y轴动态下移：图标顶端 Y + 图标总高度 + 留白 10 像素
        m_relicPriceTexts[i]->setPos(px, relicY + actualIconWidth + 10);
        m_relicPriceTexts[i]->setZValue(10);

        // ========================================================
        // 🔴 视觉升级 3：遗物名字大字号化！(字号 9 -> 14)
        // ========================================================
        m_relicNameTexts[i] = m_scene->addText(
            m_shopRelics[i]->getName(),
            QFont("Microsoft YaHei", 14));
        m_relicNameTexts[i]->setDefaultTextColor(QColor(200, 200, 200));

        qreal nx = relicStartX + i * relicSpacing + (actualIconWidth / 2.0) - (m_relicNameTexts[i]->boundingRect().width() / 2.0);
        m_relicNameTexts[i]->setPos(nx, relicY + actualIconWidth + 40);
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
    // 🔴 同步 GlobalSaveData 里的钱，确保各模块金币统一！
    int gold = GlobalSaveData::getInstance()->gold;

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
    if (GlobalSaveData::getInstance()->gold < price) return;

    int slot = -1;
    for (int i = 0; i < 7; ++i) {
        if (m_cardSlots[i] && m_cardSlots[i]->getLogicCard() == card) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    // ========================================================
    // 🚚 1. 立即扣錢，但先不把卡牌放進牌庫！
    // ========================================================
    GlobalSaveData::getInstance()->gold -= price;

    if (m_cardSlots[slot] == m_currentHoveredItem) {
        m_currentHoveredItem = nullptr;
        moveHandOffScreen();
    }

    CardItem* purchased = m_cardSlots[slot];
    m_cardSlots[slot] = nullptr;

    if (slot == m_saleIndex && m_saleTag) {
        m_saleTag = nullptr;
    }

    // 🚀 2. 讓紫紅色的流星起飛！
    QPointF cardCenter = purchased->sceneBoundingRect().center();
    playPurchaseEffect(purchased, cardCenter);

    // 隱藏卡牌，準備銷毀
    m_scene->removeItem(purchased);

    // 刷新商店裡其他東西你還買不買得起 (因為錢已經扣了)
    refreshAffordability();

    qDebug() << "[MerchantView] Purchased card:" << card->getName() << "for" << price << "... waiting for delivery!";

    // ========================================================
    // 📦 3. 延遲 800ms 發貨！等流星砸中右上角的瞬間才入帳！
    // ========================================================
    QTimer::singleShot(800, this, [this, card, purchased]() {
        // 真正把卡牌寫入存檔！
        GlobalSaveData::getInstance()->deckIds.append(card->getId());

        // 廣播給系統：牌庫更新了！(如果有綁定 TopBar 刷新，此時數字才會變！)
        emit shopDataChanged();

        // 徹底銷毀卡牌的 UI 殼子
        delete purchased;
    });
}

void MerchantView::onRelicClicked(int index) {
    if (index < 0 || index >= 3 || !m_shopRelics[index]) return;
    int price = m_relicPrices[index];
    if (GlobalSaveData::getInstance()->gold < price) return;

    // 1. 立刻扣錢，但先不發貨！
    GlobalSaveData::getInstance()->gold -= price;

    Relic* purchasedRelic = m_shopRelics[index];
    RelicItem* icon = m_relicIcons[index];

    // ========================================================
    // 🔴 核心修复区：购买瞬间，不仅要缩回手，还要强制销毁词条！
    // ========================================================
    if (icon == m_currentHoveredItem) {
        m_currentHoveredItem = nullptr;
        moveHandOffScreen();
        hideRelicTooltip(); // ✨ 加上这句净化咒语！彻底消灭幽灵词条！
    }

    m_relicIcons[index] = nullptr; // 斷開連結

    // 2. 起飛！
    QPointF center = icon->sceneBoundingRect().center();
    playPurchaseEffect(icon, center);

    // 3. 隱藏商店殘留物並刷新價格顯示
    m_scene->removeItem(icon);
    if (m_relicPriceTexts[index]) { m_scene->removeItem(m_relicPriceTexts[index]); delete m_relicPriceTexts[index]; m_relicPriceTexts[index] = nullptr; }
    if (m_relicNameTexts[index]) { m_scene->removeItem(m_relicNameTexts[index]); delete m_relicNameTexts[index]; m_relicNameTexts[index] = nullptr; }
    refreshAffordability();

    // ========================================================
    // 🚚 魔法 3：延遲發貨！等 800ms 流星抵達時，再呼叫頂欄出現！
    // ========================================================
    QTimer::singleShot(800, this, [this, purchasedRelic, icon]() {
        GlobalSaveData::getInstance()->relicIds.append(purchasedRelic->getId());
        emit relicBought(purchasedRelic); // 司令部這時才會收到信號！
        emit shopDataChanged();
        delete icon; // 功成身退
    });
}

void MerchantView::playPurchaseEffect(QGraphicsItem* item, const QPointF& center) {
    bool isRelic = (dynamic_cast<RelicItem*>(item) != nullptr);

    // ========================================================
    // 🔮 魔法 1：跨次元结界！覆盖在整个 GameWindow 之上
    // ========================================================
    QGraphicsView* fxView = new QGraphicsView(this->window());
    fxView->resize(this->window()->size()); // 覆盖 1600x900
    fxView->setStyleSheet("background: transparent; border: none;");
    fxView->setAttribute(Qt::WA_TransparentForMouseEvents);
    fxView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fxView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // ❌ 已经彻底删除了极其耗性能的 FullViewportUpdate！

    QGraphicsScene* fxScene = new QGraphicsScene(0, 0, fxView->width(), fxView->height(), fxView);
    fxView->setScene(fxScene);
    fxView->show();
    fxView->raise(); // 🔴 絕對壓制，蓋過 TopBar！

    // ========================================================
    // 🔪 核心修复：精准脏矩形同步！
    // 不再全屏刷新，而是让收回的手指“手把手”教透明结界哪里需要刷新！
    // ========================================================
    if (m_handAnimY) {
        connect(m_handAnimY, &QVariantAnimation::valueChanged, fxView, [this, fxView]() {
            if (m_handCursor && fxView->viewport()) {
                // 1. 拿到手部在底层场景中的物理矩形大小
                QRectF handRect = m_handCursor->sceneBoundingRect();

                // 2. 将底层的坐标精准映射到顶层的 GameWindow 坐标系
                QPoint topLeft = this->mapTo(this->window(), this->mapFromScene(handRect.topLeft()));
                QPoint bottomRight = this->mapTo(this->window(), this->mapFromScene(handRect.bottomRight()));

                // 3. 命令透明层：只刷新手指经过的这块长条形区域！
                fxView->viewport()->update(QRect(topLeft, bottomRight));
            }
        });
    }

    // ========================================================
    // 🗺️ 魔法 2：精準坐標轉換
    // 把 MerchantView 裡的坐標，轉換成 GameWindow 的全域坐標！
    // ========================================================
    QPoint viewPos = this->mapFromScene(center);
    QPointF startPos = this->mapTo(this->window(), viewPos);
    QPointF endPos;

    if (isRelic) {
        GlobalSaveData* save = GlobalSaveData::getInstance();
        // 🔴 修正偏左 Bug：因為我們接下來會「延遲發貨」，
        // 此時的 size() 就是它飛到時應該佔據的絕對空位，不再需要 -1 了！
        int currentIndex = save->relicIds.size();

        int trayStartX = 10;
        int trayStartY = 55;
        int spacing = 8;
        endPos = QPointF(trayStartX + currentIndex * (48 + spacing) + 24, trayStartY + 24);
    } else {
        endPos = QPointF(1400, 24);
    }

    QPointF ctrlPos(startPos.x() + (endPos.x() - startPos.x()) * 0.4, startPos.y() - 300);

    const int totalSteps = 50;
    const int interval = 16;

    // 🌟 光球與拖尾邏輯 (使用新的 fxScene)
    auto* glowOrb = new QGraphicsEllipseItem(-25, -25, 50, 50);
    QRadialGradient gradient(0, 0, 25);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));
    if (isRelic) {
        gradient.setColorAt(0.3, QColor(50, 200, 255, 255));
        gradient.setColorAt(1.0, QColor(0, 100, 255, 0));
    } else {
        gradient.setColorAt(0.3, QColor(200, 50, 255, 255));
        gradient.setColorAt(1.0, QColor(100, 0, 255, 0));
    }
    glowOrb->setBrush(gradient);
    glowOrb->setPen(Qt::NoPen);
    glowOrb->setPos(startPos);

    // ❌ 已经彻底删除了 QGraphicsBlurEffect，径向渐变本身就足够平滑且性能极高！
    fxScene->addItem(glowOrb);

    struct TrailParticle { QGraphicsEllipseItem* dot; qreal dx, dy; int age; int life; };
    auto* trails = new QList<TrailParticle>();
    int* pStep = new int(0);
    auto* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [=]() {
        (*pStep)++;
        int s = *pStep;
        qreal t = qreal(s) / totalSteps;
        qreal easedT = t * t * (3 - 2 * t);
        qreal u = 1.0 - easedT;
        QPointF currentPos = u * u * startPos + 2 * u * easedT * ctrlPos + easedT * easedT * endPos;
        glowOrb->setPos(currentPos);
        glowOrb->setScale(1.0 + sin(t * 3.14159) * 0.3);

        for(int i = 0; i < 2; i++) {
            auto* dot = new QGraphicsEllipseItem(-4, -4, 8, 8);
            if (isRelic) dot->setBrush(QColor(100, 220, 255, 200));
            else dot->setBrush(QColor(220, 100, 255, 200));
            dot->setPen(Qt::NoPen);
            dot->setPos(currentPos + QPointF((QRandomGenerator::global()->generateDouble()-0.5)*20, (QRandomGenerator::global()->generateDouble()-0.5)*20));
            fxScene->addItem(dot);

            qreal dx = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            qreal dy = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            trails->append({dot, dx, dy, 0, 10 + QRandomGenerator::global()->bounded(8)});
        }

        for (int i = trails->size() - 1; i >= 0; --i) {
            auto& tr = (*trails)[i];
            tr.age++;
            tr.dot->moveBy(tr.dx, tr.dy);
            qreal lifeRatio = qreal(tr.age) / tr.life;
            tr.dot->setOpacity(1.0 - lifeRatio);
            tr.dot->setScale(1.0 - lifeRatio * 0.5);
            if (tr.age >= tr.life) {
                fxScene->removeItem(tr.dot);
                delete tr.dot;
                trails->removeAt(i);
            }
        }

        // 💥 抵達終點
        if (s >= totalSteps) {
            timer->stop();
            for(int i = 0; i < 10; i++) {
                auto* spark = new QGraphicsEllipseItem(-3, -3, 6, 6);
                spark->setBrush(Qt::white);
                spark->setPen(Qt::NoPen);
                spark->setPos(endPos);
                fxScene->addItem(spark);

                qreal angle = i * (3.14159 * 2 / 10.0);
                qreal speed = 5.0 + QRandomGenerator::global()->generateDouble() * 3.0;
                qreal vX = cos(angle) * speed;
                qreal vY = sin(angle) * speed;

                auto* sparkTimer = new QTimer(fxView);
                int* sparkAge = new int(0);
                connect(sparkTimer, &QTimer::timeout, fxView, [=]() {
                    (*sparkAge)++;
                    spark->moveBy(vX, vY);
                    spark->setOpacity(1.0 - (*sparkAge) / 12.0);
                    if(*sparkAge >= 12) {
                        sparkTimer->stop();
                        fxScene->removeItem(spark);
                        delete spark;
                        delete sparkAge;
                        sparkTimer->deleteLater();
                    }
                });
                sparkTimer->start(16);
            }

            for (auto& tr : *trails) { fxScene->removeItem(tr.dot); delete tr.dot; }
            delete trails;
            fxScene->removeItem(glowOrb);
            delete glowOrb;
            delete pStep;
            timer->deleteLater();

            // 🔴 延遲銷毀結界
            QTimer::singleShot(500, fxView, [fxView]() {
                fxView->hide();
                fxView->deleteLater();
            });
        }
    });
    timer->start(interval);
}

void MerchantView::onRemoveClicked() {
    if (m_cardRemoved || m_inRemovalMode) return;

    int cost = GlobalSaveData::getInstance()->cardRemovalCost;
    if (GlobalSaveData::getInstance()->gold < cost) return;

    startCardRemoval();
}

void MerchantView::startCardRemoval() {
    m_inRemovalMode = true;

    m_currentHoveredItem = nullptr;
    moveHandOffScreen();
    if (m_handCursor) m_handCursor->hide();

    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(false);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(false);
    if (m_exitBanner) m_exitBanner->setEnabled(false);
    if (m_removeButton) m_removeButton->setEnabled(false);

    showDarkOverlay();
    if (m_darkOverlay) {
        m_darkOverlay->setBrush(QColor(0, 0, 0, 200));
        m_darkOverlay->setZValue(130);
    }

    // ========================================================
    // 🪄 魔法：給主執行緒 50 毫秒的時間去渲染黑幕，然後再加載卡牌！
    // ========================================================
    QTimer::singleShot(50, this, [this]() {
        // 🔴 把生成卡牌的耗時邏輯，全部包在這個非同步閉包裡！
        QList<Card*> removable;
        GlobalSaveData* save = GlobalSaveData::getInstance();
        for (const QString& id : save->deckIds) {
            removable.append(CardFactory::createCard(id));
        }

        if (removable.isEmpty()) { cancelRemoval(); return; }

        // ========================================================
        // 🔴 魔法 1：雙重排序！(費用 -> 字母)，找牌不再眼花撩亂！
        // ========================================================
        std::sort(removable.begin(), removable.end(), [](Card* a, Card* b) {
            if (a->getCost() != b->getCost()) return a->getCost() < b->getCost();
            return a->getName() < b->getName();
        });

        // ========================================================
        // 🎨 魔法 2：6列黃金居中排版 & 計算最大滾動深度
        // ========================================================
        int columns = 6;
        qreal spacingX = 220;
        qreal spacingY = 280;
        qreal startY = 250;
        qreal screenW = 1920.0;

        qreal totalGridWidth = (columns - 1) * spacingX;
        qreal startX = (screenW - totalGridWidth) / 2.0;

        int totalRows = (removable.size() + columns - 1) / columns;
        qreal totalContentHeight = startY + (totalRows * spacingY) + 200; // 底部留 200 給按鈕

        // 如果卡牌超過螢幕高度 (1080)，就解鎖滾動！
        m_maxRemovalScrollY = std::max(0.0, totalContentHeight - 1080.0);
        m_removalScrollY = 0.0; // 每次打開重置滾動

        for (int i = 0; i < removable.size(); ++i) {
            auto* item = new CardItem(removable[i]);
            item->setDisplayOnly(true);
            item->setSelectionEnabled(true);

            int col = i % columns;
            int row = i / columns;
            QPointF targetPos(startX + col * spacingX, startY + row * spacingY);

            item->setPos(targetPos);
            item->setHomeState(targetPos, 0.0);
            item->setZValue(150); // 卡牌在 Z=150 的高度滑動

            m_scene->addItem(item);
            m_removalCardItems.append(item);

            connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
                for (auto* other : m_removalCardItems)
                    static_cast<CardItem*>(other)->setHighlighted(false);
                item->setHighlighted(true);
                if (m_confirmRemoveBtn) m_confirmRemoveBtn->show();
            });
        }

        // ========================================================
        // 🛡️ 魔法 3：無敵的底部按鈕 (把 Z-Value 拉高到 200！)
        // ========================================================
        auto* confirmBtn = new TextButton("确认移除", 200, 55);
        confirmBtn->setPos(960 - 120, 950);
        confirmBtn->setZValue(200); // 🔴 絕對高空壓制！卡牌會從它底下鑽過去，不會擋住按鈕！
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
        cancelBtn->setPos(960 + 120, 950);
        cancelBtn->setZValue(200); // 🔴 絕對高空壓制！
        m_scene->addItem(cancelBtn);
        m_cancelRemoveBtn = cancelBtn;
        connect(cancelBtn, &TextButton::clicked, this, &MerchantView::cancelRemoval);
    });
}

void MerchantView::confirmRemoval(Card* card) {
    int cost = GlobalSaveData::getInstance()->cardRemovalCost;

    // 🔴 执行删牌
    GlobalSaveData::getInstance()->deckIds.removeOne(card->getId());
    GlobalSaveData::getInstance()->gold -= cost;
    GlobalSaveData::getInstance()->cardRemovalCost += 25;

    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    m_inRemovalMode = false;
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(true);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(true);
    if (m_exitBanner) m_exitBanner->setEnabled(true);
    if (m_removeButton) m_removeButton->setEnabled(true);

    if (m_handCursor) m_handCursor->show();

    if (m_removeButton) {
        m_removeButton->hide();
        // ========================================================
        // 🔴 核心修复：既然都卖光了，底下的价签也必须赶紧藏起来喵！
        // ========================================================
        if (m_removePriceText) m_removePriceText->hide();

        // 🌟 破案关键：把名字和后缀严格改成和 .qrc 里一模一样的！
        QPixmap soldoutPix = loadPixmap(":/resources/images/events/Merchant/soldout-removebg-preview.png",
                                        "resources/images/events/Merchant/soldout-removebg-preview.png");

        if (!soldoutPix.isNull()) {
            soldoutPix = trimTransparentPadding(soldoutPix);
            m_soldoutItem = new QGraphicsPixmapItem();
            m_soldoutItem->setPixmap(soldoutPix.scaledToHeight(m_removeButton->pixmap().height(), Qt::SmoothTransformation));
            m_soldoutItem->setPos(m_removeButton->pos());
            m_soldoutItem->setZValue(10);
            m_scene->addItem(m_soldoutItem);
        } else {
            // 加一个保底报错，万一没出来控制台会告诉你！
            qDebug() << "[MerchantView] ❌ 售罄图片加载失败！请再次核对 qrc 路径！";
        }
    }
    m_cardRemoved = true;
    refreshAffordability();
    qDebug() << "[MerchantView] Removed card:" << card->getName() << "for" << cost;

    emit shopDataChanged();
}

void MerchantView::cancelRemoval() {
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    m_inRemovalMode = false;
    for (int i = 0; i < 7; ++i) if (m_cardSlots[i]) m_cardSlots[i]->setEnabled(true);
    for (int i = 0; i < 3; ++i) if (m_relicIcons[i]) m_relicIcons[i]->setEnabled(true);
    if (m_exitBanner) m_exitBanner->setEnabled(true);
    if (m_removeButton) m_removeButton->setEnabled(true);

    if (m_handCursor) m_handCursor->show();
}

// ========================================================
// 🖱️ 滾輪事件攔截 (QWidget 版)
// ========================================================
void MerchantView::wheelEvent(QWheelEvent* event) {
    // 只有在刪牌模式下，且牌多到需要滾動時才生效！
    if (m_inRemovalMode && m_maxRemovalScrollY > 0) {
        qreal scrollStep = 60.0; // 絲滑滾動靈敏度

        // 🔴 QWheelEvent 用 angleDelta().y() 來判斷方向
        // 大於 0 表示往前滾（向上），小於 0 表示往後滾（向下）
        if (event->angleDelta().y() > 0) {
            m_removalScrollY -= scrollStep; // 向上推
        } else {
            m_removalScrollY += scrollStep; // 向下拉
        }

        // 用 std::clamp 鎖死界限，防止滾出宇宙
        m_removalScrollY = std::clamp(m_removalScrollY, 0.0, m_maxRemovalScrollY);

        updateRemovalCardPositions();
        event->accept();
        return;
    }

    // 如果不是刪牌模式，把事件還給父類別處理
    EventBaseView::wheelEvent(event);
}

// ========================================================
// 🔄 刷新手牌矩陣的垂直位置
// ========================================================
void MerchantView::updateRemovalCardPositions() {
    int columns = 6;
    qreal spacingX = 220;
    qreal spacingY = 280;
    qreal startY = 250;
    qreal screenW = 1920.0;

    qreal totalGridWidth = (columns - 1) * spacingX;
    qreal startX = (screenW - totalGridWidth) / 2.0;

    for (int i = 0; i < m_removalCardItems.size(); ++i) {
        int row = i / columns;
        int col = i % columns;

        qreal targetX = startX + col * spacingX;
        // 🔴 靈魂減法：減去滾動偏移量，實現卡牌升降！
        qreal targetY = startY + row * spacingY - m_removalScrollY;

        auto* item = static_cast<CardItem*>(m_removalCardItems[i]);
        item->setPos(targetX, targetY);

        // 記得更新 HomeState，這樣滑鼠移開時它才會回彈到正確的高度！
        item->setHomeState(QPointF(targetX, targetY), 0.0);
    }
}