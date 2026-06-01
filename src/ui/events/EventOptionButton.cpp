#include "EventOptionButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>

EventOptionButton::EventOptionButton(const QString& text, std::function<void()> onClick, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_text(text), m_onClick(onClick)
{
    setAcceptHoverEvents(true);
    m_bgPixmap.load(":/resources/images/events/Random/Univ/button.png");
}

QRectF EventOptionButton::boundingRect() const {
    return QRectF(0, 0, 800, 70);
}

void EventOptionButton::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // Draw background pixmap
    if (!m_bgPixmap.isNull()) {
        painter->drawPixmap(boundingRect().toRect(), m_bgPixmap);
        
        // --- Refined Hover Effect ---
        // Instead of a rectangular overlay, we draw a subtle semi-transparent 
        // white layer ONLY over the non-transparent parts of the button
        // Or simpler: just a subtle white glow if hovered
        if (m_isHovered && m_isEnabled) {
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setBrush(QColor(255, 255, 255, 30)); // Very faint white
            painter->setPen(Qt::NoPen);
            
            // To avoid the rectangular block, we can use a slightly smaller rounded rect
            // that fits inside the button's shape, or just skip the fill and use text color change.
            // Let's use a soft white overlay but clipped/inset to look better.
            painter->drawRoundedRect(boundingRect().adjusted(10, 5, -10, -5), 15, 15);
        }
    } else {
        // Fallback
        painter->setBrush(QColor(40, 45, 50, 200));
        painter->setPen(QPen(Qt::white, 2));
        painter->drawRect(boundingRect());
    }

    // Text
    painter->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    
    // Change text color on hover for better feedback
    if (m_isHovered && m_isEnabled) {
        painter->setPen(QColor(255, 255, 200)); // Slight yellow tint on hover
    } else {
        painter->setPen(m_isEnabled ? Qt::white : Qt::gray);
    }
    
    painter->drawText(boundingRect().adjusted(50, 0, -50, 0), Qt::AlignVCenter | Qt::AlignLeft, m_text);
}

void EventOptionButton::setEnabled(bool enabled) {
    m_isEnabled = enabled;
    update();
}

void EventOptionButton::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    if (!m_isEnabled) return;
    m_isHovered = true;
    
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(100);
    anim->setStartValue(scale());
    anim->setEndValue(1.02);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    update();
}

void EventOptionButton::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    m_isHovered = false;
    
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(100);
    anim->setStartValue(scale());
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    update();
}

void EventOptionButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isEnabled && m_onClick) {
        m_onClick();
    }
}
