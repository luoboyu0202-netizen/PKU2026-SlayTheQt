#include "BattleView.h"
#include <QDebug>
#include "PlayerItem.h"
#include "ShuffleEffectItem.h"
#include <QPropertyAnimation>
#include <QTimer>
#include <QMouseEvent>
#include "CardBrowserOverlay.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>

BattleView::BattleView(QWidget* parent)
    : QGraphicsView(parent), m_layoutManager(nullptr){

    // 1. 初始化场景尺寸
    m_scene = new BattleScene(this);
    m_scene->setSceneRect(0, 0, 1920, 1080);
    setScene(m_scene);

    // ====================================================
    // 🔴【核心修正组合拳】：彻底解决窗口对齐与裁切 Bug！
    // ====================================================
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setFixedSize(1600, 900);
    this->setStyleSheet("background-color: black;");

    // 2. 禁用滚动条，保持游戏画面的纯净度
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    // 3. 骨架搭起：铺好一成不变的 UI
    initStageInfrastructure();
}

void BattleView::playShuffleAnimation() {
    QPointF startPos = m_discardPileUI->scenePos();
    QPointF endPos = m_drawPileUI->scenePos();

    for (int i = 0; i < 15; ++i) {
        QTimer::singleShot(i * 40, this, [this, startPos, endPos]() {
            ShuffleEffectItem* goldSquare = new ShuffleEffectItem(startPos, endPos);
            m_scene->addItem(goldSquare);

            QPropertyAnimation* anim = new QPropertyAnimation(goldSquare, "progress");
            anim->setDuration(500);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::InOutQuad);

            connect(anim, &QPropertyAnimation::finished, goldSquare, &QObject::deleteLater);
            connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
            anim->start();
        });
    }
}

void BattleView::initStageInfrastructure() {
    // ========================================================
    // 🗑️ 喵娘注：这里的 TopBar 和 RelicTray 已经被提拔到了全局！
    // BattleView 不再负责生成它们，保持了绝对的低耦合！
    // ========================================================

    m_drawPileUI = new PileItem(QStringLiteral("抽牌堆"));
    m_drawPileUI->setPos(80, 1000);
    m_scene->addItem(m_drawPileUI);

    m_discardPileUI = new PileItem(QStringLiteral("弃牌堆"));
    m_discardPileUI->setPos(1850, 1000);
    m_scene->addItem(m_discardPileUI);

    m_exhaustPileUI = new PileItem("消耗堆");
    m_exhaustPileUI->setPos(1850, 800);
    m_exhaustPileUI->setZValue(50);
    m_exhaustPileUI->hide();
    m_scene->addItem(m_exhaustPileUI);

    m_endTurnBtn = new EndTurnButton();
    m_endTurnBtn->setPos(1600, 850);
    m_scene->addItem(m_endTurnBtn);

    m_energyBall = new EnergyWidget();
    m_energyBall->setPos(400, 840);
    m_scene->addItem(m_energyBall);

    m_darkOverlay = new QGraphicsRectItem(0, 0, 1920, 1080);
    m_darkOverlay->setBrush(QColor(0, 0, 0, 180));
    m_darkOverlay->setZValue(100);
    m_darkOverlay->hide();
    m_scene->addItem(m_darkOverlay);

    m_promptTextItem = new QGraphicsTextItem();
    m_promptTextItem->setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
    m_promptTextItem->setDefaultTextColor(Qt::white);
    m_promptTextItem->setZValue(101);
    m_promptTextItem->hide();
    m_scene->addItem(m_promptTextItem);

    m_confirmBtn = new ConfirmButton();
    m_confirmBtn->setPos(1500, 750);
    m_confirmBtn->setZValue(102);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
}

void BattleView::bindEngine(BattleEngine* engine) {
    if (!engine) return;

    m_engine = engine;
    QString bgPath = m_engine->getBackgroundPath();
    qDebug() << "[Diagnostics - UI] 大脑传来的背景路径是:" << bgPath;

    if (!bgPath.isEmpty()) {
        QPixmap bgPixmap(bgPath);
        if (!bgPixmap.isNull()) {
            bgPixmap = bgPixmap.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(bgPixmap);
            bgItem->setZValue(-1000);
            bgItem->setPos(0, 0);
            m_scene->addItem(bgItem);
            qDebug() << "[Diagnostics - UI] 成功绘制背景图片！";
        } else {
            qDebug() << "[UI Architecture] 警告：背景图加载失败，路径：" << bgPath;
        }
    } else {
        qDebug() << "[UI Architecture] 致命警告：路径是空的！一定是 Launcher 没传进来或者 Engine 没存住！";
    }

    Player* playerLogic = m_engine->getPlayer();

    if (m_playerItem) {
        if (m_playerItem->scene() != nullptr) {
            m_scene->removeItem(m_playerItem);
        }
        m_playerItem->deleteLater();
        m_playerItem = nullptr;
    }

    m_playerItem = new PlayerItem(playerLogic);
    m_playerItem->setPos(400, 620);
    m_scene->addItem(m_playerItem);

    CardManager* cardManager = engine->getCardManager();

    for (EnemyItem* oldItem : m_enemyItems) {
        if (oldItem->scene()) { m_scene->removeItem(oldItem); }
        oldItem->deleteLater();
    }
    m_enemyItems.clear();

    const QList<Enemy*>& enemies = m_engine->getEnemies();

    for (int i = 0; i < enemies.size(); ++i) {
        Enemy* enemyLogic = enemies[i];

        int spriteYOffset = 0;
        QString id = enemyLogic->getId();

        if (id == "Slime_Small") spriteYOffset = 60;
        else if (id == "Slime_01") spriteYOffset = 10;
        else if (id == "Flying_Bat") spriteYOffset = -40;

        EnemyItem* enemyItem = new EnemyItem(enemyLogic, m_engine, spriteYOffset);

        int slot = enemyLogic->getSlotIndex();
        if (slot < 0 || slot > 3) slot = 0;

        enemyItem->setPos(MONSTER_START_X + (slot * MONSTER_SPACING), UNIFIED_BASE_Y);

        m_scene->addItem(enemyItem);
        m_enemyItems.append(enemyItem);

        connect(enemyLogic, &Enemy::died, this, [this, enemyItem]() {
            m_enemyItems.removeOne(enemyItem);
            // if (enemyItem->scene()) {
            //     m_scene->removeItem(enemyItem);
            // }
            // enemyItem->deleteLater();
        });
    }

    m_energyBall->bindPlayer(playerLogic);

    if (m_layoutManager) { delete m_layoutManager; }
    m_layoutManager = new HandLayoutManager(m_scene, playerLogic, cardManager, this);

    connect(m_layoutManager, &HandLayoutManager::cardPlayedRequest,
            m_engine, &BattleEngine::playCard,
            Qt::UniqueConnection);

    connect(cardManager, &CardManager::pileCountsChanged, this, [this](int drawCount, int discardCount, int exhaustCount) {
        m_drawPileUI->updateCount(drawCount);
        m_discardPileUI->updateCount(discardCount);
        m_exhaustPileUI->updateCount(exhaustCount);

        if (exhaustCount > 0) {
            if (!m_exhaustPileUI->isVisible()) {
                m_exhaustPileUI->show();
            }
        } else {
            m_exhaustPileUI->hide();
        }
    });

    connect(m_endTurnBtn, &EndTurnButton::clicked, this, [engine](){
        engine->endPlayerTurn();
    });

    connect(cardManager, &CardManager::deckShuffled, this, &BattleView::playShuffleAnimation);

    qDebug() << "[UI Architecture] BattleEngine successfully wired up to BattleView with multiple enemies!";

    connect(engine, &BattleEngine::selectionModeStarted, this, [this](const QString& text) {
        m_darkOverlay->show();
        m_promptTextItem->setPlainText(text);
        qreal textWidth = m_promptTextItem->boundingRect().width();
        m_promptTextItem->setPos((1920 - textWidth) / 2.0, 300);
        m_promptTextItem->show();
        m_confirmBtn->setValid(false);
        m_confirmBtn->show();
    });

    connect(engine, &BattleEngine::selectionValidityChanged, this, [this](bool canConfirm) {
        m_confirmBtn->setValid(canConfirm);
    });

    connect(m_confirmBtn, &ConfirmButton::clicked, engine, &BattleEngine::confirmHandSelection);

    connect(engine, &BattleEngine::selectionModeEnded, this, [this]() {
        m_darkOverlay->hide();
        m_promptTextItem->hide();
        m_confirmBtn->hide();
    });

    connect(m_exhaustPileUI, &PileItem::clicked, this, [this, engine]() {
        qDebug() << "[UI] 玩家点开了消耗堆喵！";
        const QList<Card*>& exhaustCards = engine->getCardManager()->getExhaustPile();
        if (exhaustCards.isEmpty()) return;

        CardBrowserOverlay* browser = new CardBrowserOverlay(exhaustCards, "消耗牌堆");
        m_scene->addItem(browser);
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    connect(m_drawPileUI, &PileItem::clicked, this, [this, engine]() {
        const QList<Card*>& drawCards = engine->getCardManager()->getDrawPile();
        if (drawCards.isEmpty()) return;
        CardBrowserOverlay* browser = new CardBrowserOverlay(drawCards, "抽牌堆");
        m_scene->addItem(browser);
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    connect(m_discardPileUI, &PileItem::clicked, this, [this, engine]() {
        const QList<Card*>& discardCards = engine->getCardManager()->getDiscardPile();
        if (discardCards.isEmpty()) return;
        CardBrowserOverlay* browser = new CardBrowserOverlay(discardCards, "弃牌堆");
        m_scene->addItem(browser);
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    connect(engine, &BattleEngine::enemySummoned, this, [this, engine](Enemy* newEnemy) {
        qDebug() << "[UI] 接收到召唤信号，准备生成肉体：" << newEnemy->getName();
        int spriteYOffset = (newEnemy->getId() == "Slime_Small") ? 60 : 0;
        EnemyItem* newItem = new EnemyItem(newEnemy, engine, spriteYOffset);

        int slot = newEnemy->getSlotIndex();
        newItem->setPos(MONSTER_START_X + (slot * MONSTER_SPACING), UNIFIED_BASE_Y);

        m_scene->addItem(newItem);
        m_enemyItems.append(newItem);

        connect(newEnemy, &Enemy::died, this, [this, newItem]() {
            m_enemyItems.removeOne(newItem);
            // if (newItem->scene()) {
            //     m_scene->removeItem(newItem);
            // }
            // newItem->deleteLater();
        });

        newItem->setScale(0.1);
        QPropertyAnimation* popAnim = new QPropertyAnimation(newItem, "scale");
        popAnim->setDuration(500);
        popAnim->setEndValue(1.0);
        popAnim->setEasingCurve(QEasingCurve::OutBack);
        popAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    connect(cardManager, &CardManager::cardInsertedToDiscard, this, [this](Card* card) {
        CardItem* visualCard = new CardItem(card);
        visualCard->setGhostMode(true);
        m_scene->addItem(visualCard);

        visualCard->setPos(1920 / 2 - 100, 1080 / 2 - 200);
        visualCard->setZValue(1000);

        QParallelAnimationGroup* group = new QParallelAnimationGroup(visualCard);

        QPropertyAnimation* flyAnim = new QPropertyAnimation(visualCard, "pos");
        flyAnim->setDuration(300);
        flyAnim->setEndValue(m_discardPileUI->pos());
        flyAnim->setEasingCurve(QEasingCurve::InQuad);

        QPropertyAnimation* scaleAnim = new QPropertyAnimation(visualCard, "scale");
        scaleAnim->setDuration(300);
        scaleAnim->setStartValue(0.5);
        scaleAnim->setKeyValueAt(0.3, 1.2);
        scaleAnim->setEndValue(0.01);

        group->addAnimation(flyAnim);
        group->addAnimation(scaleAnim);

        connect(group, &QParallelAnimationGroup::finished, visualCard, &QObject::deleteLater);
        group->start(QAbstractAnimation::DeleteWhenStopped);
    });

    connect(engine, &BattleEngine::powerActivated, this, [this](Card* card, Fighter* source) {
        CardItem* ghostCard = new CardItem(card);
        ghostCard->setGhostMode(true);
        m_scene->addItem(ghostCard);

        ghostCard->setPos(1920 / 2 - 100, 1080 / 2 - 150);
        ghostCard->setZValue(2000);
        ghostCard->setScale(1.2);

        QSequentialAnimationGroup* group = new QSequentialAnimationGroup(ghostCard);

        QPropertyAnimation* focus = new QPropertyAnimation(ghostCard, "scale");
        focus->setDuration(300);
        focus->setEndValue(1.3);
        focus->setEasingCurve(QEasingCurve::OutBack);

        QPropertyAnimation* fly = new QPropertyAnimation(ghostCard, "pos");
        fly->setDuration(500);
        fly->setEndValue(m_playerItem->pos() + QPointF(0, -100));
        fly->setEasingCurve(QEasingCurve::InBack);

        QPropertyAnimation* shrink = new QPropertyAnimation(ghostCard, "scale");
        shrink->setDuration(500);
        shrink->setEndValue(0.1);

        group->addPause(200);
        group->addAnimation(focus);

        QParallelAnimationGroup* rushGroup = new QParallelAnimationGroup(group);
        rushGroup->addAnimation(fly);
        rushGroup->addAnimation(shrink);
        group->addAnimation(rushGroup);

        connect(group, &QSequentialAnimationGroup::finished, [this, ghostCard]() {
            ghostCard->deleteLater();

            QPropertyAnimation* glow = new QPropertyAnimation(m_playerItem, "scale");
            glow->setDuration(200);
            glow->setKeyValueAt(0, 1.0);
            glow->setKeyValueAt(0.5, 1.15);
            glow->setKeyValueAt(1, 1.0);
            glow->start(QAbstractAnimation::DeleteWhenStopped);
        });

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });

    connect(engine->getPlayer()->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType type, int amount) {
        Q_UNUSED(type); Q_UNUSED(amount);

        if (m_layoutManager) {
            for (CardItem* cItem : m_layoutManager->getHandItems()) {
                if (cItem) {
                    cItem->update();
                }
            }
        }
        qDebug() << "[UI] 主角状态巨变！全场手牌强制刷新！";
    });

    connect(engine, &BattleEngine::topCardRevealed, this, [this, engine](Card* card, bool exhaustIt) {
        CardItem* ghostCard = new CardItem(card);
        ghostCard->setGhostMode(true);
        m_scene->addItem(ghostCard);

        ghostCard->setPos(m_drawPileUI->pos());
        ghostCard->setZValue(3000);

        QParallelAnimationGroup* popGroup = new QParallelAnimationGroup(ghostCard);

        QPropertyAnimation* flyCenter = new QPropertyAnimation(ghostCard, "pos");
        flyCenter->setDuration(400);
        flyCenter->setEndValue(QPointF(1920 / 2.0 - 100, 1080 / 2.0 - 150));
        flyCenter->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation* scaleUp = new QPropertyAnimation(ghostCard, "scale");
        scaleUp->setDuration(400);
        scaleUp->setEndValue(1.5);

        popGroup->addAnimation(flyCenter);
        popGroup->addAnimation(scaleUp);

        connect(popGroup, &QParallelAnimationGroup::finished, [this, engine, ghostCard, card, exhaustIt]() {
            QTimer::singleShot(800, [this, engine, ghostCard, card, exhaustIt]() {

                engine->executeRevealedCard(card, exhaustIt);

                if (card->getType() == CardType::Power) {
                    ghostCard->deleteLater();
                }
                else {
                    QPropertyAnimation* flyAway = new QPropertyAnimation(ghostCard, "pos");
                    flyAway->setDuration(300);

                    bool willExhaust = exhaustIt || card->isExhaustOnUse();
                    flyAway->setEndValue(willExhaust ? m_exhaustPileUI->pos() : m_discardPileUI->pos());
                    flyAway->setEasingCurve(QEasingCurve::InBack);

                    QPropertyAnimation* scaleDown = new QPropertyAnimation(ghostCard, "scale");
                    scaleDown->setDuration(300);
                    scaleDown->setEndValue(0.1);

                    QParallelAnimationGroup* endGroup = new QParallelAnimationGroup(ghostCard);
                    endGroup->addAnimation(flyAway);
                    endGroup->addAnimation(scaleDown);

                    connect(endGroup, &QParallelAnimationGroup::finished, ghostCard, &QObject::deleteLater);
                    endGroup->start(QAbstractAnimation::DeleteWhenStopped);
                }
            });
        });

        popGroup->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void BattleView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (m_scene) {
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

