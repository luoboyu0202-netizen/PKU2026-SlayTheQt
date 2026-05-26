#include "BattleScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>
#include <QPen>
#include <QColor>
#include <QBrush>
#include <QDebug>
#include <QtMath> // 用于计算角度，需要包含这个头文件喵！

BattleScene::BattleScene(QObject* parent)
    : QGraphicsScene(parent),
    m_isTargeting(false),
    m_arrowItem(nullptr),
    m_arrowHeadItem(nullptr),    // 🔴 极其重要：必须初始化为空指针！
    m_isHoveringEnemy(false)     // 顺手把布尔值也初始化，养成好习惯喵！
{
    // 设置经典的 16:9 视轨大画布
    setSceneRect(0, 0, 1920, 1080);

    // 设置暗色调背景，衬托卡牌和特效
    setBackgroundBrush(QBrush(QColor(25, 25, 27)));
}

void BattleScene::startTargeting(const QPointF& startPos) {
    m_isTargeting = true;
    m_arrowStart = startPos;
    m_arrowEnd = startPos;
    m_isHoveringEnemy = false;

    if (!m_arrowItem) {
        m_arrowItem = new QGraphicsPathItem();
        addItem(m_arrowItem);
    }

    if (!m_arrowHeadItem) {
        m_arrowHeadItem = new QGraphicsPolygonItem();

        // 🔴【变大修改】：把尖端三角形拉大！从原本的 25x12 提升到 40x20！
        QPolygonF headPolygon;
        headPolygon << QPointF(0, 0) << QPointF(-40, -20) << QPointF(-40, 20);
        m_arrowHeadItem->setPolygon(headPolygon);

        addItem(m_arrowHeadItem);
    }

    m_arrowItem->show();
    m_arrowHeadItem->show();
    updateArrowPath();
}
// 🔴【新接口】：卡牌在检测到自己指向了怪物时，会呼叫场景改变颜色！
void BattleScene::setTargetingEnemy(bool isTargetingEnemy) {
    if (m_isHoveringEnemy == isTargetingEnemy) return; // 状态没变就不管

    m_isHoveringEnemy = isTargetingEnemy;
    updateArrowPath(); // 强制刷新一次颜色
}

// 🔴 返回类型改为 Enemy*
Enemy* BattleScene::updateTargeting(const QPointF& currentPos) {
    if (!m_isTargeting) return nullptr;
    m_arrowEnd = currentPos;

    bool hitEnemy = false;
    Enemy* targetedEnemy = nullptr; // 🔴 新增：用来记录抓到的逻辑怪物

    QList<QGraphicsItem*> collisions = items(currentPos);

    for (QGraphicsItem* item : collisions) {
        // 使用 dynamic_cast 甄别它是不是一只 EnemyItem 喵！
        if (EnemyItem* enemyItem = dynamic_cast<EnemyItem*>(item)) {
            hitEnemy = true;
            // 🔴 核心咬合点：从肉体（EnemyItem）中提取出灵魂（Enemy* 逻辑指针）！
            targetedEnemy = enemyItem->getLogicEnemy();
            break;
        }
    }

    // 直接通知自己变色
    setTargetingEnemy(hitEnemy);
    updateArrowPath();

    return targetedEnemy; // 🔴 完美的交接！把怪物交还给卡牌
}

void BattleScene::stopTargeting() {
    if (!m_isTargeting) return;
    m_isTargeting = false;
    m_isHoveringEnemy = false;

    if (m_arrowItem) m_arrowItem->hide();
    if (m_arrowHeadItem) m_arrowHeadItem->hide();

    qDebug() << "[UI] Targeting stopped.";
}

void BattleScene::updateArrowPath() {
    if (!m_arrowItem || !m_arrowHeadItem) return;

    // 瞄准怪物时变金黄，平时为尖塔红
    QColor currentColor = m_isHoveringEnemy ? QColor(241, 196, 15) : QColor(231, 76, 60);

    // 🔴【变粗修改】：虚线宽度从 8 飙升到 15！极度粗壮！
    QPen linePen(currentColor, 15, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    m_arrowItem->setPen(linePen);

    // 同步更新箭头的颜色
    m_arrowHeadItem->setBrush(currentColor);
    m_arrowHeadItem->setPen(QPen(currentColor, 1));

    // 计算二次贝塞尔曲线
    QPainterPath path;
    path.moveTo(m_arrowStart);

    qreal midX = (m_arrowStart.x() + m_arrowEnd.x()) / 2.0;
    qreal offset = qAbs(m_arrowStart.x() - m_arrowEnd.x()) * 0.3 + 100;

    QPointF ctrlPoint(midX - 100, qMin(m_arrowStart.y(), m_arrowEnd.y()) - offset);

    path.quadTo(ctrlPoint, m_arrowEnd);
    m_arrowItem->setPath(path);

    updateArrowHead(m_arrowEnd, ctrlPoint);
}

void BattleScene::updateArrowHead(const QPointF& endPoint, const QPointF& controlPoint) {
    // 箭头需要“顺着”曲线到达终点的方向。
    // 在二次贝塞尔曲线中，终点的切线方向，就是控制点指向终点的方向！

    qreal dx = endPoint.x() - controlPoint.x();
    qreal dy = endPoint.y() - controlPoint.y();

    // 用 atan2 算出这个向量在平面直角坐标系中的角度（弧度制）
    qreal angleRad = qAtan2(dy, dx);

    // 弧度转角度
    qreal angleDeg = qRadiansToDegrees(angleRad);

    // 把三角箭头摆放到终点，并根据计算出的角度进行旋转！
    m_arrowHeadItem->setPos(endPoint);
    m_arrowHeadItem->setRotation(angleDeg);
}

void BattleScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // 强制调用 Qt 基类默认实现！
    // 这样 Qt 才会精准定位鼠标下方的卡牌，并触发 CardItem::mousePressEvent
    QGraphicsScene::mousePressEvent(event);
}

void BattleScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    // 将移动事件完美向下分发
    QGraphicsScene::mouseMoveEvent(event);
}

void BattleScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    // 将抬起事件完美向下分发，激活卡牌的碰撞扣血检测
    QGraphicsScene::mouseReleaseEvent(event);
}
