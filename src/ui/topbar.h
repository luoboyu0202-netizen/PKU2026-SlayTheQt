#pragma once
#include <QGraphicsObject>
#include <QFont>
#include "../entities/Player.h" // 引入玩家底层数据类
#include "PileItem.h" // 🔴 引入牌堆实体

class TopBar : public QGraphicsObject {
    Q_OBJECT
public:
    explicit TopBar(QGraphicsItem* parent = nullptr);
    virtual ~TopBar() = default;

    // 设置顶栏动态宽度（用于全屏/窗口缩放）
    void setBarWidth(qreal width);
    qreal barWidth() const { return m_barWidth; }
    // 返回牌堆图标中心点（场景坐标），供飞行动画定位
    QPointF deckPileCenterScenePos() const;

    // 核心契约：绑定底层数据实体
    void bindPlayer(Player* player);
    // ========================================================
    // 🔴【新增】：暴露给外部的刷新接口
    // 当卡牌飞行动画结束入账时，调用它让数字跳动！
    // ========================================================
    void refreshDeckCount();

    // QGraphicsItem 必须实现的两个虚函数
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updatePlayerName(const QString& name) {
        m_playerName = name;
        update(); // 别忘了触发重绘！
    }


public slots:
    // 接收底层信号的槽函数
    void updateHp(int current, int max);
    void updateEnergy(int current, int max);
    void updateGold(int current);
    void updateBlock(int block);
    // 🔴【新增】：监听牌堆图标的点击
    void onDeckPileClicked();

signals:
    void deckViewRequested(); // 🔴 向上级请求展开牌库结界！
    void returnToTitleRequested(); // 🔴 新增：请求返回主菜单信号

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

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
    // 🔴 全局总牌组的物理实体
    PileItem* m_masterDeckPile;

    QRectF m_exitBtnRect; // 🔴 退出按钮的碰撞箱
    qreal m_barWidth = 1600; // 顶栏动态宽度
    qreal m_goldX = 1200;    // 金币图标X（随宽度等比例缩放）
};
