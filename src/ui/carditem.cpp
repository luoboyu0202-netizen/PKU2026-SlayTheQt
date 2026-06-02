#include "CardItem.h"
#include "BattleScene.h"
#include "EnemyItem.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "battleengine.h"
#include <QGraphicsOpacityEffect>
#include <random>
#include <QVariantAnimation>
#include <QTextDocument>

// 构造函数初始化
CardItem::CardItem(Card* logicCard, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_logicCard(logicCard),
    m_homeRotation(0.0), m_isHovered(false), m_isDragging(false),
    m_isPlayed(false), m_isPlayable(true),
    m_currentTargetedEnemy(nullptr) {

    setAcceptHoverEvents(true);

    // 🔴 加载卡牌立绘！
    if (m_logicCard && !m_logicCard->getImagePath().isEmpty()) {
        m_cardPixmap.load(m_logicCard->getImagePath());
    }
}

void CardItem::checkPlayability(int currentEnergy) {
    bool bright = true;
    BattleEngine* engine = BattleEngine::getInstance();

    // ========================================================
    // 👑【至高无上优先级】：如果是时停选牌阶段，所有牌都是潜在祭品！
    // ========================================================
    if (engine && engine->isSelectingHandCard()) {
        bright = true;
    }
    // 常规战斗阶段
    else if (m_logicCard->isUnplayable()) {
        bright = true;
    } else {
        bright = (currentEnergy >= m_logicCard->getCost());
    }

    // 刷新视觉与交互闸门
    if (m_isPlayable != bright) {
        m_isPlayable = bright;
        update();
    }
}

void CardItem::setHomeState(const QPointF& pos, qreal rotation) {
    m_homePos = pos;
    m_homeRotation = rotation;
}

void CardItem::animateToHome() {
    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(200);
    posAnim->setEndValue(m_homePos);
    posAnim->setEasingCurve(QEasingCurve::OutQuad);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* rotAnim = new QPropertyAnimation(this, "rotation");
    rotAnim->setDuration(200);
    rotAnim->setEndValue(m_homeRotation);
    rotAnim->setEasingCurve(QEasingCurve::OutQuad);
    rotAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // ========================================================
    // 🔴 修复 2：卡牌变小的元凶！必须把它强行拉回 1.0 倍大小！
    // ========================================================
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(200);
    scaleAnim->setEndValue(m_baseScale);
    scaleAnim->setEasingCurve(QEasingCurve::OutQuad);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF CardItem::boundingRect() const {
    return QRectF(-120, -150, 240, 300);
}

void CardItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    if (m_logicCard == nullptr) {
        qDebug() << "[CRASH-TEST] 🚨 致命错误：CardItem 在尝试 Paint 时，m_logicCard 丢失了！";
        return;
    }

    if (!m_logicCard) return;

    QRectF cardRect(-75, -105, 150, 210);

    // ========================================================
    // 🎨 1. 画卡牌底板与边框
    // ========================================================
    QColor bgColor(40, 40, 45);
    QColor borderColor;
    switch (m_logicCard->getType()) {
    case CardType::Attack: borderColor = QColor(220, 50, 50); break;
    case CardType::Skill:  borderColor = QColor(50, 200, 100); break;
    case CardType::Power:  borderColor = QColor(50, 150, 250); break;
    default: borderColor = Qt::gray;
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(bgColor);
    painter->setPen(QPen(borderColor, 3));
    painter->drawRoundedRect(cardRect, 8, 8);

    // 展示模式下的“黄金选中框”
    if (property("ui_selected").toBool()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255, 215, 0), 6, Qt::SolidLine));
        painter->drawRoundedRect(cardRect, 8, 8);
    }

    // ========================================================
    // 🎨 2. 画卡牌上半部分的立绘！
    // ========================================================
    QRectF imageRect(-65, -95, 130, 90);
    if (!m_cardPixmap.isNull()) {
        painter->drawPixmap(imageRect.toRect(), m_cardPixmap);
    } else {
        painter->setBrush(Qt::darkGray);
        painter->drawRect(imageRect);
    }

    // ========================================================
    // 🟢 绘制卡牌名字（包含升级强化绿色光效）
    // ========================================================
    QRectF nameBannerRect(-75, 0, 150, 25);
    QFont nameFont("Microsoft YaHei", 12, QFont::Bold);
    painter->setFont(nameFont);

    if (m_logicCard->isUpgraded()) {
        painter->setPen(QColor(100, 255, 100));
    } else {
        painter->setPen(Qt::white);
    }
    painter->drawText(nameBannerRect, Qt::AlignCenter, m_logicCard->getName());

    // ========================================================
    // 🎨 4. 画下半部分的描述文字！(富文本自动换行)
    // ========================================================
    QRectF descRect(-65, 35, 130, 60);
    painter->setPen(Qt::lightGray);
    painter->setFont(QFont("Microsoft YaHei", 9));

    Player* player = nullptr;
    if (!m_isDisplayOnly) {
        if (BattleEngine::getInstance()) {
            player = BattleEngine::getInstance()->getPlayer();
        }
    }

    Fighter* logicTarget = nullptr;
    if (m_currentTargetedEnemy) {
        logicTarget = m_currentTargetedEnemy->getLogicEnemy();
    }

    QString dynamicDesc = m_logicCard->getDynamicDescription(player, logicTarget);

    QTextDocument doc;
    doc.setDocumentMargin(0);
    doc.setDefaultFont(QFont("Microsoft YaHei", 9));

    QString htmlStr = QString("<div align='center' style='color: white;'>%1</div>").arg(dynamicDesc);
    doc.setHtml(htmlStr);
    doc.setTextWidth(descRect.width());

    painter->save();
    painter->translate(descRect.topLeft());
    doc.drawContents(painter);
    painter->restore();

    if (!m_isPlayable && !m_logicCard->isUnplayable()) {
        painter->setBrush(QColor(0, 0, 0, 150));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(cardRect, 12, 12);
    }

    // ========================================================
    // ⚔️ 终极斩杀冲突：完美保留你的断网免死金牌围墙！
    // ========================================================
    if (!m_isDisplayOnly) {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->isSelectingHandCard()) {
            if (engine->getSelectedCards().contains(m_logicCard)) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(241, 196, 15), 4, Qt::SolidLine));
                painter->drawRoundedRect(cardRect, 8, 8);
            }
        }
    }

    // 商店等高亮模式
    if (m_isHighlighted) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(241, 196, 15), 4, Qt::SolidLine));
        painter->drawRoundedRect(cardRect, 8, 8);
    }

    // ========================================================
    // 🎨 5. 画左上角的能量(Cost)球！
    // ========================================================
    if (m_logicCard->getCost() >= 0) {
        QString costStr;
        if (m_logicCard->isXCost()) {
            costStr = "X";
        } else {
            costStr = QString::number(m_logicCard->getCost());
        }

        QRectF costRect(-90, -120, 32, 32);
        QRadialGradient orbGradient(costRect.center(), 16, costRect.topLeft() + QPointF(8, 8));
        orbGradient.setColorAt(0.0, QColor(100, 200, 255));
        orbGradient.setColorAt(0.8, QColor(30, 100, 200));
        orbGradient.setColorAt(1.0, QColor(10, 50, 100));

        painter->setBrush(orbGradient);
        painter->setPen(QPen(QColor(20, 25, 30), 2));
        painter->drawEllipse(costRect);

        painter->setFont(QFont("Arial", 14, QFont::Bold));
        painter->setPen(QColor(0, 0, 0, 180));
        painter->drawText(costRect.translated(1.5, 1.5), Qt::AlignCenter, costStr);

        if (!m_isPlayable && !m_isDisplayOnly) {
            painter->setPen(QColor(255, 80, 80));
        }
        else if (m_logicCard->isCostModified()) {
            painter->setPen(QColor(138, 226, 52));
        }
        else {
            painter->setPen(Qt::white);
        }
        painter->drawText(costRect, Qt::AlignCenter, costStr);
    }

    // ========================================================
    // 💰 6. 商店价签系统 (完美融合队友功能)
    // ========================================================
    if (m_price > 0) {
        QRectF priceRect(-45, cardRect.bottom() - 24, 90, 22);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(30, 30, 30, 220));
        painter->drawRoundedRect(priceRect, 4, 4);

        QColor priceColor = m_isAffordable ? QColor(255, 215, 0) : QColor(220, 50, 50);

        if (m_isOnSale) {
            int originalPrice = m_price * 2;
            painter->setPen(QColor(130, 130, 130));
            painter->setFont(QFont("Microsoft YaHei", 8));
            QRectF origRect(priceRect.x(), priceRect.y() - 1, priceRect.width(), priceRect.height() / 2);
            painter->drawText(origRect, Qt::AlignCenter, QString::number(originalPrice) + "g");

            painter->setPen(QPen(QColor(220, 50, 50), 1.5));
            qreal textW = painter->fontMetrics().horizontalAdvance(QString::number(originalPrice) + "g");
            qreal lineY = origRect.center().y();
            painter->drawLine(origRect.center().x() - textW/2 - 2, lineY,
                              origRect.center().x() + textW/2 + 2, lineY);

            painter->setPen(priceColor);
            painter->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
            painter->drawText(QRectF(priceRect.x(), priceRect.y() + priceRect.height() / 2 - 2,
                                     priceRect.width(), priceRect.height() / 2 + 2),
                              Qt::AlignCenter, QString::number(m_price) + "g");
        } else {
            painter->setPen(priceColor);
            painter->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
            painter->drawText(priceRect, Qt::AlignCenter, QString::number(m_price) + "g");
        }
    }
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    if (m_isDisplayOnly) {
        if (!m_isHovered) {
            m_defaultZ = zValue(); // 记录当前真实的 Z 值 (营火是 160，商店是 10)
            m_isHovered = true;
        }
        setZValue(200);
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
        scaleAnim->setEndValue(m_baseScale * 1.5);
        scaleAnim->setDuration(150);
        scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    if (m_isGhost) {
        event->ignore();
        return;
    }

    if (m_isPlayed || m_isDragging) return;
    if (!m_isPlayable && !m_logicCard->isUnplayable()) return;
    if (m_isSuspended) return;

    if (!m_isHovered) {
        m_defaultZ = zValue();
    }

    m_isHovered = true;
    setZValue(m_defaultZ + 100.0);
    setRotation(0.0);

    if (m_homePos.isNull() && !pos().isNull()) {
        m_homePos = pos();
    }

    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(150);
    posAnim->setEndValue(m_homePos + QPointF(0, -40));
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(150);
    scaleAnim->setEndValue(m_baseScale * 1.25);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    if (m_isDisplayOnly) {
        if (property("ui_selected").toBool()) return;
        m_isHovered = false;
        setZValue(m_defaultZ); // 绝对不再硬编码 160，完美适配所有场景！
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
        scaleAnim->setEndValue(m_baseScale);
        scaleAnim->setDuration(150);
        scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    if (m_isGhost) {
        event->ignore();
        return;
    }

    BattleEngine* engine = BattleEngine::getInstance();
    if (m_isSuspended) return;

    if (engine && engine->isSelectingHandCard()) {
        if (engine->getSelectedCards().contains(m_logicCard)) {
            return;
        }
    }

    if (m_isPlayed || m_isDragging) return;
    if (!m_isPlayable && !m_logicCard->isUnplayable()) return;

    m_isHovered = false;
    setZValue(m_defaultZ);

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(150);
    scaleAnim->setEndValue(m_baseScale);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

    animateToHome();
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isSelectionEnabled && event->button() == Qt::LeftButton) {
        emit cardClicked(this);
        event->accept();
        return;
    }

    BattleEngine* engine = BattleEngine::getInstance();
    if (m_isGhost) {
        event->ignore();
        return;
    }
    if (m_isDisplayOnly) {
        event->accept();
        return;
    }

    if (engine && engine->isSelectingHandCard()) {
        if (event->button() == Qt::LeftButton) {
            bool alreadySelected = engine->getSelectedCards().contains(m_logicCard);
            if (!alreadySelected && engine->getSelectedCards().size() >= engine->getSelectionLimit()) {
                event->accept();
                return;
            }

            engine->toggleCardSelection(m_logicCard);

            if (!alreadySelected) {
                qDebug() << "[UI] 选中了祭品，弹起：" << m_logicCard->getName();
                setPos(pos().x(), pos().y() - 40);
            } else {
                qDebug() << "[UI] 取消了祭品，缩回：" << m_logicCard->getName();
                setPos(pos().x(), pos().y() + 40);
            }
            update();
        }
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_logicCard->isUnplayable()) {
            event->accept();
            return;
        }

        if (!m_isPlayable) return;

        m_isDragging = true;
        setZValue(200);
        m_currentTargetedEnemy = nullptr;

        if (m_logicCard->requiresTarget()) {
            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) bScene->startTargeting(this->scenePos());
        } else {
            setRotation(0.0);
        }
        event->accept();
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isSuspended) {
        event->accept();
        return;
    }

    if (m_isDisplayOnly) {
        event->accept();
        return;
    }

    if (m_isGhost) {
        event->ignore();
        return;
    }

    if (m_isDragging) {
        if (m_logicCard->requiresTarget()) {
            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) bScene->updateTargeting(event->scenePos());

            QList<QGraphicsItem*> hitItems = scene()->items(event->scenePos());
            EnemyItem* foundEnemy = nullptr;

            for (QGraphicsItem* item : hitItems) {
                EnemyItem* e = dynamic_cast<EnemyItem*>(item);
                if (e && !e->getLogicEnemy()->isDead()) {
                    foundEnemy = e;
                    break;
                }
            }

            if (foundEnemy != m_currentTargetedEnemy) {
                if (m_currentTargetedEnemy) {
                    m_currentTargetedEnemy->m_isTargeted = false;
                    m_currentTargetedEnemy->update();
                }

                m_currentTargetedEnemy = foundEnemy;
                if (m_currentTargetedEnemy) {
                    m_currentTargetedEnemy->m_isTargeted = true;
                    m_currentTargetedEnemy->update();
                    qDebug() << "[UI Vibe] >> TARGET LOCKED ON:" << m_currentTargetedEnemy->getLogicEnemy()->getName();
                }
                update();
            }
        } else {
            setPos(event->scenePos());

            BattleScene* bScene = dynamic_cast<BattleScene*>(scene());
            if (bScene) {
                EnemyItem* currentHitItem = nullptr;
                QList<QGraphicsItem*> colliders = bScene->items(event->scenePos());

                for (QGraphicsItem* item : colliders) {
                    EnemyItem* eItem = dynamic_cast<EnemyItem*>(item);
                    if (eItem && !eItem->getLogicEnemy()->isDead()) {
                        currentHitItem = eItem;
                        break;
                    }
                }

                if (m_currentTargetedEnemy != currentHitItem) {
                    m_currentTargetedEnemy = currentHitItem;
                    update();
                }
            }
        }
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (m_isDisplayOnly) {
        event->accept();
        return;
    }

    if (m_isGhost) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        BattleScene* bScene = dynamic_cast<BattleScene*>(scene());

        if (m_logicCard->requiresTarget() && bScene) {
            bScene->updateTargeting(event->scenePos());

            QList<QGraphicsItem*> colliders = bScene->items(event->scenePos());
            for (QGraphicsItem* item : colliders) {
                EnemyItem* eItem = dynamic_cast<EnemyItem*>(item);
                if (eItem && !eItem->getLogicEnemy()->isDead()) {
                    m_currentTargetedEnemy = eItem;
                    break;
                }
            }
        }

        if (m_currentTargetedEnemy) {
            m_currentTargetedEnemy->m_isTargeted = false;
            m_currentTargetedEnemy->update();
        }

        if (m_logicCard->requiresTarget()) {
            if (bScene) bScene->stopTargeting();

            if (m_currentTargetedEnemy && !m_currentTargetedEnemy->getLogicEnemy()->isDead()) {
                qDebug() << "[UI] Polish Play Success! Targeted card executed on:" << m_currentTargetedEnemy->getLogicEnemy()->getName();
                emit cardPlayedRequest(m_logicCard, m_currentTargetedEnemy->getLogicEnemy());
            } else {
                qDebug() << "[UI] Missed target during polish test. Snapping back.";
                animateToHome();
            }
            m_currentTargetedEnemy = nullptr;
            update();
        } else {
            if (event->scenePos().y() < 820.0) {
                qDebug() << "[UI] Polish Area Play Success!";
                emit cardPlayedRequest(m_logicCard, nullptr);
            } else {
                qDebug() << "[UI] Drag too short. Polish abort.";
                animateToHome();
            }
            m_currentTargetedEnemy = nullptr;
            update();
        }
        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}

void CardItem::animatePlayAndDiscard() {
    m_isPlayed = true;
    QParallelAnimationGroup* playGroup = new QParallelAnimationGroup(this);

    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(300);
    posAnim->setEndValue(QPointF(1800, 1000));
    posAnim->setEasingCurve(QEasingCurve::InBack);
    playGroup->addAnimation(posAnim);

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setEndValue(0.1);
    playGroup->addAnimation(scaleAnim);

    QPropertyAnimation* rotateAnim = new QPropertyAnimation(this, "rotation");
    rotateAnim->setDuration(300);
    rotateAnim->setEndValue(this->rotation() + 720.0);
    playGroup->addAnimation(rotateAnim);

    connect(playGroup, &QParallelAnimationGroup::finished, this, &CardItem::deleteLater);
    qDebug() << "[UI Animation] Polish play transition started for:" << m_logicCard->getName();
    playGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardItem::animateSuspendInCenter() {
    m_isSuspended = true;
    setZValue(103);

    QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(300);
    anim->setEndValue(QPointF(960, 500));
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    setScale(m_baseScale * 1.2);
}

void CardItem::animateTrueExhaust() {
    m_isPlayed = true;
    QGraphicsScene* s = scene();
    if (!s) return;

    qreal baseZ = zValue();
    int particleCount = 40;
    QRectF cardRect = boundingRect();

    for (int i = 0; i < particleCount; ++i) {
        qreal size = randomBetween(10, 18);
        QGraphicsEllipseItem* ash = new QGraphicsEllipseItem(0, 0, size, size);

        QColor emberColor(randomBetween(40, 65), randomBetween(30, 40), randomBetween(30, 40));
        ash->setBrush(QBrush(emberColor));
        ash->setPen(Qt::NoPen);
        ash->setZValue(baseZ + 1);
        s->addItem(ash);

        qreal startX = scenePos().x() + randomBetween(cardRect.left(), cardRect.right());
        qreal startY = scenePos().y() + randomBetween(cardRect.top(), cardRect.bottom());
        ash->setPos(startX, startY);

        qreal targetX = startX + randomBetween(-350, 350);
        qreal targetY = startY - randomBetween(400, 700);
        qreal startRot = this->rotation();
        qreal targetRot = startRot + randomBetween(-720, 720);

        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(randomBetween(1500, 2500));
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutQuad);

        connect(anim, &QVariantAnimation::valueChanged, [ash, startX, startY, targetX, targetY, startRot, targetRot](const QVariant& value) {
            qreal progress = value.toReal();
            ash->setPos(startX + (targetX - startX) * progress, startY + (targetY - startY) * progress);
            ash->setRotation(startRot + (targetRot - startRot) * progress);

            qreal opacity = 1.0;
            if (progress > 0.3) {
                opacity = qMax(0.0, 1.0 - ((progress - 0.3) / 0.7));
            }
            if (progress >= 1.0) {
                opacity = 0.0;
            }
            ash->setOpacity(opacity);
        });

        connect(anim, &QVariantAnimation::finished, [ash]() {
            delete ash;
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    QParallelAnimationGroup* masterGroup = new QParallelAnimationGroup(this);

    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(1500);
    posAnim->setEndValue(this->pos() + QPointF(0, -120));
    posAnim->setEasingCurve(QEasingCurve::InQuad);
    masterGroup->addAnimation(posAnim);

    if (this->graphicsEffect()) {
        QGraphicsOpacityEffect* opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(this->graphicsEffect());
        if (opacityEffect) {
            QPropertyAnimation* fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
            fadeAnim->setDuration(1500);
            fadeAnim->setStartValue(1.0);
            fadeAnim->setEndValue(0.0);
            fadeAnim->setEasingCurve(QEasingCurve::Linear);
            masterGroup->addAnimation(fadeAnim);
        }
    }

    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(1500);
    scaleAnim->setEndValue(0.01);
    masterGroup->addAnimation(scaleAnim);

    connect(masterGroup, &QParallelAnimationGroup::finished, this, &CardItem::deleteLater);
    qDebug() << "[UI Animation] 视网膜级冥灰崩解特效启动喵！！！ ->" << m_logicCard->getName();
    masterGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal CardItem::randomBetween(qreal low, qreal high) {
    return low + (static_cast<qreal>(std::rand()) / static_cast<qreal>(RAND_MAX)) * (high - low);
}