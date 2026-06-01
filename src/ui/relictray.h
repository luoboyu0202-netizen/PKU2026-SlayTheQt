// RelicTray.h
#pragma once
#include <QGraphicsObject>
#include <QList>
#include "RelicItem.h"
#include "relics/Relic.h"

class RelicManager; // 前向声明

class RelicTray : public QGraphicsObject {
    Q_OBJECT
public:
    explicit RelicTray(QGraphicsItem* parent = nullptr);

    // 🔴 新方法：直接接收外部传入的遗物指针列表（由 GameWindow 统一管理生命周期）
    void setRelics(const QList<Relic*>& relics);

    // 保留 bindManager 以兼容旧代码（可不调用）
    void bindManager(RelicManager* manager);

    QRectF boundingRect() const override;
    void clearAll(); // 可保留，但只清理 UI 项，不 delete 遗物
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        Q_UNUSED(painter);
        Q_UNUSED(option);
        Q_UNUSED(widget);
        // RelicTray 本身不画任何东西，只负责摆放子项
    }

    // ❌ 删除 syncWithGlobalSave()，不再需要

public slots:
    void onNewRelicAdded(Relic* relic);

private:
    RelicManager* m_manager = nullptr;
    QList<RelicItem*> m_items;       // UI 遗物项
    QList<Relic*> m_currentRelics;   // 当前展示的逻辑遗物指针（仅用于避免重复添加）
    qreal m_spacing = 8;
};