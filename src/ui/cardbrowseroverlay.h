#pragma once
#include <QGraphicsObject>
#include <QList>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "cards/Card.h"
#include "CardItem.h"
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSimpleTextItem>

class CardBrowserOverlay : public QGraphicsObject {
    Q_OBJECT
public:
    // 传入要展示的卡牌列表，以及大标题
    explicit CardBrowserOverlay(const QList<Card*>& cards, const QString& title, QGraphicsItem* parent = nullptr);
    ~CardBrowserOverlay();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void closed(); // 通知外界关闭界面

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    // 🖱️ 监听滚轮滚动
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;

private:
    QList<CardItem*> m_uiCards;
    QString m_title;
    QRectF m_closeBtnRect;
    bool m_isCloseBtnHovered = false;

    QPolygonF m_closeBtnPolygon; // 存储瘦长六边形的 6 个顶点

    // 🔄 统一刷新卡牌位置的神器
    void updateCardPositions();

    // 🔴 滚动相关的核心变量
    qreal m_currentScrollY = 0.0; // 当前向下滚动的偏移量
    qreal m_maxScrollY = 0.0;     // 到底能滚多深？(取决于牌的数量)

    // 🔴 独立的高层级视觉图元
    QGraphicsRectItem* m_topBanner = nullptr;       // 顶部遮罩条（隐藏滑上去的卡牌）
    QGraphicsSimpleTextItem* m_titleText = nullptr; // 独立标题
    QGraphicsPolygonItem* m_closeBtnVisual = nullptr; // 关闭按钮的图形
    QGraphicsSimpleTextItem* m_closeBtnText = nullptr;// 关闭按钮的文字
};
