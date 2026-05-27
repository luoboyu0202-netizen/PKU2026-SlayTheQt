#include "TextButton.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

TextButton::TextButton(const QString& text, int width, int height, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_text(text), m_width(width), m_height(height) {
    m_font = QFont("Microsoft YaHei", 18, QFont::Bold);
    setAcceptHoverEvents(true);
}

QRectF TextButton::boundingRect() const {
    return QRectF(-m_width / 2, -m_height / 2, m_width, m_height);
}

void TextButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    const QRectF r = boundingRect();
    const qreal pressOffset = m_isPressed ? 3.0 : 0.0;
    painter->translate(0, pressOffset);

    QPainterPath plate;
    const qreal notch = 18.0;
    plate.moveTo(r.left() + notch, r.top());
    plate.lineTo(r.right() - notch, r.top());
    plate.lineTo(r.right(), r.center().y());
    plate.lineTo(r.right() - notch, r.bottom());
    plate.lineTo(r.left() + notch, r.bottom());
    plate.lineTo(r.left(), r.center().y());
    plate.closeSubpath();

    QColor top = m_isHovered ? QColor(75, 56, 34, 245) : QColor(42, 34, 28, 240);
    QColor bottom = m_isPressed ? QColor(92, 58, 28, 250) : QColor(25, 22, 20, 245);
    QLinearGradient fill(r.topLeft(), r.bottomLeft());
    fill.setColorAt(0.0, top);
    fill.setColorAt(0.55, QColor(31, 27, 24, 245));
    fill.setColorAt(1.0, bottom);

    painter->setBrush(fill);
    painter->setPen(QPen(QColor(45, 24, 12, 220), 5));
    painter->drawPath(plate);

    QColor rim = m_isHovered ? QColor(255, 208, 95) : QColor(184, 126, 46);
    painter->setPen(QPen(rim, m_isHovered ? 3 : 2));
    painter->drawPath(plate);

    QRectF inner = r.adjusted(10, 8, -10, -8);
    QPainterPath innerPlate;
    innerPlate.moveTo(inner.left() + notch * 0.65, inner.top());
    innerPlate.lineTo(inner.right() - notch * 0.65, inner.top());
    innerPlate.lineTo(inner.right(), inner.center().y());
    innerPlate.lineTo(inner.right() - notch * 0.65, inner.bottom());
    innerPlate.lineTo(inner.left() + notch * 0.65, inner.bottom());
    innerPlate.lineTo(inner.left(), inner.center().y());
    innerPlate.closeSubpath();
    painter->setPen(QPen(QColor(255, 232, 160, m_isHovered ? 100 : 55), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(innerPlate);

    painter->setPen(QPen(QColor(25, 12, 5, 180), 2));
    painter->setFont(m_font);
    painter->drawText(r.translated(2, 2), Qt::AlignCenter, m_text);

    painter->setPen(m_isHovered ? QColor(255, 244, 198) : QColor(232, 216, 178));
    painter->drawText(r, Qt::AlignCenter, m_text);
}

void TextButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event); m_isHovered = true; update();
}
void TextButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event); m_isHovered = false; update();
}
void TextButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true; update(); event->accept();
    }
}
void TextButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false; update();
        emit clicked(); event->accept();
    }
}
