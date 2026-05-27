#pragma once
#include <QGraphicsObject>
#include <QFont>
#include <QString>

class TextButton : public QGraphicsObject {
    Q_OBJECT
public:
    explicit TextButton(const QString& text, int width = 160, int height = 50,
                        QGraphicsItem* parent = nullptr);

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
    QString m_text;
    int m_width;
    int m_height;
    bool m_isHovered = false;
    bool m_isPressed = false;
    QFont m_font;
};
