#include "RewardScreen.h"
#include "GameWindow.h"
#include "../logic/GlobalSaveData.h"
#include <QDebug>
#include "logic/cardfactory.h"
#include "logic/RelicFactory.h" // 🔴 极其关键：引入你的遗物工厂，用来查户口！
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <cmath>
#include <QRandomGenerator>

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
    this->resize(parent ? parent->width() : 1600, parent ? parent->height() : 900);
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
        // 🌟 第一份：常规的卡牌奖励（所有人都有）
        RewardItemButton* cardBtn1 = new RewardItemButton(RewardItemButton::Card, "  增加一张卡牌到你的牌组");
        cardBtn1->setIcon(QIcon(":/resources/images/ui/card_reward_icon.png"));
        connect(cardBtn1, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
        m_listLayout->addWidget(cardBtn1);

        // ========================================================
        // 📿 念珠手镯特判：发第二份卡牌奖励！
        // ========================================================
        GlobalSaveData* save = GlobalSaveData::getInstance();

        // 查户口：看看全局存档里有没有念珠手镯
        if (save->relicIds.contains("relic_prayer_wheel")) {

            // 💡 严谨原版设定：如果是精英怪或 Boss，其实是不触发念珠手镯的。
            // 如果你的 BattleResult 里存了打的是什么怪 (比如 result.nodeType == NodeType::Monster)
            // 你可以把条件改成: if (result.nodeType == NodeType::Monster && save->relicIds.contains("relic_prayer_wheel"))

            qDebug() << "[RewardScreen] 📿 念珠手镯发威，追加第二份卡牌奖励！";

            RewardItemButton* cardBtn2 = new RewardItemButton(RewardItemButton::Card, "  增加一张卡牌到你的牌组");
            // 故意在文字上做点微小区分（可选），或者保持一模一样
            // cardBtn2->setText("  增加一张卡牌到你的牌组 (念珠手镯)");
            cardBtn2->setIcon(QIcon(":/resources/images/ui/card_reward_icon.png"));
            connect(cardBtn2, &QPushButton::clicked, this, &RewardScreen::onRewardItemClicked);
            m_listLayout->addWidget(cardBtn2);
        }
    }
}

void RewardScreen::animateAndRemoveItem(RewardItemButton* btn) {
    // 【极其关键】：在 btn 被 delete 之前，赶紧把它的数据复印保存下来！
    RewardItemButton::RewardType type = btn->getType();
    QString rId = btn->relicId;
    int gAmount = btn->goldAmount;

    // 拿到被点击按钮在全局窗口中的中心坐标，作为流星起飞点
    QPoint startCenter = btn->mapTo(this, btn->rect().center());

    btn->hide();
    m_listLayout->removeWidget(btn);
    btn->deleteLater();

    // ==========================================
    // 🎯 1. 精确计算入局终点坐标
    // ==========================================
    QPoint targetPos;
    GlobalSaveData* save = GlobalSaveData::getInstance();

    GameWindow* gw = qobject_cast<GameWindow*>(this->parentWidget());
    if (type == RewardItemButton::Gold) {
        // 🔴 金币飞行终点：跟随顶栏金币图标动态位置
        targetPos = gw ? gw->goldIconGlobalPos().toPoint() : QPoint(1200, 24);
    }
    else if (type == RewardItemButton::Relic) {
        // 🔴 极其聪明的位置预判：计算它将要落入 RelicTray 的哪个槽位
        int currentRelicIndex = save->relicIds.size();
        if (currentRelicIndex < 0) currentRelicIndex = 0;
        save->relicIds.append(rId);
        int trayStartX = 10;
        int trayStartY = 55;
        int spacing = 8;
        // 加上半径偏移，让流星中心砸准图标中心
        targetPos = QPoint(trayStartX + currentRelicIndex * (48 + spacing) + 24, trayStartY + 24);
    }
    else {
        // 🔴 卡牌飞行终点：跟随牌堆图标动态位置
        targetPos = gw ? gw->deckPileGlobalPos().toPoint() : QPoint(1370, 56);
    }

    // ==========================================
    // 🌌 2. 创建临时特效结界 (QGraphicsView)
    // ==========================================
    // 为了播放粒子特效，我们在 RewardScreen 顶层盖一层全透明的画布
    QGraphicsView* fxView = new QGraphicsView(this);
    fxView->resize(this->size());
    fxView->setStyleSheet("background: transparent; border: none;");
    fxView->setAttribute(Qt::WA_TransparentForMouseEvents); // 绝对不能挡住玩家鼠标
    fxView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fxView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* fxScene = new QGraphicsScene(0, 0, this->width(), this->height(), fxView);
    fxView->setScene(fxScene);
    fxView->show();
    fxView->raise();

    // ==========================================
    // ☄️ 3. 释放流星与星尘拖尾 (移植商店顶级特效)
    // ==========================================
    QPointF startPos(startCenter);
    QPointF endPos(targetPos);

    // 贝塞尔向上抛物线控制点
    QPointF ctrlPos(startPos.x() + (endPos.x() - startPos.x()) * 0.4, startPos.y() - 300);

    const int totalSteps = 50;
    const int interval = 16;

    // 高燃核心光球 (根据类型换颜色)
    auto* glowOrb = new QGraphicsEllipseItem(-25, -25, 50, 50);
    QRadialGradient gradient(0, 0, 25);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));

    // 🔴 颜色区分魔法：金币是金黄，遗物是幽蓝，卡牌是紫红！
    if (type == RewardItemButton::Gold) {
        gradient.setColorAt(0.3, QColor(255, 220, 50, 255));
        gradient.setColorAt(1.0, QColor(255, 100, 0, 0));
    } else if (type == RewardItemButton::Relic) {
        gradient.setColorAt(0.3, QColor(50, 200, 255, 255));
        gradient.setColorAt(1.0, QColor(0, 100, 255, 0));
    } else {
        gradient.setColorAt(0.3, QColor(200, 50, 255, 255));
        gradient.setColorAt(1.0, QColor(100, 0, 255, 0));
    }

    glowOrb->setBrush(gradient);
    glowOrb->setPen(Qt::NoPen);
    glowOrb->setPos(startPos);

    auto* blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(10);
    glowOrb->setGraphicsEffect(blur);
    fxScene->addItem(glowOrb);

    struct TrailParticle { QGraphicsEllipseItem* dot; qreal dx, dy; int age; int life; };
    auto* trails = new QList<TrailParticle>();

    int* pStep = new int(0);
    auto* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [=]() {
        (*pStep)++;
        int s = *pStep;
        qreal t = qreal(s) / totalSteps;

        // 🚀 贝塞尔曲线运动
        qreal easedT = t * t * (3 - 2 * t);
        qreal u = 1.0 - easedT;
        QPointF currentPos = u * u * startPos + 2 * u * easedT * ctrlPos + easedT * easedT * endPos;
        glowOrb->setPos(currentPos);
        glowOrb->setScale(1.0 + sin(t * 3.14159) * 0.3);

        // ✨ 喷射星尘
        for(int i = 0; i < 2; i++) {
            auto* dot = new QGraphicsEllipseItem(-4, -4, 8, 8);
            if (type == RewardItemButton::Gold) dot->setBrush(QColor(255, 200, 50, 200));
            else if (type == RewardItemButton::Relic) dot->setBrush(QColor(100, 220, 255, 200));
            else dot->setBrush(QColor(220, 100, 255, 200));

            dot->setPen(Qt::NoPen);
            dot->setPos(currentPos + QPointF((QRandomGenerator::global()->generateDouble()-0.5)*20, (QRandomGenerator::global()->generateDouble()-0.5)*20));
            fxScene->addItem(dot);

            qreal dx = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            qreal dy = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            trails->append({dot, dx, dy, 0, 10 + QRandomGenerator::global()->bounded(8)});
        }

        // 💨 拖尾消散
        for (int i = trails->size() - 1; i >= 0; --i) {
            auto& tr = (*trails)[i];
            tr.age++;
            tr.dot->moveBy(tr.dx, tr.dy);
            qreal lifeRatio = qreal(tr.age) / tr.life;
            tr.dot->setOpacity(1.0 - lifeRatio);
            tr.dot->setScale(1.0 - lifeRatio * 0.5);
            if (tr.age >= tr.life) {
                fxScene->removeItem(tr.dot);
                delete tr.dot;
                trails->removeAt(i);
            }
        }

        // 💥 4. 抵达终点：入账爆点与结算！
        if (s >= totalSteps) {
            timer->stop();

            // 爆开一圈火花
            for(int i = 0; i < 10; i++) {
                auto* spark = new QGraphicsEllipseItem(-3, -3, 6, 6);
                spark->setBrush(Qt::white);
                spark->setPen(Qt::NoPen);
                spark->setPos(endPos);
                fxScene->addItem(spark);

                qreal angle = i * (3.14159 * 2 / 10.0);
                qreal speed = 5.0 + QRandomGenerator::global()->generateDouble() * 3.0;
                qreal vX = cos(angle) * speed;
                qreal vY = sin(angle) * speed;

                auto* sparkTimer = new QTimer(fxView);
                int* sparkAge = new int(0);
                connect(sparkTimer, &QTimer::timeout, fxView, [=]() {
                    (*sparkAge)++;
                    spark->moveBy(vX, vY);
                    spark->setOpacity(1.0 - (*sparkAge) / 12.0);
                    if(*sparkAge >= 12) {
                        sparkTimer->stop();
                        fxScene->removeItem(spark);
                        delete spark;
                        delete sparkAge;
                        sparkTimer->deleteLater();
                    }
                });
                sparkTimer->start(16);
            }

            // 彻底清扫流星内存
            for (auto& tr : *trails) {
                fxScene->removeItem(tr.dot);
                delete tr.dot;
            }
            delete trails;
            fxScene->removeItem(glowOrb);
            delete glowOrb;
            delete pStep;
            timer->deleteLater();

            // ========================================================
            // 📢 发送最终入账信号给司令部 (GameWindow)！
            // 司令部收到后，UI 就会在爆点的中心瞬间 Q 弹现身！
            // ========================================================
            if (type == RewardItemButton::Relic) {
                emit relicFlightFinished(rId);
            } else if (type == RewardItemButton::Gold) {
                emit goldFlightFinished(gAmount);
            } else if (type == RewardItemButton::Card) {
                emit deckUpdated();
            }

            // 延时 0.5 秒销毁临时画布结界，给爆破火花留出播放时间
            QTimer::singleShot(500, this, [fxView]() {
                fxView->hide();
                fxView->deleteLater();
            });
        }
    });

    timer->start(interval);
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