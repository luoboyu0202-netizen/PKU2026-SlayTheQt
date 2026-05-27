#pragma once
#include <QGraphicsObject>
#include "entities/relics/Relic.h"

class RelicItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit RelicItem(Relic* logicRelic, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Relic* m_logicRelic;
    int m_displayCounter;
};