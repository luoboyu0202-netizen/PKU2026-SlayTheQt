#include "LeaveButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include <QImage>
#include <QBitmap>

LeaveButton::LeaveButton(QGraphicsItem* parent) : QGraphicsObject(parent) {
    m_font = QFont("Microsoft YaHei", 14, QFont::Bold);
    setAcceptHoverEvents(true);
}

void LeaveButton::setIcon(const QString& imagePath) {
    QImage img(imagePath);
    if (!img.isNull()) {
        // 尝试“抠图”：将接近黑色的背景设为透明
        img = img.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb pixel = img.pixel(x, y);
                // 如果 R,G,B 都小于 40，认为是背景，设为全透明
                if (qRed(pixel) < 40 && qGreen(pixel) < 40 && qBlue(pixel) < 40) {
                    img.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }
        m_icon = QPixmap::fromImage(img).scaled(280, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    update();
}

QRectF LeaveButton::boundingRect() const {
    if (!m_icon.isNull()) {
        return QRectF(-m_icon.width()/2, -m_icon.height()/2, m_icon.width(), m_icon.height());
    }
    return QRectF(-60, -20, 120, 40);
}

void LeaveButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!m_icon.isNull()) {
        qreal op = m_isPressed ? 0.7 : m_isHovered ? 1.1 : 1.0;
        painter->setOpacity(op);
        if (m_isPressed) painter->translate(0, 3);
        painter->drawPixmap(boundingRect().toRect(), m_icon);
    } else {
        QColor bgColor = m_isPressed ? QColor(60, 60, 60)
                       : m_isHovered ? QColor(90, 90, 90)
                       : QColor(50, 50, 50, 200);

        painter->setBrush(bgColor);
        painter->setPen(QPen(QColor(200, 200, 200), 1.5));
        painter->drawRoundedRect(boundingRect(), 6, 6);

        painter->setPen(Qt::white);
        painter->setFont(m_font);
        painter->drawText(boundingRect(), Qt::AlignCenter, "离开");
    }
}

void LeaveButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_isHovered = true;
    update();
}

void LeaveButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_isHovered = false;
    update();
}

void LeaveButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        update();
        event->accept();
    }
}

void LeaveButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false;
        update();
        emit clicked();
        event->accept();
    }
}
