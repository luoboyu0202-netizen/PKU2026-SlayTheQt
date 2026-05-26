#pragma once
#include <QGraphicsObject>
#include "../entities/Player.h"

class EnergyWidget : public QGraphicsObject {
    Q_OBJECT
public:
    explicit EnergyWidget(QGraphicsItem* parent = nullptr);
    virtual ~EnergyWidget() = default;

    // 核心契约：绑定玩家底层数据
    void bindPlayer(Player* player);

    // QGraphicsItem 核心虚函数
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

public slots:
    void onEnergyChanged(int current, int max);

private:
    int m_currentEnergy;
    int m_maxEnergy;
    bool m_isEmpty; // 🔴【新状态】：标记能量是否耗尽
};
