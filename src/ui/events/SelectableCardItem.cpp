#include "SelectableCardItem.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

SelectableCardItem::SelectableCardItem(Card* card, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_card(card) {
    setAcceptHoverEvents(true);
}

void SelectableCardItem::setHighlighted(bool h) {
    m_highlighted = h;
    update();
}

QRectF SelectableCardItem::boundingRect() const {
    return QRectF(-75, -110, 150, 220);
}

void SelectableCardItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    QColor bg;
    switch (m_card->getType()) {
    case CardType::Attack: bg = QColor(140, 30, 30); break;
    case CardType::Skill:  bg = QColor(30, 100, 30); break;
    case CardType::Power:  bg = QColor(30, 30, 140); break;
    default: bg = QColor(60, 60, 60); break;
    }

    painter->setBrush(m_highlighted ? bg.lighter(160) : bg);
    painter->setPen(QPen(m_highlighted ? QColor(255, 215, 0)
                     : m_hovered ? Qt::white : QColor(100, 100, 100), 2));
    painter->drawRoundedRect(boundingRect(), 8, 8);

    QFont nameFont("Microsoft YaHei", 13, QFont::Bold);
    painter->setFont(nameFont);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(-65, -100, 130, 30), Qt::AlignCenter, m_card->getName());

    QFont costFont("Microsoft YaHei", 20, QFont::Bold);
    painter->setFont(costFont);
    painter->setPen(QColor(255, 220, 100));
    painter->drawText(QRectF(-65, -70, 130, 30), Qt::AlignCenter, QString::number(m_card->getCost()) + " 费");

    QFont descFont("Microsoft YaHei", 10);
    painter->setFont(descFont);
    painter->setPen(QColor(200, 200, 200));
    QString desc = m_card->getDescription();
    if (desc.length() > 30) desc = desc.left(28) + "...";
    painter->drawText(QRectF(-65, -30, 130, 120), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, desc);
}

void SelectableCardItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event); m_hovered = true; update();
}
void SelectableCardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event); m_hovered = false; update();
}
void SelectableCardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) emit clicked(m_card);
}
