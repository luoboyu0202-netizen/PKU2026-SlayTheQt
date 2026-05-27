#include "TopBar.h"
#include <QPainter>
#include <QPainterPath>

TopBar::TopBar(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_playerName(""), m_hp(0), m_maxHp(0),
      m_energy(0), m_maxEnergy(0), m_gold(0), m_block(0) {
    m_uiFont = QFont("Arial", 16, QFont::Bold); // 使用更通用的字体
}

void TopBar::bindPlayer(Player* player) {
    if (!player) return;
    m_playerName = player->getName();
    m_hp = player->getHp();
    m_maxHp = player->getMaxHp();
    m_energy = player->getEnergy();
    m_maxEnergy = player->getMaxEnergy();
    m_gold = player->getGold();
    m_block = player->getBlock();

    qDebug() << "[TopBar] Binding player:" << m_playerName << "HP:" << m_hp << "/" << m_maxHp;

    connect(player, &Player::hpChanged, this, &TopBar::updateHp);
    connect(player, &Player::energyChanged, this, &TopBar::updateEnergy);
    connect(player, &Player::goldChanged, this, &TopBar::updateGold);
    connect(player, &Player::blockChanged, this, &TopBar::updateBlock);
    update();
}

void TopBar::updateHp(int current, int max) { m_hp = current; m_maxHp = max; update(); }
void TopBar::updateBlock(int block) { m_block = block; update(); }
void TopBar::updateEnergy(int current, int max) { m_energy = current; m_maxEnergy = max; update(); }
void TopBar::updateGold(int current) { m_gold = current; update(); }

QRectF TopBar::boundingRect() const {
    return QRectF(0, 0, 1920, 60);
}

void TopBar::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // ---- 深黑褐背景（稍微调亮，增加辨识度） ----
    painter->fillRect(QRectF(0, 0, 1920, 48), QColor(45, 38, 30, 255));
    
    // 亮色顶边确认渲染
    painter->setPen(QPen(QColor(220, 180, 100), 2));
    painter->drawLine(0, 0, 1920, 0);

    // ---- 底部金色装饰线 ----
    painter->setPen(QPen(QColor(180, 150, 100), 1.5));
    painter->drawLine(0, 48, 1920, 48);

    painter->setFont(m_uiFont);

    // ---- 1. 角色名称 ----
    painter->setPen(QColor(240, 240, 240));
    painter->drawText(QRectF(20, 0, 200, 48), Qt::AlignLeft | Qt::AlignVCenter, m_playerName);

    // ---- 2. HP（心形图标 + 文字） ----
    QPainterPath heart;
    qreal hx = 240, hy = 24;
    heart.moveTo(hx, hy + 3);
    heart.cubicTo(hx, hy - 2, hx - 8, hy - 5, hx - 8, hy + 2);
    heart.cubicTo(hx - 8, hy + 9, hx, hy + 13, hx, hy + 18);
    heart.cubicTo(hx, hy + 13, hx + 8, hy + 9, hx + 8, hy + 2);
    heart.cubicTo(hx + 8, hy - 5, hx, hy - 2, hx, hy + 3);
    painter->setBrush(QColor(220, 40, 40));
    painter->setPen(QPen(QColor(150, 20, 20), 1.5));
    painter->drawPath(heart);

    painter->setPen(QColor(255, 230, 220));
    QString hpText = QString("%1 / %2").arg(m_hp).arg(m_maxHp);
    painter->drawText(QRectF(hx + 20, 0, 150, 48), Qt::AlignLeft | Qt::AlignVCenter, hpText);

    // ---- 3. 金币 ----
    painter->setBrush(QColor(230, 190, 50));
    painter->setPen(QPen(QColor(160, 120, 20), 1.5));
    painter->drawEllipse(QPointF(1780, 24), 8, 8);

    painter->setPen(QColor(230, 190, 50));
    painter->drawText(QRectF(1795, 0, 100, 48), Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_gold));
}
