#include "ShopCardItem.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

ShopCardItem::ShopCardItem(Card* card, int price, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_card(card), m_price(price), m_originalPrice(price) {
    setAcceptHoverEvents(true);

    QString imgPath = m_card->getImagePath();
    if (!imgPath.isEmpty()) {
        QPixmap raw(imgPath);
        if (!raw.isNull()) {
            m_cardImage = raw.scaled(140, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
}

void ShopCardItem::setAffordable(bool canAfford) {
    m_affordable = canAfford;
    update();
}

void ShopCardItem::setOnSale(bool onSale) {
    m_onSale = onSale;
    if (onSale) m_price = m_originalPrice / 2;
    update();
}

QPointF ShopCardItem::topEdgeCenter() const {
    return mapToScene(QPointF(0, boundingRect().top()));
}

QRectF ShopCardItem::boundingRect() const {
    // H = W * 1.4 (Gemini recommendation)
    return QRectF(-92, -130, 185, 260);
}

void ShopCardItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    QColor bg;
    switch (m_card->getType()) {
    case CardType::Attack: bg = QColor(140, 30, 30); break;
    case CardType::Skill:  bg = QColor(30, 100, 30); break;
    case CardType::Power:  bg = QColor(30, 30, 140); break;
    default: bg = QColor(60, 60, 60); break;
    }
    painter->setBrush(bg);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1), 10, 10);

    if (!m_cardImage.isNull()) {
        painter->drawPixmap(QRect(-78, -118, 156, 105), m_cardImage);
    }

    QFont nameFont("Microsoft YaHei", 11, QFont::Bold);
    painter->setFont(nameFont);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(-78, -15, 156, 24), Qt::AlignCenter, m_card->getName());

    QFont costFont("Microsoft YaHei", 12);
    painter->setFont(costFont);
    painter->setPen(QColor(255, 220, 100));
    painter->drawText(QRectF(-78, 12, 156, 18), Qt::AlignCenter,
                      QString::number(m_card->getCost()) + " 费");

    QFont descFont("Microsoft YaHei", 8);
    painter->setFont(descFont);
    painter->setPen(QColor(180, 180, 180));
    QString desc = m_card->getDescription();
    if (desc.length() > 36) desc = desc.left(34) + "...";
    painter->drawText(QRectF(-78, 32, 156, 45), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, desc);
}

void ShopCardItem::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    if (m_scaleAnim) { m_scaleAnim->stop(); delete m_scaleAnim; m_scaleAnim = nullptr; }
    m_scaleAnim = new QPropertyAnimation(this, "scale", this);
    m_scaleAnim->setDuration(150);
    m_scaleAnim->setStartValue(scale());
    m_scaleAnim->setEndValue(1.15);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_scaleAnim->start(); // KeepWhenStopped (default) — we manage lifecycle manually
    emit hovered(this);
}

void ShopCardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    if (m_scaleAnim) { m_scaleAnim->stop(); delete m_scaleAnim; m_scaleAnim = nullptr; }
    m_scaleAnim = new QPropertyAnimation(this, "scale", this);
    m_scaleAnim->setDuration(150);
    m_scaleAnim->setStartValue(scale());
    m_scaleAnim->setEndValue(1.0);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_scaleAnim->start();
    emit unhovered(this);
}

void ShopCardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_affordable) {
        event->accept();
        emit clicked(m_card, m_price);
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}
