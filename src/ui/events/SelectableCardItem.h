#pragma once
#include <QGraphicsObject>
#include "../../entities/cards/card.h"

class SelectableCardItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit SelectableCardItem(Card* card, QGraphicsItem* parent = nullptr);

    Card* card() const { return m_card; }
    void setHighlighted(bool h);
    bool isHighlighted() const { return m_highlighted; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void clicked(Card* card);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Card* m_card;
    bool m_hovered = false;
    bool m_highlighted = false;
};
