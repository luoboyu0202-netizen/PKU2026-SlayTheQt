#pragma once
#include <QGraphicsObject>
#include <QString>
#include <QPixmap> // 🔴【新增】：用于加载图片资源
#include <QGraphicsSceneMouseEvent>

// 前置声明图片类，防止头文件循环包含
class QPixmap;

class PileItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX)
public:
    explicit PileItem(const QString& name, QGraphicsItem* parent = nullptr);

    void updateCount(int count); // 接收底层数量更新
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 在 signals 区域加上：
signals:
    void clicked();

    // 在 protected 区域加上鼠标拦截：
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    int m_count;
};
