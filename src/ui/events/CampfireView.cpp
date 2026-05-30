#include "CampfireView.h"
#include "../carditem.h"
#include <QTimer>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QRandomGenerator>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPen>
#include <cmath>

// ---- 动画工具 ----
static QVariantAnimation* makePulse(QObject* parent, QGraphicsItem* target,
                                     qreal amp, int duration) {
    auto* anim = new QVariantAnimation(parent);
    anim->setDuration(duration);
    anim->setLoopCount(-1);
    anim->setKeyValueAt(0.0, 1.0);
    anim->setKeyValueAt(0.5, 1.0 + amp);
    anim->setKeyValueAt(1.0, 1.0);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    QObject::connect(anim, &QVariantAnimation::valueChanged, parent, [target](const QVariant& v) {
        target->setScale(v.toReal());
    });
    return anim;
}

// ============================================================
// 构造
// ============================================================
CampfireView::CampfireView(Player* player, CardManager* cardManager,
                           RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, cardManager, relicManager, parent) {
    setupContent();
}

void CampfireView::setupContent() {
    // 1. 切换为真正的越肩视角人物
    QPixmap otsPixmap(":/resources/images/events/Campfire/rear_side-removebg-preview.png");
    if (!otsPixmap.isNull()) {
        // 放大角色，使其占据左侧近景
        m_playerImage->setPixmap(otsPixmap.scaled(1100, 1100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_playerImage->setPos(-100, 240); 
        m_playerImage->setZValue(120); // 提升 Z-Order，使其在某些烟雾之上或与其交互
    }

    createCampfireVisual();
    showDarkOverlay();

    // 背景烟雾团（选项背后，营造梦幻感，增加模糊）
    m_choiceCloud = new QGraphicsEllipseItem(-550, -250, 1100, 500);
    QRadialGradient cloudGrad(0, 0, 550);
    cloudGrad.setColorAt(0.0, QColor(255, 255, 250, 150));
    cloudGrad.setColorAt(0.5, QColor(220, 220, 215, 60));
    cloudGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    m_choiceCloud->setBrush(cloudGrad);
    m_choiceCloud->setPen(Qt::NoPen);
    m_choiceCloud->setPos(960, 450);
    m_choiceCloud->setZValue(98);
    auto* cloudBlur = new QGraphicsBlurEffect();
    cloudBlur->setBlurRadius(50);
    m_choiceCloud->setGraphicsEffect(cloudBlur);
    m_scene->addItem(m_choiceCloud);
    makePulse(this, m_choiceCloud, 0.03, 3500)->start();

    // "我该做什么呢？" 提示文字（颜色调至极亮）
    m_promptText = new QGraphicsTextItem("我该做什么呢？");
    m_promptText->setDefaultTextColor(QColor(255, 250, 240)); 
    m_promptText->setFont(QFont("Microsoft YaHei", 36, QFont::Bold));
    m_promptText->setZValue(101);
    m_promptText->setPos(960 - m_promptText->boundingRect().width() / 2, 260);
    m_scene->addItem(m_promptText);

    // 按钮居中排列
    m_restBtn = new IconButton(":/resources/images/events/Campfire/Rest.png");
    m_restBtn->setPos(660, 420);
    m_restBtn->setZValue(100);
    m_scene->addItem(m_restBtn);
    connect(m_restBtn, &IconButton::clicked, this, &CampfireView::onRest);

    m_restLabel = new QGraphicsTextItem("休息");
    m_restLabel->setDefaultTextColor(QColor(255, 215, 120)); // 更亮一些的金黄
    m_restLabel->setFont(QFont("Microsoft YaHei", 26, QFont::Bold));
    // 280/2 = 140
    m_restLabel->setPos(660 + 140 - m_restLabel->boundingRect().width() / 2, 620);
    m_restLabel->setZValue(101);
    m_scene->addItem(m_restLabel);

    m_upgradeBtn = new IconButton(":/resources/images/events/Campfire/Smith.png");
    m_upgradeBtn->setPos(980, 420); 
    m_upgradeBtn->setZValue(100);
    m_scene->addItem(m_upgradeBtn);
    connect(m_upgradeBtn, &IconButton::clicked, this, &CampfireView::onUpgrade);

    m_upgradeLabel = new QGraphicsTextItem("锻造");
    m_upgradeLabel->setDefaultTextColor(QColor(255, 215, 120));
    m_upgradeLabel->setFont(QFont("Microsoft YaHei", 26, QFont::Bold));
    m_upgradeLabel->setPos(980 + 140 - m_upgradeLabel->boundingRect().width() / 2, 620);
    m_upgradeLabel->setZValue(101);
    m_scene->addItem(m_upgradeLabel);
}

// ============================================================
// 火堆场景
// ============================================================
void CampfireView::createCampfireVisual() {
    auto* rng = QRandomGenerator::global();
    const QPointF fireCenter(1380, 880); 

    // ---- 1. 背景底色 ----
    auto* upperDark = new QGraphicsRectItem(0, 0, 1920, 1080);
    QLinearGradient upperGrad(0, 0, 0, 1080);
    upperGrad.setColorAt(0.0, QColor(5, 5, 8));
    upperGrad.setColorAt(0.7, QColor(20, 15, 12));
    upperGrad.setColorAt(1.0, QColor(40, 30, 25));
    upperDark->setBrush(upperGrad);
    upperDark->setPen(Qt::NoPen);
    upperDark->setZValue(0);
    m_scene->addItem(upperDark);
    m_fireItems.append(upperDark);

    // ---- 2. 引入真正的火堆图片 (Bonfire.png) - 保持静态且进一步缩小 ----
    QPixmap firePixmap(":/resources/images/events/Campfire/Bonfire.png");
    if (!firePixmap.isNull()) {
        // 进一步缩小火焰大小 (从 700x560 -> 550x440)
        auto* fireImg = new QGraphicsPixmapItem(firePixmap.scaled(550, 440, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // 重新对齐位置（底部中心对准 fireCenter）
        fireImg->setPos(fireCenter.x() - 275, fireCenter.y() - 420);
        fireImg->setZValue(4); 
        m_scene->addItem(fireImg);
        m_fireItems.append(fireImg);
    }

    // ---- 3. 下半地面 ----
    QPainterPath groundPath;
    for (int row = 0; row < 6; ++row) {
        qreal yBase = 750 + row * 70;
        for (int col = 0; col < 16; ++col) {
            qreal xBase = col * 160 - 100 + (row % 2) * 80;
            qreal jx = rng->bounded(20) - 10;
            qreal jy = rng->bounded(15) - 7;
            QPainterPath stone;
            stone.moveTo(xBase + jx, yBase + jy);
            stone.lineTo(xBase + 155 + rng->bounded(15), yBase - 5 + rng->bounded(15));
            stone.lineTo(xBase + 165 + rng->bounded(10), yBase + 75 + rng->bounded(10));
            stone.lineTo(xBase - 5 + rng->bounded(15), yBase + 78 + rng->bounded(8));
            stone.closeSubpath();
            groundPath.addPath(stone);
        }
    }
    auto* ground = new QGraphicsPathItem(groundPath);
    ground->setBrush(QColor(40, 35, 30));
    ground->setPen(QPen(QColor(15, 12, 10), 2));
    ground->setZValue(1);
    m_scene->addItem(ground);
    m_fireItems.append(ground);

    // ---- 4. 地面火光染色 ----
    auto* floorGlow = new QGraphicsEllipseItem(-600, -80, 1200, 250);
    QRadialGradient floorFill(0, 0, 600);
    floorFill.setColorAt(0.0, QColor(255, 180, 70, 160));
    floorFill.setColorAt(0.35, QColor(220, 140, 50, 100));
    floorFill.setColorAt(0.7, QColor(140, 70, 25, 30));
    floorFill.setColorAt(1.0, QColor(0, 0, 0, 0));
    floorGlow->setBrush(floorFill);
    floorGlow->setPen(Qt::NoPen);
    floorGlow->setPos(fireCenter.x(), fireCenter.y() + 20);
    floorGlow->setZValue(2);
    m_scene->addItem(floorGlow);
    m_fireItems.append(floorGlow);

    // ---- 5. 环境光晕 ----
    auto* ambGlow = new QGraphicsEllipseItem(-700, -500, 1400, 1000);
    QRadialGradient ambFill(0, 0, 700);
    ambFill.setColorAt(0.0, QColor(245, 220, 175, 70));
    ambFill.setColorAt(1.0, QColor(0, 0, 0, 0));
    ambGlow->setBrush(ambFill);
    ambGlow->setPen(Qt::NoPen);
    ambGlow->setPos(fireCenter.x(), fireCenter.y() - 100);
    ambGlow->setZValue(3);
    m_scene->addItem(ambGlow);
    m_fireItems.append(ambGlow);
    makePulse(this, ambGlow, 0.05, 1500)->start();

    // ---- 6. 火星 (15个) ----
    for (int i = 0; i < 15; ++i) {
        qreal size = 2 + rng->bounded(3);
        auto* ember = new QGraphicsEllipseItem(-size / 2, -size / 2, size, size);
        ember->setBrush(QColor(255, 240, 190, 175));
        ember->setPen(Qt::NoPen);
        ember->setPos(fireCenter.x() + rng->bounded(200) - 100,
                      fireCenter.y() + rng->bounded(50) - 160);
        ember->setZValue(12);
        m_scene->addItem(ember);
        m_fireItems.append(ember);

        auto* opacityEff = new QGraphicsOpacityEffect();
        opacityEff->setOpacity(0.0);
        ember->setGraphicsEffect(opacityEff);
        QPointF start = ember->pos();
        QPointF end(start.x() + rng->bounded(60) - 30, start.y() - 150 - rng->bounded(100));
        auto* floatAnim = new QVariantAnimation(this);
        floatAnim->setDuration(2600 + rng->bounded(1600));
        floatAnim->setLoopCount(-1);
        floatAnim->setStartValue(start);
        floatAnim->setEndValue(end);
        connect(floatAnim, &QVariantAnimation::valueChanged, this, [ember, opacityEff](const QVariant& v) {
            ember->setPos(v.toPointF());
            qreal t = (ember->pos().y() + 200) / 400.0;
            t = qBound(0.0, t, 1.0);
            opacityEff->setOpacity(0.5 * (1.0 - t));
        });
        floatAnim->start();
        m_fireAnimations.append(floatAnim);
    }
}



// ============================================================
// 休息
// ============================================================
void CampfireView::onRest() {
    m_restBtn->hide();
    m_upgradeBtn->hide();
    m_restLabel->hide();
    m_upgradeLabel->hide();
    m_promptText->hide();
    m_choiceCloud->hide();
    setLeaveButtonVisible(false);

    if (m_overlayText) {
        m_overlayText->setPlainText("休息中...");
        m_overlayText->setPos(960 - m_overlayText->boundingRect().width() / 2, 400);
    }

    createRestSmoke();

    QTimer::singleShot(2500, this, [this]() {
        // 熄灭火焰
        for (auto* item : m_fireItems) {
            // Z=2到Z=12之间是火、光、木柴、火星
            if (item->zValue() >= 2 && item->zValue() <= 12) {
                item->hide();
            }
        }
        for (auto* anim : m_fireAnimations) {
            anim->stop();
        }
    });

    QTimer::singleShot(3600, this, [this]() {
        m_player->heal(static_cast<int>(m_player->getMaxHp() * 0.3));
    });

    QTimer::singleShot(6200, this, [this]() {
        hideDarkOverlay();
        // 永久隐藏选项相关 UI
        m_restBtn->hide();
        m_upgradeBtn->hide();
        m_restLabel->hide();
        m_upgradeLabel->hide();
        m_promptText->hide();
        m_choiceCloud->hide();
        
        // 更新并显示离开按钮（前进箭头）
        if (m_leaveBtn) {
            m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
            m_leaveBtn->show();
        }
    });
}

void CampfireView::createRestSmoke() {
    auto* rng = QRandomGenerator::global();
    const QPointF fireCenter(1380, 880);

    auto makeSmoke = [this, rng, fireCenter](qreal minDist, qreal maxDist) {
        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal dist = minDist + rng->bounded(maxDist - minDist);
        qreal tx = fireCenter.x() + cos(angle) * dist;
        qreal ty = fireCenter.y() + sin(angle) * dist - rng->bounded(100);

        const qreal size = 60 + rng->bounded(120); 
        auto* smoke = new QGraphicsEllipseItem(-size / 2, -size / 2, size, size, nullptr);
        smoke->setBrush(QColor(220, 215, 205, 180));
        smoke->setPen(Qt::NoPen);
        smoke->setPos(fireCenter);
        smoke->setZValue(105);
        m_scene->addItem(smoke);

        auto* opacityEff = new QGraphicsOpacityEffect();
        opacityEff->setOpacity(0.0);
        smoke->setGraphicsEffect(opacityEff);

        QPointF drift(tx + rng->bounded(200) - 100, ty + rng->bounded(150) - 75);

        auto* pathAnim = new QVariantAnimation(this);
        pathAnim->setDuration(4000 + rng->bounded(2000));
        pathAnim->setStartValue(QPointF(fireCenter));
        pathAnim->setKeyValueAt(0.3, drift);
        pathAnim->setKeyValueAt(1.0, QPointF(drift.x() + rng->bounded(300) - 150,
                                              drift.y() - 400 - rng->bounded(200)));
        pathAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(pathAnim, &QVariantAnimation::valueChanged, this, [smoke](const QVariant& v) {
            smoke->setPos(v.toPointF());
        });

        auto* opacityAnim = new QVariantAnimation(this);
        opacityAnim->setDuration(pathAnim->duration());
        opacityAnim->setKeyValueAt(0.0, 0.0);
        opacityAnim->setKeyValueAt(0.15, 0.7); 
        opacityAnim->setKeyValueAt(0.6, 0.5);
        opacityAnim->setKeyValueAt(1.0, 0.0);
        connect(opacityAnim, &QVariantAnimation::valueChanged, this, [opacityEff](const QVariant& v) {
            opacityEff->setOpacity(v.toReal());
        });

        auto* scaleAnim = new QVariantAnimation(this);
        scaleAnim->setDuration(pathAnim->duration());
        scaleAnim->setStartValue(0.3);
        scaleAnim->setEndValue(2.5 + rng->bounded(100) / 100.0);
        scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(scaleAnim, &QVariantAnimation::valueChanged, this, [smoke](const QVariant& v) {
            smoke->setScale(v.toReal());
        });

        auto* group = new QParallelAnimationGroup(this);
        group->addAnimation(pathAnim);
        group->addAnimation(opacityAnim);
        group->addAnimation(scaleAnim);
        connect(group, &QAbstractAnimation::finished, this, [smoke]() { delete smoke; });
        group->start(QAbstractAnimation::DeleteWhenStopped);
    };

    // 爆发性产生大量烟雾
    for (int i = 0; i < 20; ++i)  makeSmoke(100, 300);
    for (int i = 0; i < 40; ++i)  makeSmoke(300, 800);
    for (int i = 0; i < 60; ++i)  makeSmoke(800, 1600);

    // 全屏遮罩加强
    auto* veil = new QGraphicsRectItem(0, 0, 1920, 1080);
    veil->setBrush(QColor(230, 225, 215));
    veil->setPen(Qt::NoPen);
    veil->setZValue(300); // 确保遮住角色和状态栏
    m_scene->addItem(veil);

    auto* veilOpacity = new QGraphicsOpacityEffect();
    veilOpacity->setOpacity(0.0);
    veil->setGraphicsEffect(veilOpacity);

    auto* veilAnim = new QVariantAnimation(this);
    veilAnim->setDuration(6000);
    veilAnim->setKeyValueAt(0.0, 0.0);
    veilAnim->setKeyValueAt(0.2, 0.85); 
    veilAnim->setKeyValueAt(0.5, 0.85);
    veilAnim->setKeyValueAt(1.0, 0.0);
    connect(veilAnim, &QVariantAnimation::valueChanged, this, [veilOpacity](const QVariant& v) {
        veilOpacity->setOpacity(v.toReal());
    });
    connect(veilAnim, &QAbstractAnimation::finished, this, [veil]() { delete veil; });
    veilAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================
// 锻造
// ============================================================
void CampfireView::onUpgrade() {
    m_restBtn->hide();
    m_upgradeBtn->hide();
    m_restLabel->hide();
    m_upgradeLabel->hide();
    m_promptText->hide();
    m_choiceCloud->hide();
    setLeaveButtonVisible(false);

    QList<Card*> candidates = allUpgradableCards();
    if (candidates.isEmpty()) {
        if (m_overlayText) {
            m_overlayText->setPlainText("没有可升级的卡牌");
            m_overlayText->setPos(960 - m_overlayText->boundingRect().width() / 2, 400);
            m_overlayText->show();
        }
        QTimer::singleShot(1500, this, [this]() {
            hideDarkOverlay();
            m_restBtn->show();
            m_upgradeBtn->show();
            m_restLabel->show();
            m_upgradeLabel->show();
            m_promptText->show();
            m_choiceCloud->show();
            setLeaveButtonVisible(true);
        });
        return;
    }

    showCardSelector();
}

QList<Card*> CampfireView::allUpgradableCards() const {
    QList<Card*> result;
    auto add = [&](const QList<Card*>& pile) {
        for (Card* c : pile) {
            if (c && !c->isUpgraded() && !result.contains(c))
                result.append(c);
        }
    };
    add(m_cardManager->getDrawPile());
    add(m_cardManager->getHand());
    add(m_cardManager->getDiscardPile());
    add(m_cardManager->getExhaustPile());
    return result;
}

void CampfireView::showCardSelector() {
    // 调暗背景并提升遮罩层
    showDarkOverlay();
    if (m_darkOverlay) {
        m_darkOverlay->setBrush(QColor(0, 0, 0, 200)); // 进一步加深背景
        m_darkOverlay->setZValue(150); 
    }

    m_cardSelectPrompt = new QGraphicsTextItem("选择一张卡牌升级");
    m_cardSelectPrompt->setDefaultTextColor(QColor(255, 230, 150)); // 柔和的金黄色
    m_cardSelectPrompt->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    m_cardSelectPrompt->setZValue(160);
    m_scene->addItem(m_cardSelectPrompt);
    m_cardSelectPrompt->setPos(960 - m_cardSelectPrompt->boundingRect().width() / 2, 140);

    QList<Card*> candidates = allUpgradableCards();

    const int cols = qMin(candidates.size(), 5);
    const int cardW = 170;
    const int cardH = 240;
    const int startX = 960 - (cols * cardW) / 2;

    for (int i = 0; i < candidates.size(); ++i) {
        auto* item = new CardItem(candidates[i]);
        item->setSelectionEnabled(true);
        int col = i % cols;
        int row = i / cols;
        item->setPos(startX + col * cardW + cardW / 2, 320 + row * (cardH + 20));
        item->setZValue(160);
        m_scene->addItem(item);
        m_cardDisplayItems.append(item);

        connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
            for (auto* other : m_cardDisplayItems)
                static_cast<CardItem*>(other)->setHighlighted(false);
            item->setHighlighted(true);
            m_selectedCard = item->getLogicCard();
            if (m_confirmBtn) m_confirmBtn->show();
        });
    }

    m_confirmBtn = new TextButton("确认升级", 200, 55);
    m_confirmBtn->setPos(960 - 120, 900);
    m_confirmBtn->setZValue(160);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
    connect(m_confirmBtn, &TextButton::clicked, this, &CampfireView::confirmUpgrade);

    m_cancelBtn = new TextButton("取消", 200, 55);
    m_cancelBtn->setPos(960 + 120, 900);
    m_cancelBtn->setZValue(160);
    m_scene->addItem(m_cancelBtn);
    connect(m_cancelBtn, &TextButton::clicked, this, &CampfireView::cancelUpgrade);
}


void CampfireView::confirmUpgrade() {
    if (!m_selectedCard) return;

    for (auto* item : m_cardDisplayItems) { m_scene->removeItem(item); delete item; }
    m_cardDisplayItems.clear();
    if (m_cardSelectPrompt) { m_scene->removeItem(m_cardSelectPrompt); delete m_cardSelectPrompt; m_cardSelectPrompt = nullptr; }
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    // 🔴 关键修复：确认升级后彻底隐藏蒙版，恢复原始亮度
    hideDarkOverlay();
    
    runUpgradeAnimation(m_selectedCard);
}

void CampfireView::cancelUpgrade() {
    m_selectedCard = nullptr;
    for (auto* item : m_cardDisplayItems) { m_scene->removeItem(item); delete item; }
    m_cardDisplayItems.clear();
    if (m_cardSelectPrompt) { m_scene->removeItem(m_cardSelectPrompt); delete m_cardSelectPrompt; m_cardSelectPrompt = nullptr; }
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    m_restBtn->show();
    m_upgradeBtn->show();
    m_restLabel->show();
    m_upgradeLabel->show();
    m_promptText->show();
    m_choiceCloud->show();
    setLeaveButtonVisible(true);
}

void CampfireView::runUpgradeAnimation(Card* card) {
    auto* animCard = new CardItem(card);
    animCard->setPos(960, 450);
    animCard->setZValue(110);
    animCard->setHighlighted(true);
    m_scene->addItem(animCard);

    auto* seq = new QSequentialAnimationGroup(this);

    auto* s1 = new QVariantAnimation(this);
    s1->setDuration(400);
    s1->setStartValue(1.0);
    s1->setEndValue(1.15);
    s1->setEasingCurve(QEasingCurve::OutBack);
    QObject::connect(s1, &QVariantAnimation::valueChanged, this, [animCard](const QVariant& v) {
        animCard->setScale(v.toReal());
    });
    seq->addAnimation(s1);

    auto* shake1 = new QVariantAnimation(this);
    shake1->setDuration(200);
    shake1->setKeyValueAt(0.0, QPointF(960, 450));
    shake1->setKeyValueAt(0.25, QPointF(958, 452));
    shake1->setKeyValueAt(0.5, QPointF(962, 448));
    shake1->setKeyValueAt(0.75, QPointF(959, 451));
    shake1->setKeyValueAt(1.0, QPointF(960, 450));
    QObject::connect(shake1, &QVariantAnimation::valueChanged, this, [animCard](const QVariant& v) {
        animCard->setPos(v.toPointF());
    });
    seq->addAnimation(shake1);
    seq->addPause(100);

    // 光粒爆散 (12粒子并行)
    auto* burstGroup = new QParallelAnimationGroup(this);
    for (int i = 0; i < 12; ++i) {
        qreal size = 5 + QRandomGenerator::global()->bounded(6);
        auto* p = new QGraphicsEllipseItem(-size / 2, -size / 2, size, size, nullptr);
        p->setBrush(i % 3 == 0 ? QColor(255, 245, 225, 230)
                     : i % 3 == 1 ? QColor(248, 235, 210, 220)
                     : QColor(240, 228, 205, 235));
        p->setPen(Qt::NoPen);
        p->setPos(960, 450);
        p->setZValue(115);
        m_scene->addItem(p);

        double angle = (2.0 * M_PI * i) / 12.0;
        qreal radius = 70 + QRandomGenerator::global()->bounded(60);
        QPointF target(960 + cos(angle) * radius, 450 + sin(angle) * radius);

        auto* pMove = new QVariantAnimation(this);
        pMove->setDuration(480);
        pMove->setStartValue(QPointF(960, 450));
        pMove->setEndValue(target);
        pMove->setEasingCurve(QEasingCurve::OutQuad);
        QObject::connect(pMove, &QVariantAnimation::valueChanged, this, [p](const QVariant& v) {
            p->setPos(v.toPointF());
        });

        auto* pFadeEff = new QGraphicsOpacityEffect(); pFadeEff->setOpacity(1.0);
        p->setGraphicsEffect(pFadeEff);
        auto* pFade = new QVariantAnimation(this);
        pFade->setDuration(480);
        pFade->setStartValue(1.0);
        pFade->setEndValue(0.0);
        pFade->setEasingCurve(QEasingCurve::OutQuad);
        QObject::connect(pFade, &QVariantAnimation::valueChanged, this, [pFadeEff](const QVariant& v) {
            pFadeEff->setOpacity(v.toReal());
        });

        auto* pGroup = new QParallelAnimationGroup(this);
        pGroup->addAnimation(pMove);
        pGroup->addAnimation(pFade);
        QObject::connect(pGroup, &QAbstractAnimation::finished, this, [p]() { delete p; });
        burstGroup->addAnimation(pGroup);
    }
    seq->addAnimation(burstGroup);

    auto* shake2 = new QVariantAnimation(this);
    shake2->setDuration(300);
    shake2->setKeyValueAt(0.0, QPointF(960, 450));
    shake2->setKeyValueAt(0.2, QPointF(950, 445));
    shake2->setKeyValueAt(0.4, QPointF(970, 455));
    shake2->setKeyValueAt(0.6, QPointF(955, 442));
    shake2->setKeyValueAt(0.8, QPointF(965, 458));
    shake2->setKeyValueAt(1.0, QPointF(960, 450));
    QObject::connect(shake2, &QVariantAnimation::valueChanged, this, [animCard](const QVariant& v) {
        animCard->setPos(v.toPointF());
    });
    seq->addAnimation(shake2);

    auto* s2 = new QVariantAnimation(this);
    s2->setDuration(300);
    s2->setStartValue(1.15);
    s2->setEndValue(1.0);
    s2->setEasingCurve(QEasingCurve::InBack);
    QObject::connect(s2, &QVariantAnimation::valueChanged, this, [animCard](const QVariant& v) {
        animCard->setScale(v.toReal());
    });
    seq->addAnimation(s2);

    QObject::connect(seq, &QAbstractAnimation::finished, this, [this, card, animCard]() {
        card->upgrade();
        m_scene->removeItem(animCard);
        delete animCard;

        if (m_overlayText) {
            m_overlayText->setPlainText("升级成功！");
            m_overlayText->show();
            m_overlayText->setPos(960 - m_overlayText->boundingRect().width() / 2, 400);
        }
        QTimer::singleShot(1200, this, [this]() {
            hideDarkOverlay();
            // 永久隐藏选项相关 UI
            m_restBtn->hide();
            m_upgradeBtn->hide();
            m_restLabel->hide();
            m_upgradeLabel->hide();
            m_promptText->hide();
            m_choiceCloud->hide();
            
            if (m_leaveBtn) {
                m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
                m_leaveBtn->show();
            }
        });
    });

    seq->start(QAbstractAnimation::DeleteWhenStopped);
}
