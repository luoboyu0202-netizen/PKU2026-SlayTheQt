#include "LeaveButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QBitmap>
#include <QPropertyAnimation>

LeaveButton::LeaveButton(QGraphicsItem* parent) : QGraphicsObject(parent) {
    m_font = QFont("Microsoft YaHei", 20, QFont::Bold);
    setAcceptHoverEvents(true);
    // ========================================================
    // 🔴 设定物理锚点：因为你的 boundingRect 是 (-宽/2, -高/2)
    // 所以它的原点就是最完美的正中心 (0, 0)！
    // ========================================================
    setTransformOriginPoint(0, 0);
}

void LeaveButton::setIcon(const QString& imagePath) {
    QImage img(imagePath);
    if (!img.isNull()) {
        img = img.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb pixel = img.pixel(x, y);
                if (qRed(pixel) < 40 && qGreen(pixel) < 40 && qBlue(pixel) < 40) {
                    img.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }
        m_icon = QPixmap::fromImage(img).scaled(720, 360, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_width = m_icon.width();
        m_height = m_icon.height();
    }
    update();
}

void LeaveButton::setText(const QString& text) {
    m_text = text;
    update();
}

QRectF LeaveButton::boundingRect() const {
    if (!m_icon.isNull()) {
        return QRectF(-m_width / 2, -m_height / 2, m_width, m_height);
    }
    // Default: larger rectangle for text button
    return QRectF(-100, -50, 200, 100);
}

void LeaveButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!m_icon.isNull()) {
        qreal op = m_isPressed ? 0.7 : m_isHovered ? 1.1 : 1.0;
        painter->setOpacity(op);
        if (m_isPressed) painter->translate(0, 3);
        painter->drawPixmap(boundingRect().toRect(), m_icon);
    }

    // Draw text ON the icon, centered
    if (!m_text.isEmpty()) {
        painter->setPen(QColor(255, 255, 245));
        painter->setFont(m_font);

        // Draw text shadow for readability
        QRectF textRect = boundingRect().adjusted(0, m_height / 3, 0, -m_height / 5);
        painter->setPen(QPen(QColor(0, 0, 0, 180), 2));
        painter->drawText(textRect.translated(1, 1), Qt::AlignCenter, m_text);

        painter->setPen(QColor(255, 255, 245));
        painter->drawText(textRect, Qt::AlignCenter, m_text);
    }
}

void LeaveButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_isHovered = true;
    update();

    // ========================================================
    // 🌟 鼠标进入：极其丝滑地放大到 1.15 倍！
    // ========================================================
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(150); // 150 毫秒的极速弹起
    anim->setEndValue(1.15);
    anim->setEasingCurve(QEasingCurve::OutQuad); // 带有物理缓动的美学曲线
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LeaveButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_isHovered = false;
    update();

    // ========================================================
    // 🌟 鼠标离开：极其丝滑地回弹到 1.0 倍！
    // ========================================================
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(150);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
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
