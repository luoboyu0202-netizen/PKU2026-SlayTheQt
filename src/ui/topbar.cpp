#include "TopBar.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include "../logic/GlobalSaveData.h" // 🔴 必须包含
#include <QGraphicsSceneMouseEvent> // 🔴 必须引入鼠标事件

TopBar::TopBar(QGraphicsItem* parent)
    : QGraphicsObject(parent), m_playerName(""), m_hp(0), m_maxHp(0),
    m_energy(0), m_maxEnergy(0), m_gold(0), m_block(0) {
    m_uiFont = QFont("Arial", 16, QFont::Bold);

    // ========================================================
    // 🃏 挂载全局牌堆图标（父对象设为 this，跟着 TopBar 混！）
    // ========================================================
    m_masterDeckPile = new PileItem("总牌组", this);

    // 🔴 极其精确的狙击：完美对齐我们在 RewardScreen 写的终点坐标！
    m_masterDeckPile->setPos(1400, 32);

    // 连通点击神经
    connect(m_masterDeckPile, &PileItem::clicked, this, &TopBar::onDeckPileClicked);

    // 初始化退出按钮的位置 (右上角区域)
    m_exitBtnRect = QRectF(1450, 10, 120, 30);
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
    return QRectF(0, 0, 1600, 60);
}

void TopBar::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);

    // ---- 深黑褐背景 ----
    painter->fillRect(QRectF(0, 0, 1600, 48), QColor(45, 38, 30, 255));

    painter->setPen(QPen(QColor(220, 180, 100), 2));
    painter->drawLine(0, 0, 1600, 0);
    painter->setPen(QPen(QColor(180, 150, 100), 1.5));
    painter->drawLine(0, 48, 1600, 48);
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

    // ---- 3. 金币 (🔴 挪到 X=1200 的位置，完美避开 1400 的牌库) ----
    painter->setBrush(QColor(230, 190, 50));
    painter->setPen(QPen(QColor(160, 120, 20), 1.5));
    painter->drawEllipse(QPointF(1200, 24), 8, 8);

    painter->setPen(QColor(230, 190, 50));
    // 文字也跟着挪过来
    painter->drawText(QRectF(1215, 0, 100, 48), Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_gold));

    // 🔴 【新增】绘制“保存并退出”按钮
    painter->setBrush(QColor(60, 40, 40, 200)); // 按钮底色
    painter->setPen(QPen(QColor(220, 100, 100), 1.5)); // 红色描边
    painter->drawRoundedRect(m_exitBtnRect, 5, 5); // 圆角矩形

    painter->setPen(QColor(255, 230, 230));
    QFont exitFont("Arial", 12, QFont::Bold);
    painter->setFont(exitFont);
    painter->drawText(m_exitBtnRect, Qt::AlignCenter, "保存并退出");
}

// ========================================================
// 🔄 刷新数字大屏
// ========================================================
void TopBar::refreshDeckCount() {
    int count = GlobalSaveData::getInstance()->deckIds.size();
    m_masterDeckPile->updateCount(count);
}

// ========================================================
// 👁️ 召唤阅兵结界：把活儿甩给最高司令部！
// ========================================================
void TopBar::onDeckPileClicked() {
    qDebug() << "[TopBar] 玩家点击了总牌库！向最高司令部请求全屏支援！";
    emit deckViewRequested(); // 🔴 发射信号！
}

// 🔴 【新增】鼠标点击事件探测
void TopBar::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_exitBtnRect.contains(event->pos())) {
        qDebug() << "[TopBar] 玩家点击了保存并退出！";
        // 1. 命令数据中心立刻写盘！
        GlobalSaveData::getInstance()->saveToFile();
        // 2. 发射信号让上层切回主界面
        emit returnToTitleRequested();
        event->accept();
        return;
    }
    // 不要忘记调用基类方法，否则其他点击事件（如牌堆）可能会失效
    QGraphicsObject::mousePressEvent(event);
}