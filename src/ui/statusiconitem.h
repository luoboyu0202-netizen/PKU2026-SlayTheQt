#pragma once
#include <QGraphicsObject>
#include <QPixmap>
#include <QString>
#include "../entities/StatusManager.h"

class StatusIconItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit StatusIconItem(StatusType type, int amount, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 更新层数时调用它，它会自动重绘
    void setAmount(int amount);

protected:
    // 鼠标悬停事件，用来显示 Tooltip
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    StatusType m_type;
    int m_amount;
    QPixmap m_icon;
    QString m_tooltipText;
    bool m_isDebuff; // 决定画红框还是蓝框
};
