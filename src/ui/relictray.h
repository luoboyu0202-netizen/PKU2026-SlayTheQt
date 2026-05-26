#pragma once
#include <QGraphicsObject>
#include "entities/relics/RelicManager.h"
#include "RelicItem.h"

class RelicTray : public QGraphicsObject {
    Q_OBJECT
public:
    explicit RelicTray(QGraphicsItem* parent = nullptr);

    void bindManager(RelicManager* manager);

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {} // 托盘本身不画东西

private slots:
    void onNewRelicAdded(Relic* relic);

private:
    RelicManager* m_manager;
    QList<RelicItem*> m_items;
    const qreal m_spacing = 8.0; // 遗物间隔
};