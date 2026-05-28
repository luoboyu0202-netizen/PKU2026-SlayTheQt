#include "RewardScreen.h"
#include "../logic/GlobalSaveData.h"
#include <QDebug>

// ==========================================
// 🎟️ 单个条目样式优化 (更贴近原版)
// ==========================================
RewardItemButton::RewardItemButton(RewardType type, const QString& text, QWidget* parent)
    : QPushButton(text, parent), m_type(type)
{
    this->setFixedSize(400, 65); // 稍微加宽一点，显得更大气
    this->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(20, 25, 30, 230);" // 偏蓝黑的暗色调
        "   color: #e0e0e0;"
        "   font-size: 18px;"
        "   text-align: left;"
        "   padding-left: 20px;"
        "   border: 1px solid #4a5a6a;"
        "}"
        "QPushButton:hover { background-color: rgba(40, 50, 60, 255); border: 1px solid #7aa0c0; color: white; }"
        "QPushButton:disabled { background-color: rgba(10, 10, 10, 150); color: #555555; border: 1px solid #222222; text-decoration: line-through; }"
        );
}

// ==========================================
// 🏆 战利品界面：幽灵悬浮层与下坠动画
// ==========================================
RewardScreen::RewardScreen(QWidget *parent) : QWidget(parent) {
    // 1. 铺满整个屏幕的半透明黑纱（这就是保留战斗背景的秘诀！）
    this->setFixedSize(1600, 900);
    this->setStyleSheet("RewardScreen { background-color: rgba(0, 0, 0, 160); }"); // 160透明度，隐约透出后面的怪和背景

    // 2. 制作那块砸下来的“黑板”
    m_boardWidget = new QWidget(this);
    m_boardWidget->setFixedSize(500, 700); // 黑板的物理大小
    // 给黑板加上杀戮尖塔那种铁灰色的背景和边缘
    m_boardWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #2b2f36;" // 铁灰色
        "   border: 3px solid #1a1c20;"
        "   border-radius: 8px;"
        "}"
        // 🔴 极其关键：防止内部的按钮继承黑板的边框样式
        "QPushButton { border-radius: 0px; }"
        );

    // 3. 组装黑板内部的内容 (标题、列表、按钮)
    QVBoxLayout* boardLayout = new QVBoxLayout(m_boardWidget);
    boardLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    boardLayout->setContentsMargins(0, 30, 0, 30);

    QLabel* titleLabel = new QLabel("获取战利品", m_boardWidget);
    titleLabel->setStyleSheet("color: #FFD700; font-size: 36px; font-weight: bold; border: none; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    boardLayout->addWidget(titleLabel);
    boardLayout->addSpacing(30);

    // 专门装奖励的容器
    m_listLayout = new QVBoxLayout();
    m_listLayout->setAlignment(Qt::AlignTop);
    m_listLayout->setSpacing(10);
    boardLayout->addLayout(m_listLayout);

    boardLayout->addStretch();

    // 继续按钮
    m_proceedButton = new QPushButton("继 续", m_boardWidget);
    m_proceedButton->setFixedSize(200, 60);
    m_proceedButton->setStyleSheet(
        "QPushButton { background-color: #3b5a7a; color: white; font-size: 22px; font-weight: bold; border: 2px solid #5a7a9a; border-radius: 5px; }"
        "QPushButton:hover { background-color: #4b7aa0; border-color: #7aa0c0; }"
        );
    connect(m_proceedButton, &QPushButton::clicked, this, &RewardScreen::onProceedClicked);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_proceedButton);
    boardLayout->addLayout(btnLayout);

    // ==========================================
    // 💥 核心魔法：注入弹簧动画引擎！
    // ==========================================
    m_dropAnimation = new QPropertyAnimation(m_boardWidget, "pos", this);
    m_dropAnimation->setDuration(800); // 下落全过程耗时 0.8 秒
    m_dropAnimation->setEasingCurve(QEasingCurve::OutBounce); // 🔴 就是这个！带有极其真实的重力回弹感！
}

// 播放登场动画
void RewardScreen::dropDown() {
    this->raise(); // 确保遮罩在最顶层
    this->show();

    // 设置动画起点：屏幕正上方（Y坐标为负数，完全藏在屏幕外面）
    int startX = (this->width() - m_boardWidget->width()) / 2;
    int startY = -m_boardWidget->height();

    // 设置动画终点：屏幕正中央
    int endX = startX;
    int endY = (this->height() - m_boardWidget->height()) / 2;

    m_dropAnimation->setStartValue(QPoint(startX, startY));
    m_dropAnimation->setEndValue(QPoint(endX, endY));

    // 播放砸下动画！
    m_dropAnimation->start();
}

// ... loadRewards() 和 拾取点击的逻辑代码保持不变（和上一个版本一样即可）...
void RewardScreen::loadRewards(const BattleResult& result) {
    QLayoutItem* item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    if (result.rewardGold > 0) {
        RewardItemButton* goldBtn = new RewardItemButton(RewardItemButton::Gold, QString("💰 获得 %1 金币").arg(result.rewardGold));
        goldBtn->goldAmount = result.rewardGold;
        connect(goldBtn, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(goldBtn);
    }
    for (const QString& relicId : result.rewardRelicIds) {
        RewardItemButton* relicBtn = new RewardItemButton(RewardItemButton::Relic, QString("🏺 获得遗物：") + relicId);
        relicBtn->relicId = relicId;
        connect(relicBtn, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(relicBtn);
    }
    if (result.hasCardReward) {
        RewardItemButton* cardBtn = new RewardItemButton(RewardItemButton::Card, "🃏 增加一张卡牌到你的牌组");
        connect(cardBtn, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(cardBtn);
    }
}

// ==========================================
// 💥 顶级特效引擎：瞬间消失与纯净图标飞行
// ==========================================
void RewardScreen::animateAndRemoveItem(RewardItemButton* btn) {
    // 【极其关键】：在 btn 被 delete 之前，赶紧把它的数据复印保存下来！
    RewardItemButton::RewardType type = btn->getType();
    QString rId = btn->relicId;
    int gAmount = btn->goldAmount;

    QPoint startCenter = btn->mapTo(this, btn->rect().center());

    btn->hide();
    m_listLayout->removeWidget(btn);
    btn->deleteLater();

    // ==========================================
    // 3. 🖼️ 量身定制：只让纯净的“实体图标”飞出去！
    // ==========================================
    QLabel* ghostIcon = new QLabel(this);
    ghostIcon->setAttribute(Qt::WA_TransparentForMouseEvents); // 幽灵不挡鼠标点击

    if (btn->getType() == RewardItemButton::Gold) {
        // 💰 纯净的金币图标
        ghostIcon->setFixedSize(30, 30);
        ghostIcon->setStyleSheet("background-color: #F1C40F; border-radius: 15px; border: 2px solid #D4AC0D;");
    }
    else if (btn->getType() == RewardItemButton::Relic) {
        // 🏺 真正的遗物贴图！(直接复用你 RelicItem 里的路径逻辑)
        ghostIcon->setFixedSize(48, 48);
        QPixmap pix(QString(":/resources/images/relics/%1.png").arg(btn->relicId));
        if (!pix.isNull()) {
            ghostIcon->setPixmap(pix.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        } else {
            // 防闪退兜底：没图就画个带字的灰圈
            ghostIcon->setStyleSheet("background-color: #2c3e50; border-radius: 24px; border: 2px solid #bdc3c7; color: white; font-weight: bold;");
            ghostIcon->setText(btn->relicId.left(3));
            ghostIcon->setAlignment(Qt::AlignCenter);
        }
    }
    else {
        // 🃏 卡牌背面图标
        ghostIcon->setFixedSize(40, 56);
        ghostIcon->setStyleSheet("background-color: #3b5a7a; border: 2px solid #5a7a9a; border-radius: 4px;");
    }

    // 把纯净图标的中心点，严丝合缝地对齐到刚刚被你点掉的按钮中心
    ghostIcon->move(startCenter - QPoint(ghostIcon->width() / 2, ghostIcon->height() / 2));
    ghostIcon->show();

    // ==========================================
    // 4. 🎯 精确计算入局坐标
    // ==========================================
    QPoint targetPos;
    GlobalSaveData* save = GlobalSaveData::getInstance();

    if (btn->getType() == RewardItemButton::Gold) {
        targetPos = QPoint(1480, 24);
    }
    else if (btn->getType() == RewardItemButton::Relic) {
        // 🔴 极其聪明的位置预判：
        // 因为你在 onRewardItemClicked 里已经把遗物 append 进 save 了，
        // 所以现在 size() - 1 就是它在 RelicTray 里的绝对索引位！
        int currentRelicIndex = save->relicIds.size() - 1;
        if (currentRelicIndex < 0) currentRelicIndex = 0;

        int trayStartX = 20;
        int trayStartY = 60;
        int spacing = 5;

        targetPos = QPoint(trayStartX + currentRelicIndex * (48 + spacing), trayStartY);
    }
    else {
        targetPos = QPoint(1400, 24);
    }

    // ==========================================
    // 5. ✈️ 纯享版平滑飞行特效
    // ==========================================
    // 🔴 这一次我们不改变大小了，就让原汁原味的 48x48 遗物图飞过去！
    QPropertyAnimation* flyAnim = new QPropertyAnimation(ghostIcon, "pos");
    flyAnim->setStartValue(ghostIcon->pos());
    flyAnim->setEndValue(targetPos);
    flyAnim->setDuration(550); // 0.55秒飞达，快准狠
    flyAnim->setEasingCurve(QEasingCurve::InOutQuad);

    // 🔴 核心魔法：当飞行到达终点时！
    connect(flyAnim, &QPropertyAnimation::finished, this, [this, ghostIcon, type, rId, gAmount]() {
        ghostIcon->deleteLater(); // 1. 幽灵功成身退，销毁自己

        // 2. 扣动扳机！通知司令部更新 UI 并且数据入账！
        if (type == RewardItemButton::Relic) {
            emit relicFlightFinished(rId);
        } else if (type == RewardItemButton::Gold) {
            emit goldFlightFinished(gAmount);
        }
    });

    flyAnim->start();
}

// ==========================================
// 🖱️ 点击事件：交接给特效引擎
// ==========================================
// 1. 修改点击事件：变得极其干净，只负责触发特效
void RewardScreen::onRewardItemClicked() {
    RewardItemButton* clickedBtn = qobject_cast<RewardItemButton*>(sender());
    if (!clickedBtn) return;

    if (clickedBtn->getType() == RewardItemButton::Gold ||
        clickedBtn->getType() == RewardItemButton::Relic) {

        // 🔴 直接呼叫特效引擎！数据入账交给飞行结束后的信号去办！
        animateAndRemoveItem(clickedBtn);
    }
    else if (clickedBtn->getType() == RewardItemButton::Card) {
        qDebug() << "[RewardScreen] 准备弹出卡牌三选一...";
    }
}

void RewardScreen::onProceedClicked() {
    emit proceedRequested();
}