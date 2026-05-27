#pragma once
#include <QGraphicsObject>
#include <QFont>

class LeaveButton : public QGraphicsObject {
    Q_OBJECT
public:
    explicit LeaveButton(QGraphicsItem* parent = nullptr);

    void setIcon(const QString& imagePath);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

signals:
    void clicked();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    bool m_isHovered = false;
    bool m_isPressed = false;
    QFont m_font;
    QPixmap m_icon;
};
