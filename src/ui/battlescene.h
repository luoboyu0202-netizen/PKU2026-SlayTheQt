#pragma once
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem> // 🔴 新增：用于绘制箭头三角形
#include <QPointF>
#include "EnemyItem.h"  // 🔴 必须加上这个！不然场景不认识什么是怪物喵！

class BattleScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit BattleScene(QObject* parent = nullptr);
    virtual ~BattleScene() = default;

    // 瞄准线控制接口
    void startTargeting(const QPointF& startPos);
    // 🔴 从 void 改为返回 Enemy* 指针喵！
    Enemy* updateTargeting(const QPointF& currentPos);
    void stopTargeting();
    // 🔴【核心修复】：补上这句声明！让外界（比如卡牌）能呼叫场景变色喵！
    void setTargetingEnemy(bool isTargetingEnemy);

protected:
    // 接管场景的鼠标事件以实现视窗级拖拽划线
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void updateArrowPath();
    // 🔴 新增：辅助函数，用来计算箭头三角形的角度和位置
    void updateArrowHead(const QPointF& endPoint, const QPointF& controlPoint);

    bool m_isTargeting;
    QPointF m_arrowStart;
    QPointF m_arrowEnd;

    bool m_isHoveringEnemy; // 🔴 新增：记录当前是不是正悬停在怪物身上喵！

    QGraphicsPathItem* m_arrowItem; // 承载贝塞尔曲线的 Qt 内置高效路径图元
    QGraphicsPolygonItem* m_arrowHeadItem; // 🔴 新增：专门用来画鼠标那头的三角箭头！
};
