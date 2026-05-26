#include "ShuffleEffectItem.h"
#include <QPainter>
#include <QLinearGradient>
#include <cmath>

ShuffleEffectItem::ShuffleEffectItem(QPointF startPos, QPointF endPos, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_progress(0.0), m_startPos(startPos), m_endPos(endPos) {

    // 随机生成抛物线顶点的高度 (100 到 250 之间)，让每个方块飞的轨迹不一样！
    m_peakHeight = 100.0 + (rand() % 150);
    setPos(startPos);
}

QRectF ShuffleEffectItem::boundingRect() const {
    return QRectF(-8, -12, 16, 24); // 迷你金色卡牌/方块大小
}

void ShuffleEffectItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    // 绝美的金色渐变
    QLinearGradient grad(-8, -12, 8, 12);
    grad.setColorAt(0, QColor(255, 215, 0));   // 亮金色
    grad.setColorAt(1, QColor(184, 134, 11));  // 暗金色

    painter->setBrush(grad);
    painter->setPen(QPen(Qt::white, 1));
    painter->drawRoundedRect(boundingRect(), 2, 2);
}

// 🔴 动画引擎每秒会调用这里 60 次，传入从 0.0 到 1.0 的进度！
void ShuffleEffectItem::setProgress(qreal p) {
    m_progress = p;

    // 1. X 轴匀速前进
    qreal currentX = m_startPos.x() + (m_endPos.x() - m_startPos.x()) * p;

    // 2. Y 轴起飞：利用 sin(p * 3.14) 产生一个先变大后变小的值，完美模拟抛物线！
    qreal currentY = m_startPos.y() + (m_endPos.y() - m_startPos.y()) * p - m_peakHeight * std::sin(p * 3.1415926);

    setPos(currentX, currentY);

    // 3. 一边飞一边狂转两圈半 (900度)！
    setRotation(p * 900);
}
