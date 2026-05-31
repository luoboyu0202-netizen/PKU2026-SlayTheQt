#pragma once
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include "../../entities/cards/card.h"

class ShopCardItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
public:
    explicit ShopCardItem(Card* card, int price, QGraphicsItem* parent = nullptr);

    Card* card() const { return m_card; }
    int price() const { return m_price; }
    int originalPrice() const { return m_originalPrice; }
    void setAffordable(bool canAfford);
    void setOnSale(bool onSale);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QPointF topEdgeCenter() const;

signals:
    void clicked(Card* card, int price);
    void hovered(ShopCardItem* item);
    void unhovered(ShopCardItem* item);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Card* m_card;
    int m_price;
    int m_originalPrice;
    bool m_affordable = true;
    bool m_onSale = false;
    QPropertyAnimation* m_scaleAnim = nullptr;
    QPixmap m_cardImage;
};
