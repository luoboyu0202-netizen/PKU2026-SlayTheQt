#pragma once
#include <QGraphicsObject>
#include <QFont>
#include "../entities/Player.h" // 引入玩家底层数据类

class TopBar : public QGraphicsObject {
    Q_OBJECT
public:
    explicit TopBar(QGraphicsItem* parent = nullptr);
    virtual ~TopBar() = default;

    // 核心契约：绑定底层数据实体
    void bindPlayer(Player* player);

    // QGraphicsItem 必须实现的两个虚函数
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    // 接收底层信号的槽函数
    void updateHp(int current, int max);
    void updateEnergy(int current, int max);
    void updateGold(int current);
    void updateBlock(int block);

private:
    // UI 层的显示缓存
    QString m_playerName;
    int m_hp;
    int m_maxHp;
    int m_energy;
    int m_maxEnergy;
    int m_gold;
    int m_block;

    QFont m_uiFont; // 统一的 UI 字体
};
