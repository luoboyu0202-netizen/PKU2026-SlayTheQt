#include "RewardScreen.h"
#include "../logic/GlobalSaveData.h"
#include <QDebug>
#include "logic/cardfactory.h"
#include "logic/RelicFactory.h" // 🔴 极其关键：引入你的遗物工厂，用来查户口！

// ==========================================
// 🎟️ 单个条目样式优化 (更贴近原版)
// ==========================================
// ==========================================
// 🎟️ 单个条目样式优化 (支持高清图标排版)
// ==========================================
RewardItemButton::RewardItemButton(RewardType type, const QString& text, QWidget* parent)
    : QPushButton(text, parent), m_type(type)
{
    this->setFixedSize(400, 65);

    // 🔴 开启图标与文字的完美间距
    this->setIconSize(QSize(40, 40));

    this->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(20, 25, 30, 230);"
        "   color: #e0e0e0;"
        "   font-size: 20px;"             // 字体稍微加大一点，更有气势
        "   font-weight: bold;"           // 战利品名字必须加粗
        "   text-align: left;"
        "   padding-left: 20px;"          // 左边留出边距
        "   border: 1px solid #4a5a6a;"
        "   border-radius: 4px;"          // 微微的圆角，更显高级
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(45, 55, 65, 255);"
        "   border: 2px solid #F1C40F;"   // 悬停时变成极度诱惑的金边！
        "   color: white;"
        "}"
        "QPushButton:disabled {"
        "   background-color: rgba(10, 10, 10, 150);"
        "   color: #555555;"
        "   border: 1px solid #222222;"
        "   text-decoration: line-through;" // 已拾取的划线效果
        "}"
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

    // 就在 m_dropAnimation 初始化的下面：
    m_draftOverlay = new CardDraftOverlay(this);
    m_draftOverlay->move(0, 0); // 盖满全屏
    m_draftOverlay->hide();

    connect(m_draftOverlay, &CardDraftOverlay::cardSelected, this, [this](QString cardId) {
        m_draftOverlay->hide();

        // 🔴 玩家选完啦！重新把黑板请出来！
        m_boardWidget->show();

        GlobalSaveData::getInstance()->deckIds.append(cardId);
        if (m_pendingCardButton) {
            animateAndRemoveItem(m_pendingCardButton); // 发射幽灵卡牌！
            m_pendingCardButton = nullptr;
        }
    });

    // 🟢 修正后的返回信号连接
    connect(m_draftOverlay, &CardDraftOverlay::returnRequested, this, [this]() {
        m_draftOverlay->hide();
        m_boardWidget->show(); // 👈 仅仅把钉子户黑板重新请出来，按钮完好无损，允许玩家反复横跳！
        m_pendingCardButton = nullptr;
    });
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

void RewardScreen::loadRewards(const BattleResult& result) {
    QLayoutItem* item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 💰 1. 金币奖励
    if (result.rewardGold > 0) {
        // 删掉 emoji，前面留两个空格给图标腾点视觉空间
        RewardItemButton* goldBtn = new RewardItemButton(RewardItemButton::Gold, QString("  获得 %1 金币").arg(result.rewardGold));
        goldBtn->goldAmount = result.rewardGold;

        // 🔴 设置金币的真实图标！(请换成你自己的金币贴图路径喵)
        goldBtn->setIcon(QIcon(":/resources/images/ui/gold_icon.png"));

        connect(goldBtn, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(goldBtn);
    }

    // 🏺 2. 遗物奖励
    for (const QString& relicId : result.rewardRelicIds) {
        // ========================================================
        // 🕵️‍♀️ 户口盘查魔法：向工厂借一个临时克隆体，问完名字就销毁！
        // ========================================================
        QString relicName = relicId; // 默认用 ID 兜底
        Relic* tempRelic = RelicFactory::createRelic(relicId, nullptr);
        if (tempRelic) {
            relicName = tempRelic->getName(); // 拿到真正的中文名啦！(例如：异蛇之眼)
            delete tempRelic; // 问完当场销毁，深藏功与名！
        }

        RewardItemButton* relicBtn = new RewardItemButton(RewardItemButton::Relic, QString("  获得遗物：%1").arg(relicName));
        relicBtn->relicId = relicId;

        // 🔴 设置遗物的专属贴图！(完全复用之前的路径逻辑)
        QIcon relicIcon(QString(":/resources/images/relics/%1.png").arg(relicId));
        relicBtn->setIcon(relicIcon);

        connect(relicBtn, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(relicBtn);
    }

    // 🃏 3. 卡牌奖励
    if (result.hasCardReward) {
        RewardItemButton* cardBtn = new RewardItemButton(RewardItemButton::Card, "  增加一张卡牌到你的牌组");

        // 🔴 设置卡牌的真实图标！(可以用一张卡背图，或者精美的卡组图标)
        cardBtn->setIcon(QIcon(":/resources/images/ui/card_reward_icon.png"));

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
        targetPos = QPoint(1200, 24);
    }
    else if (btn->getType() == RewardItemButton::Relic) {
        // 🔴 极其聪明的位置预判：
        // 因为你在 onRewardItemClicked 里已经把遗物 append 进 save 了，
        // 所以现在 size() - 1 就是它在 RelicTray 里的绝对索引位！
        int currentRelicIndex = save->relicIds.size();
        if (currentRelicIndex < 0) currentRelicIndex = 0;

        int trayStartX = 10;
        int trayStartY = 55;
        int spacing = 8;

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
        } else if (type == RewardItemButton::Card) {
            // 🔴【新增】：卡牌幽灵飞到总牌库了！大喊一声！
            emit deckUpdated();
        }
    });

    flyAnim->start();
}

void RewardScreen::onRewardItemClicked() {
    RewardItemButton* clickedBtn = qobject_cast<RewardItemButton*>(sender());
    if (!clickedBtn) return;

    if (clickedBtn->getType() == RewardItemButton::Gold ||
        clickedBtn->getType() == RewardItemButton::Relic) {
        // 金币和遗物：不需要选，直接销毁肉体，播放飞行特效！
        animateAndRemoveItem(clickedBtn);
    }
    else if (clickedBtn->getType() == RewardItemButton::Card) {
        // 🃏 卡牌：先别急着销毁！留着案发现场！
        qDebug() << "[RewardScreen] 正在生成三选一盲盒...";
        m_pendingCardButton = clickedBtn; // 记下是哪个按钮触发的
        m_boardWidget->hide();

        // 呼叫工厂发牌算法！
        QList<QString> draftIds = CardFactory::generateCardRewardIds(3);

        // 展开三选一结界！
        m_draftOverlay->showDraft(draftIds);
    }
}

// ==========================================
// 🚪 离场通道：点击“继续”按钮
// ==========================================
void RewardScreen::onProceedClicked() {
    qDebug() << "[RewardScreen] 玩家点击了继续，准备向司令部请求放行！";
    // 发射极其关键的信号，通知 GameWindow 降下黑幕、切回大地图！
    emit proceedRequested();
}