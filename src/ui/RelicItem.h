#pragma once
#include <QGraphicsObject>
#include "entities/relics/Relic.h"
#include <QPixmap> // 🔴 新增：引入贴图类

class RelicItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit RelicItem(Relic* logicRelic, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Relic* m_logicRelic;
    int m_displayCounter;
    QPixmap m_pixmap; // 🔴 新增：专属贴图变量
};