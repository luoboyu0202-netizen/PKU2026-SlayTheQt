#include "PileItem.h"
#include <QPainter>
#include <QFont>

PileItem::PileItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_name(name), m_count(0) {
    setAcceptHoverEvents(true);
}

void PileItem::updateCount(int count) {
    if (m_count != count) {
        m_count = count;
        update(); // 数字变化时触发重绘
    }
}

QRectF PileItem::boundingRect() const {
    return QRectF(-40, -40, 80, 80);
}

void PileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // 绘制几层卡牌叠在一起的“牌堆感”
    painter->setBrush(QColor(40, 40, 45));
    painter->setPen(QPen(QColor(150, 150, 150), 2));
    painter->drawRect(-35, -35, 60, 80);
    painter->drawRect(-30, -30, 60, 80);
    painter->drawRect(-25, -25, 60, 80);

    // 绘制牌堆名称
    painter->setPen(Qt::white);
    QFont font("Microsoft YaHei", 10, QFont::Bold);
    painter->setFont(font);
    painter->drawText(QRectF(-25, -20, 60, 20), Qt::AlignCenter, m_name);

    // 绘制醒目的数字
    QFont countFont("Arial", 20, QFont::Bold);
    painter->setFont(countFont);
    painter->setPen(QColor(241, 196, 15)); // 金黄色数字
    painter->drawText(QRectF(-25, 10, 60, 40), Qt::AlignCenter, QString::number(m_count));
}

void PileItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(); // 发射被点击信号！
        event->accept();
    }
}