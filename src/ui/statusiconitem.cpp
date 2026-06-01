#include "StatusIconItem.h"
#include <QPainter>
#include <QToolTip>
#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QAbstractAnimation>
#include <QPropertyAnimation>

StatusIconItem::StatusIconItem(StatusType type, int amount, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_type(type), m_amount(amount) {

    // 🔴 必须开启这行，鼠标悬停事件才会生效！
    setAcceptHoverEvents(true);

    // 根据不同状态，加载不同的图片和提示语
    switch (type) {
    case StatusType::Strength:
        m_icon = QPixmap(":/resources/images/strength.png"); // 请换成你实际的图片路径喵
        m_tooltipText = QStringLiteral("力量\n攻击伤害增加。");
        m_isDebuff = false;
        break;
    case StatusType::Vulnerable:
        m_icon = QPixmap(":/resources/images/vulnerable.png");
        m_tooltipText = QStringLiteral("易伤\n受到的攻击伤害增加 50%。");
        m_isDebuff = true;
        break;
    case StatusType::Weak:
        m_icon = QPixmap(":/resources/images/weak.png");
        m_tooltipText = QStringLiteral("虚弱\n造成的攻击伤害减少 25%。");
        m_isDebuff = true;
        break;
    case StatusType::Dexterity:
        m_icon = QPixmap(":/resources/images/dexterity.png");
        m_tooltipText = QStringLiteral("敏捷\n获得的格挡增加。");
        m_isDebuff = true;
        break;
    case StatusType::Frail:
        m_icon = QPixmap(":/resources/images/frail.png"); // 🔴 记得找一张绿色的心碎或者盾牌破裂的图标放进资源里喵！
        m_tooltipText = QStringLiteral("脆弱\n从卡牌获得的格挡减少 25%。");
        m_isDebuff = true; // 🔴 给它画上红色的边框！
        break;
    case StatusType::Metallicize:
        m_icon = QPixmap(":/resources/images/icons/metallicize_icon.png");
        // 🔴 动态魔法：读取初始的 m_amount 嵌进文本里！
        m_tooltipText = QStringLiteral("金属化\n在你的回合结束时，获得 %1 点格挡。").arg(m_amount);
        m_isDebuff = false; // 增益状态，画高贵的蓝框！
        break;

    case StatusType::FireSource:
        m_icon = QPixmap(":/resources/images/icons/firesource_icon.png");
        m_tooltipText = QStringLiteral("薪火之源\n在你的回合开始时，获得 %1 点能量。").arg(m_amount);
        m_isDebuff = false;
        break;

    case StatusType::DarkEmbrace:
        m_icon = QPixmap(":/resources/images/icons/darkembrace_icon.png");
        m_tooltipText = QStringLiteral("黑暗之拥\n每当有一张牌被消耗时，抽 %1 张牌。").arg(m_amount);
        m_isDebuff = false;
        break;
    case StatusType::Barricade:
        m_icon = QPixmap(":/resources/images/icons/barricade_icon.png");
        m_tooltipText =QStringLiteral("壁垒\n格挡不再在你的回合开始时消失。");
        m_isDebuff = false;
        break;
        // ========================================================
    // ⛓️ 镣铐专属 UI
    // ========================================================
    case StatusType::Shackled:
        m_icon = QPixmap(":/resources/images/icons/shackled_icon.png");
        // 原作中是一个被铁链锁住的重物图标
        m_tooltipText =QStringLiteral("镣铐\n回合结束时，恢复 %1 点力量。").arg(m_amount);
        break;

    case StatusType::HellFiend:
        m_icon = QPixmap(":/resources/images/icons/HellFiend_icon.png");
        // 原作中是一个被铁链锁住的重物图标
        m_tooltipText =QStringLiteral("地狱狂徒\n每当你抽到一张名字中含有“打击”的牌时，立即将其打出。").arg(m_amount);
        break;
        // ... 其他状态 ...
    case StatusType::Confusion:
        m_icon = QPixmap(":/resources/images/icons/HellFiend_icon.png");
        // 原作中是一个被铁链锁住的重物图标
        m_tooltipText =QStringLiteral("混乱\n你感到蛇教的力量在祝福着你，但代价是...?").arg(m_amount);
        break;
        // ... 其他状态 ...
    }

    // ========================================================
    // ✨【图标出场秀】：从小变大并带有一点回弹
    // ========================================================
    this->setScale(0.0);
    QPropertyAnimation* pop = new QPropertyAnimation(this, "scale");
    pop->setDuration(400);
    pop->setStartValue(0.0);
    pop->setEndValue(1.0);
    pop->setEasingCurve(QEasingCurve::OutBack); // 典型的 Q 弹曲线
    pop->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF StatusIconItem::boundingRect() const {
    return QRectF(0, 0, 32, 32); // 图标固定大小 32x32
}

void StatusIconItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // 1. 画底框（Buff 蓝框，Debuff 红框）
    painter->setPen(m_isDebuff ? QPen(Qt::red, 2) : QPen(Qt::cyan, 2));
    painter->setBrush(QColor(0, 0, 0, 150)); // 半透明黑底
    painter->drawRect(boundingRect());

    // 2. 画中间的图标
    if (!m_icon.isNull()) {
        painter->drawPixmap(2, 2, 28, 28, m_icon);
    }

    // 3. 🔴【绝招：带黑边的数字】
    QString amountStr = QString::number(m_amount);
    QFont font = painter->font();
    font.setPixelSize(16);
    font.setBold(true);
    painter->setFont(font);

    // 第一层：在右下角偏一点的位置，画黑色的字（伪装成阴影描边）
    painter->setPen(Qt::black);
    painter->drawText(QRectF(1, 1, 32, 32), Qt::AlignRight | Qt::AlignBottom, amountStr);

    // 第二层：在正右下角，画白色的字，叠加起来就是黑边白字！极度清晰！
    painter->setPen(Qt::white);
    painter->drawText(QRectF(0, 0, 32, 32), Qt::AlignRight | Qt::AlignBottom, amountStr);
}

void StatusIconItem::setAmount(int amount) {
    m_amount = amount;

    // 🔴 力量为负数时，图标依然要坚挺地显示！
    if (m_amount == 0) {
        this->hide();
    } else {
        this->show();
        this->update();
    }

    // ========================================================
    // 🔴 核心追猎：当层数变化时，重新生成最新的动态提示词！
    // ========================================================
    switch (m_type) {
    // 对于有数值加成的动态文本，重新读取最新的 m_amount：
    case StatusType::Metallicize:
        m_tooltipText = QStringLiteral("金属化\n在你的回合结束时，获得 %1 点格挡。").arg(m_amount);
        break;
    case StatusType::FireSource:
        m_tooltipText = QStringLiteral("薪火之源\n在你的回合开始时，获得 %1 点能量。").arg(m_amount);
        break;
    case StatusType::DarkEmbrace:
        m_tooltipText = QStringLiteral("黑暗之拥\n每当有一张牌被消耗时，抽 %1 张牌。").arg(m_amount);
        break;

    // 以后如果有其他随层数变化的提示词，也可以加在这里喵！
    default:
        break;
    }

    update(); // 数值改变，立刻申请重绘数字黑边
}

// 4. 处理鼠标悬停
void StatusIconItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    // 在鼠标当前位置弹出系统的原生提示框，极其省事且丝滑！
    QToolTip::showText(event->screenPos(), m_tooltipText);
}

void StatusIconItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    QToolTip::hideText(); // 鼠标移开，关闭提示框
}
