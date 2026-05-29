#include "PlayerItem.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPropertyAnimation>
#include <QPixmap> // 别忘了包含图片类头文件喵！

PlayerItem::PlayerItem(Player* logicPlayer, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_logicPlayer(logicPlayer) {

    // 1. 🔴【新核心】：在构造函数里，加载主角的帅气图片！
    // 假设你把铁甲战士的图片放在了项目资源路径 ":/images/player/ironclad.png" 喵！
    // 请务必确保这里的路径是你实际放入资源文件的路径喵！
    if (!m_playerPixmap.load(":resources/images/ironclad.png")) {
        qDebug() << "[PlayerItem UI Error] Failed to load player pixmap! Check the path喵!";
    }

    // 2. 初始化本地缓存数值
    m_hp = logicPlayer->getHp();
    m_maxHp = logicPlayer->getMaxHp();
    m_block = logicPlayer->getBlock();

    // 🔴【神经连线 A】：监听血量变化，附赠超强果汁感的受击震动！
    connect(logicPlayer, &Player::hpChanged, this, [this](int cur, int max) {
        if (cur < m_hp) {
            // 玩家掉血了！立刻触发向左抽搐震动动画
            QPropertyAnimation* shake = new QPropertyAnimation(this, "x");
            qreal baseX = this->x();
            shake->setDuration(300); // 持续 0.3 秒
            shake->setKeyValueAt(0, baseX);
            shake->setKeyValueAt(0.2, baseX - 15); // 受击向左猛退
            shake->setKeyValueAt(0.4, baseX + 15); // 向右反弹
            shake->setKeyValueAt(0.6, baseX - 10);
            shake->setKeyValueAt(0.8, baseX + 10);
            shake->setKeyValueAt(1, baseX);        // 归位
            shake->start(QAbstractAnimation::DeleteWhenStopped);
        }
        m_hp = cur; m_maxHp = max;
        update(); // 申请重画
    });

    // 🔴【神经连线 B】：监听格挡变化
    connect(logicPlayer, &Player::blockChanged, this, [this](int blk) {
        m_block = blk;
        update();
    });

    // 🔴【神经连线 C】：接通状态背包！全自动管理增益/减益图标
    connect(logicPlayer->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType type, int amount) {
        if (amount > 0) {
            if (!m_statusIcons.contains(type)) {
                // 图标跟着主角移动震动
                StatusIconItem* newIcon = new StatusIconItem(type, amount, this);
                m_activeStatusList.append(type);
                m_statusIcons.insert(type, newIcon);
            } else {
                m_statusIcons[type]->setAmount(amount);
            }
        }
        else {
            if (m_statusIcons.contains(type)) {
                StatusIconItem* deadIcon = m_statusIcons.take(type);
                m_activeStatusList.removeAll(type);
                delete deadIcon;
            }
        }
        layoutStatusIcons(); // 呼叫排版
    });
}

QRectF PlayerItem::boundingRect() const {
    // 🔴 核心修复：把总高度从 350 拉长到 450！
    // 这样底部边界能到达 Y = 170，完美包住位于 108 的状态图标！
    return QRectF(-200, -280, 450, 450);
}

void PlayerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    // ========================================================
    // 🔴 【主角自适应核心算法】
    // ========================================================
    // 🟢 1. 拔高体型：高度从 220 强力拉高到 270，更显魁梧！
    int playerH = 270;
    int playerW = 200; // 兜底宽度

    if (!m_playerPixmap.isNull()) {
        qreal aspectRatio = static_cast<qreal>(m_playerPixmap.width()) / m_playerPixmap.height();
        playerW = static_cast<int>(playerH * aspectRatio);
    }

    // ========================================================
    // 🟢 A. 绘制自适应后的主角肉体（完美落地版）
    // ========================================================
    // 🔴 2. 核心下坠魔法：血条在 Y=90，我们让他的脚踩在 Y=80 的位置，正好留出 10 像素的完美呼吸感！
    int feetY = 80;

    if (!m_playerPixmap.isNull()) {
        // 画图时，Y 坐标加上 feetY 偏移量，强制往下拽！
        painter->drawPixmap(-playerW / 2, -playerH + feetY, playerW, playerH, m_playerPixmap);
    } else {
        painter->setBrush(Qt::blue);
        painter->drawRect(-playerW / 2, -playerH + feetY, playerW, playerH);
    }

    // 🔴 宽度从 140 加长到 180，高度从 12 加粗到 16！
    int barW = 270;
    int barH = 18;

    // 🔴 为了保持居中平衡向右伸长，把起点向左移一点 (-80)，这样整体更长更稳！
    int barX = -80;
    int barY = 90;

    // 背景层（暗红色）
    painter->setBrush(QColor(80, 20, 20));
    painter->setPen(Qt::NoPen);
    painter->drawRect(barX, barY, barW, barH);

    // 当前血量层 —— 🔴 亮绿色改为亮红色喵！
    if (m_maxHp > 0) {
        qreal hpRatio = static_cast<qreal>(m_hp) / m_maxHp;
        int currentHpW = static_cast<int>(barW * hpRatio);
        // 🔴 使用一种更鲜艳的红色，例如 crimson 或者 firebrick 喵！
        painter->setBrush(QColor(220, 20, 60)); // Crimson 红喵！
        painter->drawRect(barX, barY, currentHpW, barH);
    }

    // 🔴 绘制血量文字 —— 字体从 9 放大到 11 喵！
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 11, QFont::Bold));
    QString hpText = QString("%1/%2").arg(m_hp).arg(m_maxHp);
    painter->drawText(barX, barY, barW, barH, Qt::AlignCenter, hpText);

    // ========================================================
    // 🎨 3. 画格挡/盾牌 —— 🔴 完美的对称六边形圣盾！
    // ========================================================
    if (m_block > 0) {
        // 🔴 盾牌尺寸也跟着放大，让它更有安全感！
        int shieldX = barX - 20; // 紧贴着血条左边稍微重叠一点
        int shieldY = barY - 6;  // 稍微比血条高一点点

        painter->setBrush(QColor(0, 150, 255));
        painter->setPen(QPen(Qt::white, 2)); // 盾牌的白边也加粗到 2！

        // 🔴【核心美化】：通过严密的数学计算，画出一个完美的对称护盾喵！
        QPoint points[6] = {
            QPoint(shieldX + 12, shieldY),       // 顶部中心尖端
            QPoint(shieldX + 24, shieldY + 4),   // 右上角
            QPoint(shieldX + 24, shieldY + 18),  // 右下边缘
            QPoint(shieldX + 12, shieldY + 28),  // 底部极尖端（更锐利）
            QPoint(shieldX, shieldY + 18),       // 左下边缘
            QPoint(shieldX, shieldY + 4)         // 左上角
        };
        painter->drawPolygon(points, 6);

        // 🔴 盾牌之上的格挡数字 —— 字体放大到 12 喵！
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 12, QFont::Bold));
        // 将绘制文本的矩形区域对齐到刚才画的 24x28 的盾牌中心
        painter->drawText(shieldX, shieldY, 24, 28, Qt::AlignCenter, QString::number(m_block));
    }
}

// ========================================================
// ⚙️ 4. 全自动排版工人：永远让 Buff 乖乖躺在血条下方！
// ========================================================
void PlayerItem::layoutStatusIcons() {
    // 🔴 完美贴合新血条的左边缘！
    int startX = -80;

    // 🔴 血条在 70，厚度 16，底部是 86。留出一点点空隙，放在 92 最完美！
    int startY = 108;

    for (int i = 0; i < m_activeStatusList.size(); ++i) {
        StatusType currentType = m_activeStatusList[i];
        StatusIconItem* icon = m_statusIcons[currentType];

        icon->setPos(startX + (i * 36), startY);
    }
}