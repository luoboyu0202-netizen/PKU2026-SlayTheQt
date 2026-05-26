#pragma once
#include <QGraphicsObject>
#include <QPainter>
#include <QPolygonF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

class ConfirmButton : public QGraphicsObject {
    Q_OBJECT
public:
    explicit ConfirmButton(QGraphicsItem* parent = nullptr) : QGraphicsObject(parent) {
        // 🔴 极其重要：开启悬停雷达，让它知道鼠标来了！
        setAcceptHoverEvents(true);

        // 🔴 绘制完美的瘦长六边形！宽 180，高 50
        m_hexagon << QPointF(20, 0) << QPointF(160, 0) << QPointF(180, 25)
                  << QPointF(160, 50) << QPointF(20, 50) << QPointF(0, 25);
    }

    QRectF boundingRect() const override { return m_hexagon.boundingRect(); }

    // 暴露给外界的开关：控制它是灰色还是亮红色喵
    void setValid(bool valid) {
        if (m_isValid != valid) {
            m_isValid = valid;
            update(); // 状态改变，强制重绘！
        }
    }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);
        painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿开启，边缘丝滑

        // 1. 填充颜色：够牌了是淡红，不够牌是深灰
        QColor bgColor = m_isValid ? QColor(220, 80, 80) : QColor(60, 60, 60);
        painter->setBrush(bgColor);

        // 2. 边框颜色：如果合法且鼠标悬停，爆发出璀璨金边！否则是普通灰边
        QColor penColor = (m_isHovered && m_isValid) ? QColor(241, 196, 15) : QColor(120, 120, 120);
        painter->setPen(QPen(penColor, 3));

        // 3. 画出六边形！
        painter->drawPolygon(m_hexagon);

        // 4. 写上大字
        painter->setPen(m_isValid ? Qt::white : Qt::darkGray);
        painter->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
        painter->drawText(boundingRect(), Qt::AlignCenter, QStringLiteral("确 定"));
    }

    // ========================================================
    // 🟢 智能感应系统：鼠标进来和出去的瞬间，重绘自己！
    // ========================================================
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override {
        m_isHovered = true;
        update();
        QGraphicsObject::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override {
        m_isHovered = false;
        update();
        QGraphicsObject::hoverLeaveEvent(event);
    }

    // 🟢 智能起爆系统：自己处理点击，不用再麻烦 BattleView 了！
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (m_isValid && event->button() == Qt::LeftButton) {
            emit clicked(); // 发射引爆信号！
        }
        event->accept();
    }

signals:
    void clicked();

private:
    bool m_isValid = false;
    bool m_isHovered = false;
    QPolygonF m_hexagon;
};
