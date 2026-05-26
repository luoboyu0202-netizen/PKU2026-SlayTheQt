#include "EnergyWidget.h"
#include <QPainter>
#include <QPolygonF>
#include <QPointF>
#include <QFont>
#include <cmath>

EnergyWidget::EnergyWidget(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_currentEnergy(0), m_maxEnergy(0), m_isEmpty(false) {

    // 能量球通常层级很高，确保它压在卡牌回归轨迹的下方，但处于普通背景上方喵
    setZValue(50);
}

void EnergyWidget::bindPlayer(Player* player) {
    if (!player) return;

    m_currentEnergy = player->getEnergy();
    m_maxEnergy = player->getMaxEnergy();
    m_isEmpty = (m_currentEnergy == 0);

    // 牢牢死磕底层费用变动信号！
    connect(player, &Player::energyChanged, this, &EnergyWidget::onEnergyChanged);
    update();
}

void EnergyWidget::onEnergyChanged(int current, int max) {
    m_currentEnergy = current;
    m_maxEnergy = max;

    // 🔴【核心视觉开关】：如果费用归零，立刻激活空能封印！
    bool emptyState = (current == 0);
    if (m_isEmpty != emptyState) {
        m_isEmpty = emptyState;
    }

    update(); // 呼叫 Qt 刷帧重绘
}

QRectF EnergyWidget::boundingRect() const {
    // 以中心 (0,0) 为原点，半径为 50 像素构建 100x100 的正方形碰撞框
    return QRectF(-50, -50, 100, 100);
}

void EnergyWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ====================================================
    // 1. 利用三角函数算法，现场构建完美的尖顶正六边形喵！
    // ====================================================
    QPolygonF hexagon;
    qreal radius = 45.0; // 六边形半径
    for (int i = 0; i < 6; ++i) {
        // 每个顶点间隔 60 度，旋转 90 度让它变成“尖顶”形态（对标原版）
        qreal angle = i * 60.0 * 3.14159265 / 180.0 - (3.14159265 / 2.0);
        hexagon << QPointF(radius * std::cos(angle), radius * std::sin(angle));
    }

    // ====================================================
    // 2. 状态机动态着色：控能还是空能？
    // ====================================================
    QColor bodyColor, borderColor;

    if (m_isEmpty) {
        // 🔴【空能状态】：死灰、枯竭、暗淡
        bodyColor = QColor(44, 62, 80, 220);    // 深灰蓝底
        borderColor = QColor(127, 140, 141);    // 枯草灰边
    } else {
        // 🟡【活跃状态】：铁甲战士标志性的烈火黄金能量！
        bodyColor = QColor(230, 126, 34, 240);  // 炽热橙黄底
        borderColor = QColor(241, 196, 15);     // 闪耀纯金边
    }

    // 绘制六边形晶石内衬
    painter->setPen(QPen(borderColor, 3));
    painter->setBrush(bodyColor);
    painter->drawPolygon(hexagon);

    // 绘制一个内缩的装饰性细边框，增加 3A 级的图元精致感
    painter->setPen(QPen(QColor(255, 255, 255, m_isEmpty ? 30 : 100), 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPointF(0, 0), 35, 35);

    // ====================================================
    // 3. 能量数字绘制 (居中激荡 - 原汁原味 3/3 格式)
    // ====================================================
    // 如果空能了，数字变成微弱的灰色；有能时，呈现绝对高亮的纯白
    painter->setPen(m_isEmpty ? QColor(189, 195, 199) : Qt::white);

    // 稍微缩小一点点字号，为了完美容纳下 "3/3" 这三个字符
    QFont font("Arial", 22, QFont::Bold);
    painter->setFont(font);

    // 核心改动：直接拼出 "当前/最大" 的经典格式喵！
    QString energyText = QStringLiteral("%1/%2").arg(m_currentEnergy).arg(m_maxEnergy);

    // 依然是霸气的绝对居中对齐绘制
    painter->drawText(QRectF(-50, -50, 100, 100), Qt::AlignCenter, energyText);
}