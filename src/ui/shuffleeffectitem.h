#pragma once
#include <QGraphicsObject>
#include <QPointF>

class ShuffleEffectItem : public QGraphicsObject {
    Q_OBJECT
    // 🔴 核心魔法：自定义一个 progress（进度）属性，从 0.0 走到 1.0
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit ShuffleEffectItem(QPointF startPos, QPointF endPos, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    qreal progress() const { return m_progress; }
    void setProgress(qreal p);

private:
    qreal m_progress;
    QPointF m_startPos;
    QPointF m_endPos;
    qreal m_peakHeight; // 抛物线的随机最高点
};
