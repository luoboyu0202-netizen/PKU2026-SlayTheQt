#pragma once
#include <QGraphicsObject>
#include <QFont>
#include <QColor>
#include <QPixmap>
#include <functional>

class EventOptionButton : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit EventOptionButton(const QString& text, std::function<void()> onClick, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setEnabled(bool enabled);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_text;
    std::function<void()> m_onClick;
    bool m_isEnabled = true;
    bool m_isHovered = false;
    QPixmap m_bgPixmap;
};
