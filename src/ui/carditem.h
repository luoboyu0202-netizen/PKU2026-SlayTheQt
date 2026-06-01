#pragma once
#include <QGraphicsObject>
#include "cards/Card.h"
#include "../entities/Enemy.h" // 引入逻辑层实体

class EnemyItem;

class CardItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit CardItem(Card* logicCard, QGraphicsItem* parent = nullptr);
    virtual ~CardItem() = default;

    Card* getLogicCard() const { return m_logicCard; }

    // ========================================================
    // ⚔️ 战斗系统动画与状态
    // ========================================================
    void animateSuspendInCenter();
    bool isSuspended() const { return m_isSuspended; }
    void setSuspended(bool val) { m_isSuspended = val; }

    void setHomeState(const QPointF& pos, qreal rotation);
    void animateToHome();
    void animatePlayAndDiscard();
    void animateTrueExhaust(); // 真·灵魂燃烧

    // 响应费用变化，更新自身外观
    void checkPlayability(int currentEnergy);

    // ========================================================
    // 🎨 UI 绘制与基础交互
    // ========================================================
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setInteractive(bool interact) {
        if (!interact) {
            setAcceptedMouseButtons(Qt::NoButton);
            setFlag(QGraphicsItem::ItemIsMovable, false);
        } else {
            setAcceptedMouseButtons(Qt::LeftButton);
            setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }

    // 物理断路器与纯展示模式
    void setDisplayOnly(bool val) { m_isDisplayOnly = val; }
    void setGhostMode(bool isGhost) { m_isGhost = isGhost; }

    // ========================================================
    // ⛺ 营火与商店复用系统 (队友新增)
    // ========================================================
    void setSelectionEnabled(bool enabled) { m_isSelectionEnabled = enabled; }
    void setHighlighted(bool h) { m_isHighlighted = h; update(); }
    bool isHighlighted() const { return m_isHighlighted; }

    void setPrice(int price) { m_price = price; }
    int price() const { return m_price; }
    void setOnSale(bool onSale) { m_isOnSale = onSale; update(); }
    void setAffordable(bool canAfford) { m_isAffordable = canAfford; update(); }


signals:
    void cardPlayedRequest(Card* card, Enemy* target);
    void cardVisualDestroyed(CardItem* item);
    // 选择模式下点击卡牌
    void cardClicked(CardItem* item);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    // 基础数据
    Card* m_logicCard;
    QPointF m_homePos;
    qreal m_homeRotation;

    // 战斗状态开关
    bool m_isHovered = false;
    bool m_isDragging = false;
    bool m_isPlayed = false;
    bool m_isPlayable = false;
    bool m_isSuspended = false;
    bool m_isDisplayOnly = false;
    bool m_isGhost = false;
    qreal m_defaultZ = 10.0; // 默认层级

    // 营火/商店状态开关
    bool m_isSelectionEnabled = false;
    bool m_isHighlighted = false;
    int m_price = 0;
    bool m_isOnSale = false;
    bool m_isAffordable = true;

    // 目标与视觉
    EnemyItem* m_currentTargetedEnemy = nullptr;
    QPixmap m_cardPixmap;

    // 随机数生成器
    qreal randomBetween(qreal low, qreal high);
};