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
            // 拒绝所有鼠标输入，把所有事件直接透传给底层（或者无视）
            setAcceptedMouseButtons(Qt::NoButton);
            setFlag(QGraphicsItem::ItemIsMovable, false);
        } else {
            // 恢复交互
            setAcceptedMouseButtons(Qt::LeftButton);
            setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }

public:
    void setDisplayOnly(bool val) { m_isDisplayOnly = val; }
    void setGhostMode(bool isGhost) { m_isGhost = isGhost; }

private:
    bool m_isDisplayOnly = false; // 默认是实战模式
    bool m_isGhost = false; // 🔴 物理断路器开关

signals:
    // 【新核心】：当玩家完美拖拽命中并松手时，向外求算账！
    void cardPlayedRequest(Card* card, Enemy* target);
    // 当卡牌视觉被销毁时，通知管家重新排版
    void cardVisualDestroyed(CardItem* item);

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

    EnemyItem* m_currentTargetedEnemy;

    // 🔴【新增】：用来装卡牌立绘的相框
    QPixmap m_cardPixmap;

    // 🔴 补办随机数生成器的户口喵！
    qreal randomBetween(qreal low, qreal high);
};