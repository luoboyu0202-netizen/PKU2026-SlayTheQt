#pragma once
#include <QGraphicsObject>
#include <QPixmap>

class IconButton : public QGraphicsObject {
    Q_OBJECT
public:
    explicit IconButton(const QString& imagePath, QGraphicsItem* parent = nullptr);

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
    QPixmap m_pixmap;
    bool m_hovered = false;
    bool m_pressed = false;
};
