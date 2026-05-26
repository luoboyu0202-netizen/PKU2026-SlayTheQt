#include "EndTurnButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include "BattleEngine.h"

EndTurnButton::EndTurnButton(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_isHovered(false), m_isPressed(false) {
    setAcceptHoverEvents(true);
}

QRectF EndTurnButton::boundingRect() const {
    return QRectF(-80, -30, 160, 60);
}

void EndTurnButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // 悬停亮蓝色，按下深蓝色，平时暗蓝色
    QColor bgColor = m_isPressed ? QColor(31, 97, 141) :
                         (m_isHovered ? QColor(52, 152, 219) : QColor(41, 128, 185));

    painter->setBrush(bgColor);
    painter->setPen(QPen(Qt::white, 2));

    // 按下时微微向下偏移，产生物理按键感
    QRectF rect = boundingRect();
    if (m_isPressed) rect.translate(0, 4);

    painter->drawRoundedRect(rect, 8, 8);

    painter->setPen(Qt::white);
    QFont font("Microsoft YaHei", 14, QFont::Bold);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, QStringLiteral("结束回合"));
}

void EndTurnButton::hoverEnterEvent(QGraphicsSceneHoverEvent* event) { m_isHovered = true; update(); }
void EndTurnButton::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) { m_isHovered = false; update(); }

void EndTurnButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // ========================================================
    // 🔴【时停结界拦截】：大脑在选牌时，无情拒绝结束回合！
    // ========================================================
    if (BattleEngine::getInstance() && BattleEngine::getInstance()->isSelectingHandCard()) {
        qDebug() << "[UI] 选牌结界中，无法结束回合喵！";
        event->accept(); // 吞掉点击事件
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        update();
        event->accept();
    }
}

void EndTurnButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false;
        update();
        emit clicked(); // 🚀 发送按钮被点击的信号！
        event->accept();
    }
}
