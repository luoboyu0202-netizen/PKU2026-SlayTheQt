#include "TopBar.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>

TopBar::TopBar(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_playerName(QStringLiteral("未知英雄")),
    m_hp(0), m_maxHp(0), m_energy(0), m_maxEnergy(0), m_gold(0),m_block(0) {

    // 设置一款高保真黑体字体
    m_uiFont = QFont("Microsoft YaHei", 16, QFont::Bold);
}

void TopBar::bindPlayer(Player* player) {
    if (!player) return;

    // 1. 初始化当前界面的显示数值
    m_playerName = player->getName();
    m_hp = player->getHp();
    m_maxHp = player->getMaxHp();
    m_energy = player->getEnergy();
    m_maxEnergy = player->getMaxEnergy();
    m_gold = player->getGold();
    m_block = player->getBlock(); // 获取初始护甲
    connect(player, &Player::blockChanged, this, &TopBar::updateBlock); // 🔴【新增】监听护甲变化

    // 2. 核心解耦：将底层数据的变动信号与 UI 的更新槽函数牢牢绑定！
    connect(player, &Player::hpChanged, this, &TopBar::updateHp);
    connect(player, &Player::energyChanged, this, &TopBar::updateEnergy);
    connect(player, &Player::goldChanged, this, &TopBar::updateGold);

    // 触发首次重绘
    update();
}

void TopBar::updateHp(int current, int max) {
    m_hp = current;
    m_maxHp = max;
    update(); // 呼叫 Qt 引擎重新调用 paint() 函数重绘该区域
}

void TopBar::updateBlock(int block) {
    m_block = block;
    update();
}

void TopBar::updateEnergy(int current, int max) {
    m_energy = current;
    m_maxEnergy = max;
    update();
}

void TopBar::updateGold(int current) {
    m_gold = current;
    update();
}

QRectF TopBar::boundingRect() const {
    // 占据屏幕最上方的一整条（假设场景宽度 1920，高度 60）
    return QRectF(0, 0, 1920, 60);
}

void TopBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 1. 绘制半透明黑色背景条 (增加 UI 层级感)
    painter->fillRect(boundingRect(), QColor(10, 10, 12, 200));

    // 设置画笔和字体
    painter->setFont(m_uiFont);
    painter->setRenderHint(QPainter::Antialiasing);

    //*/ 2. 绘制玩家名称与血条区 (左侧)
    /*QString hpText = QStringLiteral("%1 : %2 / %3").arg(m_playerName).arg(m_hp).arg(m_maxHp);

    // 画一个简单的图形血条底框
    QRectF hpBarBg(20, 15, 200, 30);
    painter->fillRect(hpBarBg, QColor(80, 20, 20)); // 暗红色底

    // 画当前血量进度
    if (m_maxHp > 0) {
        qreal hpRatio = static_cast<qreal>(m_hp) / m_maxHp;
        QRectF hpBarFill(20, 15, 200 * hpRatio, 30);
        painter->fillRect(hpBarFill, QColor(231, 76, 60)); // 明亮红色填充
    }

    // 绘制血量文本
    painter->setPen(Qt::white);
    painter->drawText(hpBarBg, Qt::AlignCenter, hpText);

    // 🔴【新增】：绘制护盾条 (如果护盾大于 0)
    if (m_block > 0) {
        QRectF blockBg(230, 15, 60, 30); // 放在血条右侧
        painter->fillRect(blockBg, QColor(41, 128, 185)); // 标志性的蔚蓝色
        painter->setPen(Qt::white);
        painter->drawText(blockBg, Qt::AlignCenter, QStringLiteral("🛡 %1").arg(m_block));
    }*/

    // 3. 绘制金币区 (中间)
    // 画个简易的黄色金币小圆点代替贴图
    painter->setBrush(QColor(241, 196, 15));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(400, 30), 12, 12);

    painter->setPen(QColor(241, 196, 15)); // 金黄色文字
    QString goldText = QStringLiteral("金币: %1").arg(m_gold);
    painter->drawText(QRectF(420, 15, 150, 30), Qt::AlignLeft | Qt::AlignVCenter, goldText);

    // // 4. 绘制费用区 (右侧稍偏中)
    // painter->setPen(QColor(52, 152, 219)); // 科技蓝文字
    // QString energyText = QStringLiteral("行动力: %1 / %2").arg(m_energy).arg(m_maxEnergy);
    // painter->drawText(QRectF(600, 15, 200, 30), Qt::AlignLeft | Qt::AlignVCenter, energyText);

    // 底边高亮分割线
    painter->setPen(QPen(QColor(80, 80, 90), 2));
    painter->drawLine(0, 60, 1920, 60);
}
