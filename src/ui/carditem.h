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

    // 🔴 新增：让卡牌飞到屏幕中央悬浮！
    void animateSuspendInCenter();

    // 🔴 新增状态：我是不是正在悬浮？
    bool isSuspended() const { return m_isSuspended; }
    void setSuspended(bool val) { m_isSuspended = val; }

    void setHomeState(const QPointF& pos, qreal rotation);
    void animateToHome();
    // 🟢 正名：飞向弃牌堆（右下角）
    void animatePlayAndDiscard();

    // 🔴 新增：真·灵魂燃烧（原地化为灰烬向上飘散）
    void animateTrueExhaust();

    // 【新核心】：响应费用变化，更新自身外观
    void checkPlayability(int currentEnergy);

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

    // --- 选择模式（事件中卡牌点选，如营火升级、商店删牌） ---
    void setSelectionEnabled(bool enabled) { m_isSelectionEnabled = enabled; }
    void setHighlighted(bool h) { m_isHighlighted = h; update(); }
    bool isHighlighted() const { return m_isHighlighted; }
    // --- 商品模式（商店购买） ---
    void setPrice(int price) { m_price = price; }
    int price() const { return m_price; }
    void setOnSale(bool onSale) { m_isOnSale = onSale; update(); }
    void setAffordable(bool canAfford) { m_isAffordable = canAfford; update(); }

public:
    void setDisplayOnly(bool val) { m_isDisplayOnly = val; }
    void setGhostMode(bool isGhost) { m_isGhost = isGhost; }

private:
    bool m_isDisplayOnly = false; // 默认是实战模式
    bool m_isGhost = false; // 🔴 物理断路器开关

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
    Card* m_logicCard;
    QPointF m_homePos;
    qreal m_homeRotation;

    bool m_isHovered;
    bool m_isDragging;
    bool m_isPlayed;
    bool m_isPlayable; // 【新增】当前费用是否足够打出这张牌
    bool m_isSuspended = false; // 默认没有悬浮
    bool m_isSelectionEnabled = false;
    bool m_isHighlighted = false;
    int m_price = 0;
    bool m_isOnSale = false;
    bool m_isAffordable = true;

    EnemyItem* m_currentTargetedEnemy;

    // 🔴【新增】：用来装卡牌立绘的相框
    QPixmap m_cardPixmap;

    // 🔴 补办随机数生成器的户口喵！
    qreal randomBetween(qreal low, qreal high);

    private:
    qreal m_defaultZ = 10.0; // 默认层级
    };