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
#include "../GameWindow.h"
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPen>
#include <cmath>
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include <QWheelEvent>
#include <QPropertyAnimation>

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
    // ========================================================
    // 🟢 架构归位：这里才是火堆专属贴图真正应该待的地方！
    // ========================================================
    QPixmap otsPixmap(":/resources/images/events/Campfire/rear_side-removebg-preview.png");

    // 加上安全判断，确保基类把空画板准备好了
    if (!otsPixmap.isNull() && m_playerImage != nullptr) {
        m_playerImage->setPixmap(otsPixmap.scaled(1100, 1100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_playerImage->setPos(-100, 240);
        m_playerImage->setZValue(120); // 火堆视图要求人物层级更高，盖住某些东西
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

    // ---- 2. 引入真正的火堆图片 (Bonfire.png) ----
    QPixmap firePixmap(":/resources/images/events/Campfire/Bonfire.png");
    if (!firePixmap.isNull()) {
        auto* fireImg = new QGraphicsPixmapItem(firePixmap.scaled(550, 440, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
            if (item->zValue() >= 2 && item->zValue() <= 12) {
                item->hide();
            }
        }
        for (auto* anim : m_fireAnimations) {
            anim->stop();
        }
    });

    QTimer::singleShot(3600, this, [this]() {
        GlobalSaveData* save = GlobalSaveData::getInstance();
        int healAmount = static_cast<int>(save->maxHp * 0.3); // 恢复 30%

        save->currentHp += healAmount;
        if (save->currentHp > save->maxHp) {
            save->currentHp = save->maxHp;
        }

        qDebug() << "[Campfire] 篝火休息完毕！当前血量:" << save->currentHp << "/" << save->maxHp;

        // 🔴 扣动扳机！呼叫顶栏刷新血量条！
        emit playerStatusChanged();
    });

    QTimer::singleShot(6200, this, [this]() {
        hideDarkOverlay();
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
}

void CampfireView::createRestSmoke() {
    auto* rng = QRandomGenerator::global();
    const QPointF fireCenter(1380, 880);

    // 🌟 新增了一个 delay 参数，让烟雾错峰涌出
    auto makeSmoke = [this, rng, fireCenter](qreal minDist, qreal maxDist, int delay) {
        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal dist = minDist + rng->bounded(maxDist - minDist);
        qreal tx = fireCenter.x() + cos(angle) * dist;
        // 🔴 让 Y 轴的偏移量偏向负数（向上），营造热气上升感
        qreal ty = fireCenter.y() + sin(angle) * dist - rng->bounded(250);

        const qreal size = 80 + rng->bounded(150);

        // 🔴 魔法 1：不用正圆，用稍微压扁的椭圆！这样旋转时才会有“翻滚”的视觉差！
        auto* smoke = new QGraphicsEllipseItem(-size / 2, -size * 0.4, size, size * 0.8, nullptr);
        smoke->setBrush(QColor(230, 225, 215, 180));
        smoke->setPen(Qt::NoPen);
        smoke->setPos(fireCenter);
        smoke->setRotation(rng->bounded(360)); // 随机初始角度
        smoke->setZValue(105);
        m_scene->addItem(smoke);

        auto* opacityEff = new QGraphicsOpacityEffect();
        opacityEff->setOpacity(0.0);
        smoke->setGraphicsEffect(opacityEff);

        // 进一步强调向上漂浮
        QPointF drift(tx + rng->bounded(200) - 100, ty - rng->bounded(150));
        QPointF endPoint(drift.x() + rng->bounded(300) - 150, drift.y() - 400 - rng->bounded(300));

        auto* pathAnim = new QVariantAnimation(this);
        pathAnim->setDuration(4000 + rng->bounded(2000));
        pathAnim->setStartValue(QPointF(fireCenter));
        pathAnim->setKeyValueAt(0.3, drift);
        pathAnim->setKeyValueAt(1.0, endPoint);
        pathAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(pathAnim, &QVariantAnimation::valueChanged, this, [smoke](const QVariant& v) {
            smoke->setPos(v.toPointF());
        });

        // 🔴 魔法 2：烟雾翻滚动画
        auto* rotAnim = new QVariantAnimation(this);
        rotAnim->setDuration(pathAnim->duration());
        rotAnim->setStartValue(smoke->rotation());
        rotAnim->setEndValue(smoke->rotation() + rng->bounded(180) - 90); // 缓慢自转
        connect(rotAnim, &QVariantAnimation::valueChanged, this, [smoke](const QVariant& v) {
            smoke->setRotation(v.toReal());
        });

        auto* opacityAnim = new QVariantAnimation(this);
        opacityAnim->setDuration(pathAnim->duration());
        opacityAnim->setKeyValueAt(0.0, 0.0);
        opacityAnim->setKeyValueAt(0.15, 0.8); // 稍微提亮一点
        opacityAnim->setKeyValueAt(0.6, 0.5);
        opacityAnim->setKeyValueAt(1.0, 0.0);
        connect(opacityAnim, &QVariantAnimation::valueChanged, this, [opacityEff](const QVariant& v) {
            opacityEff->setOpacity(v.toReal());
        });

        auto* scaleAnim = new QVariantAnimation(this);
        scaleAnim->setDuration(pathAnim->duration());
        scaleAnim->setStartValue(0.2);
        scaleAnim->setEndValue(3.0 + rng->bounded(150) / 100.0); // 膨胀得更大
        scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(scaleAnim, &QVariantAnimation::valueChanged, this, [smoke](const QVariant& v) {
            smoke->setScale(v.toReal());
        });

        auto* group = new QParallelAnimationGroup(this);
        group->addAnimation(pathAnim);
        group->addAnimation(rotAnim); // 加入翻滚
        group->addAnimation(opacityAnim);
        group->addAnimation(scaleAnim);
        connect(group, &QAbstractAnimation::finished, this, [smoke]() { delete smoke; });

        // 🔴 魔法 3：錯峰出行！根据传入的 delay 延迟发射，制造绵绵不断的雾气感！
        QTimer::singleShot(delay, this, [group]() {
            group->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    // 爆发性产生大量烟雾，赋予 0~800 毫秒的不等延迟
    for (int i = 0; i < 20; ++i)  makeSmoke(100, 300, rng->bounded(200));
    for (int i = 0; i < 40; ++i)  makeSmoke(300, 800, 150 + rng->bounded(400));
    for (int i = 0; i < 60; ++i)  makeSmoke(800, 1800, 300 + rng->bounded(500));

    // ========================================================
    // 🔴 致命修复：遮天蔽日的物理黑幕！直接铺满 4000x3000！
    // ========================================================
    auto* veil = new QGraphicsRectItem(-1000, -1000, 4000, 3000);
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
    veilAnim->setKeyValueAt(0.2, 0.95); // 稍微再浓厚一点
    veilAnim->setKeyValueAt(0.5, 0.95);
    veilAnim->setKeyValueAt(1.0, 0.0);
    connect(veilAnim, &QVariantAnimation::valueChanged, this, [veilOpacity](const QVariant& v) {
        veilOpacity->setOpacity(v.toReal());
    });
    connect(veilAnim, &QAbstractAnimation::finished, this, [veil]() { delete veil; });
    veilAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CampfireView::onUpgrade() {
    qDebug() << "[CRASH-TEST] 1. 成功进入 onUpgrade()";

    if (!m_restBtn || !m_upgradeBtn) {
        qDebug() << "[CRASH-TEST] 🚨 致命错误：休息或锻造按钮的指针为空！";
        return;
    }

    m_restBtn->hide();
    m_upgradeBtn->hide();
    m_restLabel->hide();
    m_upgradeLabel->hide();
    m_promptText->hide();
    m_choiceCloud->hide();
    setLeaveButtonVisible(false);

    QList<Card*> candidates = allUpgradableCards();

    if (candidates.isEmpty()) {
        showDarkOverlay();
        if (m_darkOverlay) { m_darkOverlay->setZValue(150); }

        m_cardSelectPrompt = new QGraphicsTextItem("无牌可升");
        m_cardSelectPrompt->setDefaultTextColor(Qt::gray);
        m_cardSelectPrompt->setFont(QFont("Microsoft YaHei", 36, QFont::Bold));
        m_cardSelectPrompt->setPos(960 - m_cardSelectPrompt->boundingRect().width() / 2, 400);
        m_cardSelectPrompt->setZValue(160);
        m_scene->addItem(m_cardSelectPrompt);

        m_cancelBtn = new TextButton("返回", 200, 55);
        m_cancelBtn->setPos(960, 600);
        m_cancelBtn->setZValue(160);
        m_scene->addItem(m_cancelBtn);
        connect(m_cancelBtn, &TextButton::clicked, this, &CampfireView::cancelUpgrade);
        return;
    }

    showCardSelector(candidates);
}

QList<Card*> CampfireView::allUpgradableCards() const {
    qDebug() << "[CRASH-TEST] A. 进入 allUpgradableCards()";
    QList<Card*> result;
    GlobalSaveData* save = GlobalSaveData::getInstance();

    if (!save) {
        qDebug() << "[CRASH-TEST] 🚨 致命错误：GlobalSaveData 单例为空！";
        return result;
    }

    qDebug() << "[CRASH-TEST] B. 全局存档读取成功，卡组数量：" << save->deckIds.size();

    for (const QString& cardId : save->deckIds) {
        qDebug() << "[CRASH-TEST] C. 尝试调用工厂印卡，卡牌 ID：" << cardId;
        Card* c = CardFactory::createCard(cardId);

        if (!c) {
            qDebug() << "[CRASH-TEST] 🚨 警告：印卡失败或返回空指针！ID：" << cardId;
            continue;
        }

        if(c->getType()==CardType::Curse)continue;

        if (!c->isUpgraded()) {
            c->setProperty("original_id", cardId);
            result.append(c);
            qDebug() << "[CRASH-TEST] E. 卡牌加入升级池：" << c->getName();
        } else {
            qDebug() << "[CRASH-TEST] F. 卡牌已升级，销毁：" << c->getName();
            delete c;
        }
    }
    qDebug() << "[CRASH-TEST] G. allUpgradableCards() 结束，找到可升级卡牌数量：" << result.size();
    return result;
}

void CampfireView::showCardSelector(const QList<Card*>& candidates) {
    showDarkOverlay();
    if (m_darkOverlay) {
        m_darkOverlay->setBrush(QColor(0, 0, 0, 220));
        m_darkOverlay->setZValue(150);
    }

    m_cardSelectPrompt = new QGraphicsTextItem("选择一张卡牌升级");
    m_cardSelectPrompt->setDefaultTextColor(QColor(255, 230, 150));
    m_cardSelectPrompt->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    m_cardSelectPrompt->setZValue(300);
    m_scene->addItem(m_cardSelectPrompt);
    m_cardSelectPrompt->setPos(960 - m_cardSelectPrompt->boundingRect().width() / 2, 80);

    const int maxCols = 5;
    const int cardW = 250;
    const int cardH = 300;
    const int totalCards = candidates.size();

    for (int i = 0; i < totalCards; ++i) {
        auto* item = new CardItem(candidates[i]);
        item->setDisplayOnly(true);
        item->installEventFilter(this);
        item->setProperty("ui_selected", false);

        int row = i / maxCols;
        int cardsInThisRow = (row == (totalCards - 1) / maxCols) ? (totalCards % maxCols) : maxCols;
        if (cardsInThisRow == 0) cardsInThisRow = maxCols;

        int col = i % maxCols;
        int rowWidth = cardsInThisRow * cardW;
        qreal startX = 960.0 - rowWidth / 2.0;

        qreal x = startX + col * cardW + cardW / 2.0;
        qreal y = 380.0 + row * cardH;

        item->setPos(x, y);
        item->setZValue(160);
        m_scene->addItem(item);
        m_cardDisplayItems.append(item);
    }

    m_confirmBtn = new TextButton("确认升级", 200, 55);
    m_confirmBtn->setPos(1650, 940);
    m_confirmBtn->setZValue(300);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
    connect(m_confirmBtn, &TextButton::clicked, this, &CampfireView::confirmUpgrade);

    m_cancelBtn = new TextButton("返回", 200, 55);
    m_cancelBtn->setPos(1400, 940);
    m_cancelBtn->setZValue(300);
    m_scene->addItem(m_cancelBtn);
    connect(m_cancelBtn, &TextButton::clicked, this, &CampfireView::cancelUpgrade);
}


void CampfireView::confirmUpgrade() {
    if (!m_selectedCard) return;

    for (auto* item : m_cardDisplayItems) {
        item->hide();
    }

    if (m_cardSelectPrompt) { m_scene->removeItem(m_cardSelectPrompt); delete m_cardSelectPrompt; m_cardSelectPrompt = nullptr; }
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

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

    hideDarkOverlay();

    m_restBtn->show();
    m_upgradeBtn->show();
    m_restLabel->show();
    m_upgradeLabel->show();
    m_promptText->show();
    m_choiceCloud->show();
    setLeaveButtonVisible(true);
}

void CampfireView::runUpgradeAnimation(Card* card) {
    auto* oldAnimCard = new CardItem(card);
    oldAnimCard->setPos(960, 450);
    oldAnimCard->setZValue(110);
    oldAnimCard->setDisplayOnly(true);
    m_scene->addItem(oldAnimCard);

    auto* phase1 = new QSequentialAnimationGroup(this);

    auto* s1 = new QPropertyAnimation(oldAnimCard, "scale");
    s1->setDuration(300);
    s1->setEndValue(1.15);
    s1->setEasingCurve(QEasingCurve::OutQuad);
    phase1->addAnimation(s1);

    auto* shake = new QVariantAnimation(this);
    shake->setDuration(150);
    shake->setKeyValueAt(0.0, QPointF(960, 450));
    shake->setKeyValueAt(0.25, QPointF(950, 455));
    shake->setKeyValueAt(0.5, QPointF(970, 445));
    shake->setKeyValueAt(0.75, QPointF(955, 452));
    shake->setKeyValueAt(1.0, QPointF(960, 450));
    QObject::connect(shake, &QVariantAnimation::valueChanged, this, [oldAnimCard](const QVariant& v) {
        oldAnimCard->setPos(v.toPointF());
    });
    phase1->addAnimation(shake);

    auto* burstAndShrink = new QParallelAnimationGroup(this);

    for (int i = 0; i < 40; ++i) {
        qreal size = 6 + QRandomGenerator::global()->bounded(8);
        auto* p = new QGraphicsEllipseItem(-size / 2, -size / 2, size, size, nullptr);

        QColor fireColor = i % 3 == 0 ? QColor(255, 200, 50, 255)
                           : i % 3 == 1 ? QColor(255, 100, 20, 255)
                                        : QColor(255, 50, 10, 220);
        p->setBrush(fireColor);
        p->setPen(Qt::NoPen);
        p->setPos(960, 450);
        p->setZValue(115);
        m_scene->addItem(p);

        double angle = (2.0 * M_PI * i) / 40.0 + (QRandomGenerator::global()->bounded(10) / 100.0);
        qreal radius = 120 + QRandomGenerator::global()->bounded(200);
        QPointF target(960 + cos(angle) * radius, 450 + sin(angle) * radius + 50);

        int duration = 500 + QRandomGenerator::global()->bounded(300);

        auto* pMove = new QVariantAnimation(this);
        pMove->setDuration(duration);
        pMove->setStartValue(QPointF(960, 450));
        pMove->setEndValue(target);
        pMove->setEasingCurve(QEasingCurve::OutCirc);
        QObject::connect(pMove, &QVariantAnimation::valueChanged, this, [p](const QVariant& v) {
            p->setPos(v.toPointF());
        });

        auto* pFadeEff = new QGraphicsOpacityEffect();
        p->setGraphicsEffect(pFadeEff);
        auto* pOpacity = new QVariantAnimation(this);
        pOpacity->setDuration(duration);
        pOpacity->setStartValue(1.0);
        pOpacity->setEndValue(0.0);
        QObject::connect(pOpacity, &QVariantAnimation::valueChanged, this, [pFadeEff](const QVariant& v) {
            pFadeEff->setOpacity(v.toReal());
        });

        auto* pScale = new QVariantAnimation(this);
        pScale->setDuration(duration);
        pScale->setStartValue(1.0);
        pScale->setEndValue(0.1);
        QObject::connect(pScale, &QVariantAnimation::valueChanged, this, [p](const QVariant& v) {
            p->setScale(v.toReal());
        });

        auto* singleParticleAnim = new QParallelAnimationGroup(this);
        singleParticleAnim->addAnimation(pMove);
        singleParticleAnim->addAnimation(pOpacity);
        singleParticleAnim->addAnimation(pScale);
        QObject::connect(singleParticleAnim, &QAbstractAnimation::finished, this, [p]() { delete p; });

        burstAndShrink->addAnimation(singleParticleAnim);
    }

    auto* shrink = new QPropertyAnimation(oldAnimCard, "scale");
    shrink->setDuration(400);
    shrink->setEndValue(0.0);
    shrink->setEasingCurve(QEasingCurve::InBack);
    burstAndShrink->addAnimation(shrink);

    phase1->addAnimation(burstAndShrink);
    phase1->addPause(150);

    QObject::connect(phase1, &QAbstractAnimation::finished, this, [this, card, oldAnimCard]() {
        m_scene->removeItem(oldAnimCard);
        delete oldAnimCard;

        card->upgrade();
        QString oldId = card->property("original_id").toString();
        QString newId = card->getId();
        GlobalSaveData* save = GlobalSaveData::getInstance();
        int index = save->deckIds.indexOf(oldId);
        if (index != -1) save->deckIds[index] = newId;

        auto* newAnimCard = new CardItem(card);
        newAnimCard->setPos(960, 450);
        newAnimCard->setZValue(120);
        newAnimCard->setDisplayOnly(true);
        newAnimCard->setScale(0.0);
        m_scene->addItem(newAnimCard);

        auto* phase2 = new QSequentialAnimationGroup(this);

        auto* popOut = new QPropertyAnimation(newAnimCard, "scale");
        popOut->setDuration(500);
        popOut->setStartValue(0.0);
        popOut->setEndValue(1.3);
        popOut->setEasingCurve(QEasingCurve::OutElastic);
        phase2->addAnimation(popOut);

        phase2->addPause(1200);

        auto* flyAway = new QParallelAnimationGroup(this);

        auto* flyMove = new QPropertyAnimation(newAnimCard, "pos");
        flyMove->setDuration(600);
        // 🔴 动态终点：映射牌堆全局坐标到场景坐标
        GameWindow* gw = qobject_cast<GameWindow*>(this->window());
        QPointF flyTarget;
        if (gw) {
            QPoint globalPos = gw->deckPileGlobalPos().toPoint();
            flyTarget = this->mapToScene(this->mapFromParent(globalPos));
        } else {
            flyTarget = QPointF(1750, 60);
        }
        flyMove->setEndValue(flyTarget);
        flyMove->setEasingCurve(QEasingCurve::InBack);
        flyAway->addAnimation(flyMove);

        auto* flyShrink = new QPropertyAnimation(newAnimCard, "scale");
        flyShrink->setDuration(600);
        flyShrink->setEndValue(0.0);
        flyAway->addAnimation(flyShrink);

        phase2->addAnimation(flyAway);

        QObject::connect(phase2, &QAbstractAnimation::finished, this, [this, newAnimCard]() {
            m_scene->removeItem(newAnimCard);
            delete newAnimCard;

            qDebug() << "[Campfire] 飞卡入库！发出更新信号！";
            emit deckUpdated();

            if (m_overlayText) {
                m_overlayText->setPlainText("升级成功！");
                m_overlayText->show();
                m_overlayText->setPos(960 - m_overlayText->boundingRect().width() / 2, 400);
            }

            QTimer::singleShot(800, this, [this]() {
                hideDarkOverlay();
                m_restBtn->hide(); m_upgradeBtn->hide();
                m_restLabel->hide(); m_upgradeLabel->hide();
                m_promptText->hide(); m_choiceCloud->hide();
                if (m_leaveBtn) {
                    m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
                    m_leaveBtn->show();
                }
            });
        });

        phase2->start(QAbstractAnimation::DeleteWhenStopped);
    });

    phase1->start(QAbstractAnimation::DeleteWhenStopped);
}

bool CampfireView::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        CardItem* clickedCard = qobject_cast<CardItem*>(obj);
        if (clickedCard && m_cardDisplayItems.contains(clickedCard)) {
            m_selectedCard = clickedCard->getLogicCard();

            for (auto* c : m_cardDisplayItems) {
                CardItem* item = static_cast<CardItem*>(c);

                if (item != clickedCard && item->property("ui_selected").toBool()) {
                    item->setProperty("ui_selected", false);
                    item->setZValue(160);
                    item->update();

                    QPropertyAnimation* shrinkAnim = new QPropertyAnimation(item, "scale");
                    shrinkAnim->setDuration(200);
                    shrinkAnim->setEndValue(1.0);
                    shrinkAnim->setEasingCurve(QEasingCurve::OutCubic);
                    shrinkAnim->start(QAbstractAnimation::DeleteWhenStopped);
                }
            }

            if (!clickedCard->property("ui_selected").toBool()) {
                clickedCard->setProperty("ui_selected", true);
                clickedCard->setZValue(170);
                clickedCard->update();

                QPropertyAnimation* popAnim = new QPropertyAnimation(clickedCard, "scale");
                popAnim->setDuration(250);
                popAnim->setEndValue(1.2);
                popAnim->setEasingCurve(QEasingCurve::OutBack);
                popAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }

            if (m_confirmBtn) m_confirmBtn->show();
            return true;
        }
    }
    return EventBaseView::eventFilter(obj, event);
}

void CampfireView::wheelEvent(QWheelEvent *event) {
    if (m_cardDisplayItems.isEmpty()) return;

    const int maxCols = 5;
    const int cardH = 300;
    int totalRows = (m_cardDisplayItems.size() + maxCols - 1) / maxCols;

    if (totalRows <= 2) return;

    qreal scrollStep = (event->angleDelta().y() > 0) ? 60.0 : -60.0;

    QGraphicsObject* firstCard = m_cardDisplayItems.first();
    qreal currentTopY = firstCard->pos().y();
    qreal originalTopY = 380.0;

    qreal maxUpwardMovement = (totalRows - 2) * cardH + 50;
    qreal minY = originalTopY - maxUpwardMovement;
    qreal maxY = originalTopY;

    qreal newTopY = currentTopY + scrollStep;
    if (newTopY > maxY) {
        newTopY = maxY;
    } else if (newTopY < minY) {
        newTopY = minY;
    }

    qreal actualStep = newTopY - currentTopY;

    if (qAbs(actualStep) < 0.1) return;

    for (auto* item : m_cardDisplayItems) {
        item->moveBy(0, actualStep);
    }
}