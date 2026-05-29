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

    // 🔴 全新接口：直接从全局档案读取并生成遗物栏！
    void syncWithGlobalSave();
    void clearAll(); // 清理案发现场

public slots:
    void onNewRelicAdded(Relic* relic);

private:
    RelicManager* m_manager;
    QList<RelicItem*> m_items;
    const qreal m_spacing = 8.0; // 遗物间隔
    QList<Relic*> m_logicalRelics;
};