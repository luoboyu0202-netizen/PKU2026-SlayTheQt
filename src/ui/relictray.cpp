// RelicTray.cpp
#include "RelicTray.h"
#include <QGraphicsScene>
#include <QDebug>
#include "relics/RelicManager.h"

RelicTray::RelicTray(QGraphicsItem* parent) : QGraphicsObject(parent), m_manager(nullptr) {}

void RelicTray::bindManager(RelicManager* manager) {
    m_manager = manager;
    connect(manager, &RelicManager::relicAdded, this, &RelicTray::onNewRelicAdded);
    for (Relic* r : manager->getRelics()) {
        onNewRelicAdded(r);
    }
}

// 🔴 核心新方法：直接设置遗物列表
void RelicTray::setRelics(const QList<Relic*>& relics) {
    // 1. 清除现有 UI 项（但不删除逻辑遗物，它们由 GameWindow 管理）
    clearAll();

    // 2. 挂载新的遗物
    for (Relic* relic : relics) {
        onNewRelicAdded(relic);
    }

    qDebug() << "[RelicTray] setRelics with" << relics.size() << "relics, first:" << (relics.isEmpty() ? "null" : relics.first()->getName());

}

void RelicTray::onNewRelicAdded(Relic* relic) {
    RelicItem* item = new RelicItem(relic, this);

    qreal xPos = m_items.size() * (48 + m_spacing);
    item->setPos(xPos, 0);

    m_items.append(item);
    update();
}

QRectF RelicTray::boundingRect() const {
    return QRectF(0, 0, 800, 60);
}

void RelicTray::clearAll() {
    // 只移除和删除 UI 图元，不触碰逻辑遗物的 delete
    for (RelicItem* item : m_items) {
        if (scene()) scene()->removeItem(item);
        delete item;
    }
    m_items.clear();
    // m_currentRelics 在 setRelics 中更新，这里不清空
}