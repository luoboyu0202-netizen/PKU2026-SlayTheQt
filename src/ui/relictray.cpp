#include "RelicTray.h"

RelicTray::RelicTray(QGraphicsItem* parent) : QGraphicsObject(parent), m_manager(nullptr) {}

void RelicTray::bindManager(RelicManager* manager) {
    m_manager = manager;
    connect(manager, &RelicManager::relicAdded, this, &RelicTray::onNewRelicAdded);

    // 初始化已有遗物
    for (Relic* r : manager->getRelics()) {
        onNewRelicAdded(r);
    }
}

void RelicTray::onNewRelicAdded(Relic* relic) {
    RelicItem* item = new RelicItem(relic, this);

    // 计算位置：横向排列
    qreal xPos = m_items.size() * (48 + m_spacing);
    item->setPos(xPos, 0);

    m_items.append(item);
    update();
}

QRectF RelicTray::boundingRect() const {
    return QRectF(0, 0, 800, 60); // 预留足够的横向宽度
}