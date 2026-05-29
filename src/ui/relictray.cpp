#include "RelicTray.h"
#include "../logic/GlobalSaveData.h"
#include "../logic/RelicFactory.h"
#include <QGraphicsScene> // 👈 就在文件的最顶端加上这一句！

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

void RelicTray::clearAll() {
    // 1. 烧毁视觉图元
    for (RelicItem* item : m_items) {
        if (scene()) scene()->removeItem(item);
        delete item;
    }
    m_items.clear();

    // 2. 烧毁逻辑肉身，防止内存泄漏喵！
    for (Relic* r : m_logicalRelics) {
        delete r;
    }
    m_logicalRelics.clear();
}

void RelicTray::syncWithGlobalSave() {
    clearAll(); // 先把旧的扫干净

    GlobalSaveData* save = GlobalSaveData::getInstance();

    // 遍历玩家身上的所有遗物 ID
    for (const QString& relicId : save->relicIds) {
        // 🔴 借用工厂，临时捏一个“逻辑遗物”出来！
        Relic* relic = RelicFactory::createRelic(relicId, this);
        if (relic) {
            // 恢复它的前世记忆（比如钢笔尖的层数）
            if (save->relicCounters.contains(relicId)) {
                relic->setCounter(save->relicCounters[relicId]);
            }

            m_logicalRelics.append(relic);

            // 🟢 完美复用你之前写好的、极其优雅的 UI 挂载方法！
            onNewRelicAdded(relic);
        }
    }
}