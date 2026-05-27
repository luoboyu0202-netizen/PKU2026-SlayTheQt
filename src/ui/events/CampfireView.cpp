#include "CampfireView.h"
#include "SelectableCardItem.h"
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
    const QPointF fireCenter(1380, 880); // 画面右下侧，更符合理想图构图

    // ---- 上半纯暗面 ----
    auto* upperDark = new QGraphicsRectItem(0, 0, 1920, 1080);
    QLinearGradient upperGrad(0, 0, 0, 900);
    upperGrad.setColorAt(0.0, QColor(10, 8, 6));
    upperGrad.setColorAt(0.6, QColor(25, 20, 15));
    upperGrad.setColorAt(1.0, QColor(65, 50, 35));
    upperDark->setBrush(upperGrad);
    upperDark->setPen(Qt::NoPen);
    upperDark->setZValue(0);
    m_scene->addItem(upperDark);
    m_fireItems.append(upperDark);

    // ---- 下半石板地面 (铺满底部) ----
    QPainterPath groundPath;
    for (int row = 0; row < 6; ++row) {
        qreal yBase = 720 + row * 80;
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
    ground->setBrush(QColor(60, 50, 40));
    ground->setPen(QPen(QColor(20, 15, 10), 2));
    ground->setZValue(1);
    m_scene->addItem(ground);
    m_fireItems.append(ground);

    // ---- 地面火光染色 ----
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

    // ---- 木柴 (居中对齐 fireCenter) ----
    auto addLog = [this](qreal cx, qreal cy, qreal rot, qreal w, qreal h,
                         const QColor& base, const QColor& topRim, const QColor& burnGlow) {
        // ... (保持 logPath 逻辑不变)
        QPainterPath logPath;
        qreal hw = w / 2, hh = h / 2, r = hh * 0.7;
        logPath.moveTo(-hw + 15, -hh);
        logPath.cubicTo(-hw + 5, -hh + 3, -hw, -hh + 8, -hw, -hh + r);
        logPath.lineTo(-hw, hh - r);
        logPath.cubicTo(-hw, hh - 3, -hw + 8, hh, -hw + 18, hh);
        logPath.lineTo(hw - 12, hh);
        logPath.cubicTo(hw - 3, hh, hw + 2, hh - 5, hw, hh - r);
        logPath.lineTo(hw, -hh + r);
        logPath.cubicTo(hw + 3, -hh + 6, hw - 5, -hh + 2, hw - 10, -hh);
        logPath.closeSubpath();

        auto* log = new QGraphicsPathItem(logPath);
        QLinearGradient logFill(0, -hh, 0, hh);
        logFill.setColorAt(0.0, burnGlow);
        logFill.setColorAt(0.15, topRim);
        logFill.setColorAt(0.45, base.lighter(120));
        logFill.setColorAt(0.75, base);
        logFill.setColorAt(1.0, base.darker(170));
        log->setBrush(logFill);
        log->setPen(QPen(base.darker(200), 2.5));
        log->setPos(cx, cy);
        log->setRotation(rot);
        log->setZValue(5);
        m_scene->addItem(log);
        m_fireItems.append(log);
    };

    // 木柴位置相对于 fireCenter 对齐
    addLog(fireCenter.x() - 100, fireCenter.y() + 20, -18, 380, 42, QColor(88, 48, 24), QColor(175, 108, 60), QColor(255, 195, 85, 175));
    addLog(fireCenter.x() + 80, fireCenter.y() + 28, 15, 360, 38, QColor(75, 42, 20), QColor(158, 95, 50), QColor(245, 175, 65, 160));
    addLog(fireCenter.x() - 40, fireCenter.y() + 25, -5, 400, 40,  QColor(80, 45, 23), QColor(162, 100, 54), QColor(250, 185, 75, 170));
    addLog(fireCenter.x() + 120, fireCenter.y() + 18, 25, 340, 35, QColor(68, 38, 16), QColor(148, 88, 42), QColor(240, 168, 55, 155));
    addLog(fireCenter.x() + 20, fireCenter.y() + 32, -28, 350, 38, QColor(62, 35, 14), QColor(142, 82, 40), QColor(235, 160, 50, 150));

    // ---- 单中心火焰 (多层，从木柴上升起) ----
    auto makeFlame = [](qreal w, qreal h) {
        QPainterPath path;
        path.moveTo(w * 0.50, 0);
        path.cubicTo(w * 0.18, h * 0.12, w * 0.03, h * 0.38, w * 0.12, h * 0.62);
        path.cubicTo(w * 0.18, h * 0.52, w * 0.38, h * 0.72, w * 0.46, h * 1.0);
        path.cubicTo(w * 0.58, h * 0.70, w * 0.72, h * 0.52, w * 0.68, h * 0.28);
        path.cubicTo(w * 0.80, h * 0.42, w * 0.94, h * 0.22, w * 0.50, 0);
        path.closeSubpath();
        return path;
    };

    // 火焰层从下往上 (外层大→核心小, 基底对齐fireCenter.y=790)
    // 外层 - 暗橙 (h=440, base at 790)
    auto* fOuter = new QGraphicsPathItem(makeFlame(360, 440));
    fOuter->setBrush(QColor(200, 100, 35, 145));
    fOuter->setPen(Qt::NoPen);
    fOuter->setPos(fireCenter.x() - 180, fireCenter.y() - 440);
    fOuter->setTransformOriginPoint(180, 440);
    fOuter->setZValue(7);
    auto* foBlur = new QGraphicsBlurEffect(); foBlur->setBlurRadius(22);
    fOuter->setGraphicsEffect(foBlur);
    m_scene->addItem(fOuter);
    m_fireItems.append(fOuter);

    // 中层 - 亮橙 (h=340, base at 790)
    auto* fMid1 = new QGraphicsPathItem(makeFlame(260, 340));
    fMid1->setBrush(QColor(238, 155, 60, 180));
    fMid1->setPen(Qt::NoPen);
    fMid1->setPos(fireCenter.x() - 130, fireCenter.y() - 340);
    fMid1->setTransformOriginPoint(130, 340);
    fMid1->setZValue(8);
    auto* fm1Blur = new QGraphicsBlurEffect(); fm1Blur->setBlurRadius(14);
    fMid1->setGraphicsEffect(fm1Blur);
    m_scene->addItem(fMid1);
    m_fireItems.append(fMid1);

    // 中上层 - 金黄 (h=250, base at 790)
    auto* fMid2 = new QGraphicsPathItem(makeFlame(180, 250));
    fMid2->setBrush(QColor(252, 215, 110, 190));
    fMid2->setPen(Qt::NoPen);
    fMid2->setPos(fireCenter.x() - 90, fireCenter.y() - 250);
    fMid2->setTransformOriginPoint(90, 250);
    fMid2->setZValue(9);
    auto* fm2Blur = new QGraphicsBlurEffect(); fm2Blur->setBlurRadius(9);
    fMid2->setGraphicsEffect(fm2Blur);
    m_scene->addItem(fMid2);
    m_fireItems.append(fMid2);

    // 内层 - 亮黄白 (h=170, base at 790)
    auto* fInner = new QGraphicsPathItem(makeFlame(110, 170));
    fInner->setBrush(QColor(255, 245, 180, 205));
    fInner->setPen(Qt::NoPen);
    fInner->setPos(fireCenter.x() - 55, fireCenter.y() - 170);
    fInner->setTransformOriginPoint(55, 170);
    fInner->setZValue(10);
    auto* fiBlur = new QGraphicsBlurEffect(); fiBlur->setBlurRadius(6);
    fInner->setGraphicsEffect(fiBlur);
    m_scene->addItem(fInner);
    m_fireItems.append(fInner);

    // 核心 - 纯白热点 (火焰中上部)
    auto* fCore = new QGraphicsEllipseItem(-14, -14, 28, 28);
    fCore->setBrush(QColor(255, 255, 252, 240));
    fCore->setPen(Qt::NoPen);
    fCore->setPos(fireCenter.x() + 3, fireCenter.y() - 100);
    fCore->setZValue(11);
    auto* fcBlur = new QGraphicsBlurEffect(); fcBlur->setBlurRadius(5);
    fCore->setGraphicsEffect(fcBlur);
    m_scene->addItem(fCore);
    m_fireItems.append(fCore);

    // ---- 火焰动画 ----
    auto addFlameMotion = [this](QGraphicsItem* item, qreal deg, qreal scaleAmp, int dur) {
        auto* anim = new QVariantAnimation(this);
        anim->setDuration(dur);
        anim->setLoopCount(-1);
        anim->setKeyValueAt(0.0, QPointF(-deg, 1.0));
        anim->setKeyValueAt(0.33, QPointF(deg * 0.5, 1.0 + scaleAmp));
        anim->setKeyValueAt(0.66, QPointF(-deg * 0.7, 1.0 - scaleAmp * 0.3));
        anim->setKeyValueAt(1.0, QPointF(-deg, 1.0));
        anim->setEasingCurve(QEasingCurve::InOutSine);
        connect(anim, &QVariantAnimation::valueChanged, this, [item](const QVariant& v) {
            const QPointF p = v.toPointF();
            item->setRotation(p.x());
            item->setScale(p.y());
        });
        anim->start();
        m_fireAnimations.append(anim);
    };

    addFlameMotion(fOuter, 2.0, 0.03, 1050);
    addFlameMotion(fMid1, -2.5, 0.045, 850);
    addFlameMotion(fMid2, 2.2, 0.06, 700);
    addFlameMotion(fInner, -1.8, 0.075, 580);
    addFlameMotion(fCore, 1.0, 0.10, 480);

    // ---- 环境光晕 (大幅提升alpha) ----
    auto* ambGlow = new QGraphicsEllipseItem(-650, -480, 1300, 960);
    QRadialGradient ambFill(0, 0, 650);
    ambFill.setColorAt(0.0, QColor(245, 220, 175, 90));
    ambFill.setColorAt(0.25, QColor(200, 160, 110, 55));
    ambFill.setColorAt(0.5, QColor(140, 100, 60, 20));
    ambFill.setColorAt(0.75, QColor(60, 40, 20, 5));
    ambFill.setColorAt(1.0, QColor(0, 0, 0, 0));
    ambGlow->setBrush(ambFill);
    ambGlow->setPen(Qt::NoPen);
    ambGlow->setPos(fireCenter.x(), fireCenter.y() - 50);
    ambGlow->setZValue(3);
    m_scene->addItem(ambGlow);
    m_fireItems.append(ambGlow);
    makePulse(this, ambGlow, 0.04, 1400)->start();

    // 右侧额外暖光（给玩家打背光）
    auto* sideGlow = new QGraphicsEllipseItem(-380, -350, 760, 700);
    QRadialGradient sideFill(0, 0, 380);
    sideFill.setColorAt(0.0, QColor(210, 145, 75, 60));
    sideFill.setColorAt(0.5, QColor(150, 95, 45, 20));
    sideFill.setColorAt(1.0, QColor(0, 0, 0, 0));
    sideGlow->setBrush(sideFill);
    sideGlow->setPen(Qt::NoPen);
    sideGlow->setPos(fireCenter.x() - 140, fireCenter.y() - 60);
    sideGlow->setZValue(3);
    m_scene->addItem(sideGlow);
    m_fireItems.append(sideGlow);

    // ---- 火星 (12个) ----
    for (int i = 0; i < 12; ++i) {
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
    QList<Card*> candidates = allUpgradableCards();

    m_cardSelectPrompt = new QGraphicsTextItem("选择一张卡牌升级");
    m_cardSelectPrompt->setDefaultTextColor(Qt::white);
    m_cardSelectPrompt->setFont(QFont("Microsoft YaHei", 22, QFont::Bold));
    m_cardSelectPrompt->setZValue(100);
    m_scene->addItem(m_cardSelectPrompt);
    m_cardSelectPrompt->setPos(960 - m_cardSelectPrompt->boundingRect().width() / 2, 140);

    const int cols = qMin(candidates.size(), 5);
    const int cardW = 170;
    const int cardH = 240;
    const int startX = 960 - (cols * cardW) / 2;

    for (int i = 0; i < candidates.size(); ++i) {
        auto* item = new SelectableCardItem(candidates[i], nullptr);
        int col = i % cols;
        int row = i / cols;
        item->setPos(startX + col * cardW + cardW / 2, 320 + row * (cardH + 20));
        item->setZValue(100);
        m_scene->addItem(item);
        m_cardDisplayItems.append(item);

        connect(item, &SelectableCardItem::clicked, this, [this, item](Card*) {
            for (auto* other : m_cardDisplayItems)
                static_cast<SelectableCardItem*>(other)->setHighlighted(false);
            item->setHighlighted(true);
            m_selectedCard = item->card();
            if (m_confirmBtn) m_confirmBtn->show();
        });
    }

    m_confirmBtn = new TextButton("确认升级", 200, 55);
    m_confirmBtn->setPos(960 - 120, 900);
    m_confirmBtn->setZValue(100);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
    connect(m_confirmBtn, &TextButton::clicked, this, &CampfireView::confirmUpgrade);

    m_cancelBtn = new TextButton("取消", 200, 55);
    m_cancelBtn->setPos(960 + 120, 900);
    m_cancelBtn->setZValue(100);
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

    if (m_overlayText) m_overlayText->hide();
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
    auto* animCard = new SelectableCardItem(card, nullptr);
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
