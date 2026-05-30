#include "ui/GameWindow.h"
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include "../logic/RelicFactory.h"
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "ui/TopBar.h"
#include "ui/CardBrowserOverlay.h"
#include "events/CampfireView.h" // 🔴 确保引入了火堆界面！
// (确保顶部引入了 TopBar.h)

GameWindow::GameWindow(QWidget *parent) : QWidget(parent), m_launcher(nullptr), m_currentBattleView(nullptr) {
    // 1. 设置游戏主窗口大小
    this->setFixedSize(1600, 900);
    this->setWindowTitle("Slay the Qt");

    // 2. 搬来电视机！
    m_stack = new QStackedWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // 消除白边
    layout->addWidget(m_stack);

    // 3. 实例化大地图，并塞进频道 0
    m_mapManager = new MapManager();
    QScrollArea* mapScroll = new QScrollArea();
    mapScroll->setWidget(m_mapManager);
    mapScroll->setAlignment(Qt::AlignCenter);
    m_stack->addWidget(mapScroll); // 📺 频道 0：大地图！

    // ==========================================
    // 🌟 全局悬浮图层 (包容 TopBar 和 你的完美 RelicTray)
    // ==========================================
    m_topBarView = new QGraphicsView(this);
    // 🔴 极简修正 1：高度变成 150，完美装下 Y=70 的遗物栏！
    m_topBarView->setFixedSize(1600, 110);
    m_topBarView->move(0, 0);

    m_topBarView->setStyleSheet("background: transparent; border: none;");
    m_topBarView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_topBarView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_topBarView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 🔴 极简修正 2：舞台高度也变成 150
    QGraphicsScene* topScene = new QGraphicsScene(0, 0, 1600, 110, this);
    m_topBarView->setScene(topScene);

    // 1. 放入顶栏
    m_topBar = new TopBar();
    topScene->addItem(m_topBar);

    // ========================================================
    // 🔴 极简修正 3：原封不动地使用你写的完美遗物栏！
    // ========================================================
    m_globalRelicTray = new RelicTray();
    m_globalRelicTray->setPos(10, 55); // 👈 完美对应你的飞行坐标！一像素都不差！
    topScene->addItem(m_globalRelicTray);

    m_topBarView->raise();

    // ==========================================
    // 🎁 将战利品界面作为“全局幽灵悬浮层”！
    // ==========================================
    m_rewardScreen = new RewardScreen(this);
    m_rewardScreen->move(0, 0);
    m_rewardScreen->hide();

    // 🔴 极其关键的操作：战利品界面也需要盖在电视机上面，
    // 但是为了能看到顶栏的牌库跳动，我们让 TopBar 盖在战利品之上！
    m_rewardScreen->raise();
    m_topBarView->raise(); // 再次提权，确保 TopBar 是全游戏绝对的最顶层！

    connect(m_rewardScreen, &RewardScreen::proceedRequested, this, &GameWindow::onRewardProceedRequested);
    connect(m_mapManager, &MapManager::nodeClicked, this, &GameWindow::onMapNodeClicked);

    // 幕布逻辑保持不变
    m_curtain = new QWidget(this);
    m_curtain->resize(1600, 900);
    m_curtain->setStyleSheet("background-color: #000000;");
    m_curtain->hide();

    m_curtainEffect = new QGraphicsOpacityEffect(m_curtain);
    m_curtainEffect->setOpacity(0.0);
    m_curtain->setGraphicsEffect(m_curtainEffect);

    m_fadeAnimation = new QPropertyAnimation(m_curtainEffect, "opacity", this);
    m_fadeAnimation->setDuration(350);
    m_curtain->raise(); // 幕布切屏时盖住一切，包括 TopBar

    // =======================================================
    // 🎁 接收飞行抵达信号，更新底层 UI 与全局存档！
    // =======================================================
    connect(m_rewardScreen, &RewardScreen::relicFlightFinished, this, [this](QString relicId) {
        GlobalSaveData::getInstance()->relicIds.append(relicId);

        // ========================================================
        // 🔴 极简修正 5：直接利用你写好的方法生成 UI，完全不需要重写逻辑！
        // ========================================================
        Relic* newRelic = RelicFactory::createRelic(relicId, this);
        m_globalRelicTray->onNewRelicAdded(newRelic); // 👈 就这一句，完美搞定！

        // 顺便给底层的战斗引擎塞一份肉身
        if (m_currentBattleView && m_currentBattleView->getEngine()) {
            m_currentBattleView->getEngine()->m_relicManager->addRelic(newRelic);
        }
    });

    connect(m_rewardScreen, &RewardScreen::goldFlightFinished, this, [this](int amount) {
        GlobalSaveData::getInstance()->gold += amount;
        qDebug() << "[GameWindow] 💰 金币飞行抵达！当前余额：" << GlobalSaveData::getInstance()->gold;

        // 🔴 修正：现在我们有全局的 TopBar 了！直接更新它！
        m_topBar->updateGold(GlobalSaveData::getInstance()->gold);

        // (如果你底层还存了 Player，也可以同步更新 player，但 TopBar 才是显示的本体)
        if (m_currentBattleView && m_currentBattleView->getEngine()->getPlayer()) {
            m_currentBattleView->getEngine()->getPlayer()->setGold(GlobalSaveData::getInstance()->gold);
        }
    });

    // ========================================================
    // 🌌 极其优雅的“呼吸式同步膨胀”！
    // ========================================================
    connect(m_topBar, &TopBar::deckViewRequested, this, [this, topScene]() {
        qDebug() << "[GameWindow] 收到看牌请求！瞬间膨胀顶栏图层与舞台！";

        // 1. 🔴 突破监禁：视口和舞台同步拉大到 900！
        m_topBarView->setFixedSize(1600, 900);
        topScene->setSceneRect(0, 0, 1600, 900); // 👈 极其关键！命令底层坐标系膨胀！

        // 2. 查阅档案，临时印制卡牌
        QList<QString> deckIds = GlobalSaveData::getInstance()->deckIds;
        QList<Card*> displayCards;
        for (const QString& id : deckIds) {
            displayCards.append(CardFactory::createCard(id));
        }

        // 3. 召唤极品 UI 遮罩
        CardBrowserOverlay* overlay = new CardBrowserOverlay(displayCards, "你的牌組", 1600, 900);
        overlay->setZValue(9999);
        topScene->addItem(overlay);

        // 4. 🗑️ 关闭时的完美善后
        // 🔴 注意：这里要把 topScene 传进 lambda 表达式的捕获列表里喵！
        connect(overlay, &CardBrowserOverlay::closed, this, [this, overlay, displayCards, topScene]() {
            overlay->hide();
            overlay->deleteLater();
            qDeleteAll(displayCards);

            // 🔴 极简修正 4：缩回时变成 150 像素，而不是 60！
            m_topBarView->setFixedSize(1600, 110);
            topScene->setSceneRect(0, 0, 1600, 110);

            qDebug() << "[GameWindow] 牌库结界关闭，图层与舞台已安全缩回喵！";
        });
    });

    // ========================================================
    // 🛡️ 顶栏数据唤醒（你之前已经加好的部分）
    // ========================================================
    GlobalSaveData* save = GlobalSaveData::getInstance();
    if (m_topBar) {
        m_topBar->updatePlayerName(QStringLiteral("铁甲战士"));
        m_topBar->updateHp(save->currentHp, save->maxHp);
        m_topBar->updateGold(save->gold);
        m_topBar->refreshDeckCount();
    }

    // ========================================================
    // 🏺 极简老实的遗物读档循环（直接追加在这里！）
    // ========================================================
    if (m_globalRelicTray) {
        for (const QString& relicId : save->relicIds) {

            // 1. 老老实实地呼叫工厂，把逻辑肉身捏出来
            Relic* loadedRelic = RelicFactory::createRelic(relicId, this);

            if (loadedRelic) {
                // 2. 顺手恢复一下它的前世记忆（比如钢笔尖的层数）
                if (save->relicCounters.contains(relicId)) {
                    loadedRelic->setCounter(save->relicCounters[relicId]);
                }

                // 3. 完美调用你写的 UI 生成方法！没有废话！
                m_globalRelicTray->onNewRelicAdded(loadedRelic);
            }
        }
        qDebug() << "[GameWindow] 开局档案读取完毕！共恢复了" << save->relicIds.size() << "个遗物喵！";
    }
    connect(m_rewardScreen, &RewardScreen::deckUpdated, this, [this]() {
        // 🔴 窃听器 2 号：
        qDebug() << "🕵️‍♂️ [线索 2] 司令部接到了卡牌归巢信号！立刻呼叫 TopBar！";
        m_topBar->refreshDeckCount();
    });
}

void GameWindow::onMapNodeClicked(const MapNode& node) {
    m_lastClickedNode = node;
    if (node.type == "Monster" || node.type == "Elite" || node.type == "Boss") {
        m_curtain->raise();
        m_curtain->show();
        m_fadeAnimation->stop();
        m_fadeAnimation->setStartValue(0.0);
        m_fadeAnimation->setEndValue(1.0);
        m_fadeAnimation->disconnect();

        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, node]() {
            BattleContext context;
            GlobalSaveData* save = GlobalSaveData::getInstance();
            context.currentHp = save->currentHp;
            context.maxHp = save->maxHp;
            context.gold = save->gold;
            context.maxEnergy = save->maxEnergy;

            // 疯狂印卡、造遗物...
            for (const QString& id : save->deckIds) context.currentDeck.append(CardFactory::createCard(id, nullptr));
            for (const QString& id : save->relicIds) context.relics.append(RelicFactory::createRelic(id, nullptr));

            // =======================================================
            // 🔴 核心解耦：抹除硬编码！我们只把“标签”塞进密令里发往战斗引擎！
            // =======================================================
            context.nodeType = node.type;       // "Monster", "Elite", "Boss" 等
            context.currentLayer = node.layer;  // 0 ~ 14 层

            m_launcher = new BattleLauncher(this);
            connect(m_launcher, &BattleLauncher::battleConcluded, this, &GameWindow::onBattleConcluded);

            m_currentBattleView = m_launcher->launch(context);
            m_stack->addWidget(m_currentBattleView);
            m_stack->setCurrentWidget(m_currentBattleView);

        // ========================================================
        // 🔴 3. 极其关键的生命线：让全局顶栏认主！
        // ========================================================
            if (m_currentBattleView && m_currentBattleView->getEngine()) {
                Player* player = m_currentBattleView->getEngine()->getPlayer();
                if (player) {
                    // 让全局的 TopBar 绑定刚刚在战斗沙盒里生成的玩家！
                    m_topBar->bindPlayer(player);
                }
            }

        m_fadeAnimation->disconnect();
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);
            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_curtain->hide();
           });
           m_fadeAnimation->start();
        });

        m_fadeAnimation->start();
    }else if (node.type == "Campfire") {
        enterCampfireEvent(); // 调用我们专门写的新函数！
    }
}

void GameWindow::onBattleConcluded(BattleResult result) {
    if (!result.isVictory) return;

    // 🔴 极其丝滑：不要降下全屏黑幕，直接召唤战利品悬浮层！
    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->currentHp = result.currentHp;
    save->maxHp = result.maxHp;
    save->maxEnergy = result.maxEnergy;

    m_rewardScreen->loadRewards(result);
    m_rewardScreen->dropDown(); // 💥 砰！伴随着抖动砸下！
}

// 3. 玩家点完继续，打扫战场并回大地图！
void GameWindow::onRewardProceedRequested() {
    // 玩家拿完东西了，现在才需要降下全屏黑幕进行“场景切换”！
    m_rewardScreen->hide(); // 隐藏悬浮层

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        // 更新大地图标记
        m_mapManager->m_currentLayer = m_lastClickedNode.layer;
        m_mapManager->m_currentNodeId = m_lastClickedNode.id;
        m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
        m_mapManager->refreshNodeStates();

        // 📺 切回大地图
        m_stack->setCurrentIndex(0);

        // 🗑️ 此时才真正销毁战斗场景（尸体清理完毕）！
        m_stack->removeWidget(m_currentBattleView);
        m_currentBattleView->deleteLater();
        m_currentBattleView = nullptr;
        m_launcher = nullptr;

        // 拉起黑幕，回到大地图
        m_fadeAnimation->disconnect();
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);
        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_curtain->hide();
        });
        m_fadeAnimation->start();
    });

    m_fadeAnimation->start();
}

void GameWindow::enterCampfireEvent() {
    qDebug() << "[GameWindow] 拦截成功！开始安营扎寨！准备降下黑幕...";

    // ========================================================
    // 🎬 1. 入场：降下全局黑幕
    // ========================================================
    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect(); // 极其良好的习惯：清空上一次的槽函数

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // ========================================================
        // ⛺ 2. 幕后布置：黑屏遮挡完毕，开始在后方搭建火堆场景
        // ========================================================
        CampfireView* campfireView = new CampfireView(nullptr, nullptr, nullptr, this);

        // 恢复 16:9 黄金比例，彻底消灭黑边
        campfireView->setGeometry(-5, -5, 1610, 910);

        // 🔴 极其关键的层级控制：先放火堆，再压顶栏，最后把黑幕提回最上层！
        campfireView->raise();
        m_topBarView->raise();
        m_curtain->raise();

        campfireView->show();

        // 接入通讯天线
        connect(campfireView, &CampfireView::playerStatusChanged, this, [this]() {
            GlobalSaveData* save = GlobalSaveData::getInstance();
            m_topBar->updateHp(save->currentHp, save->maxHp);
        });

        connect(campfireView, &CampfireView::deckUpdated, this, [this]() {
            m_topBar->refreshDeckCount();
        });

        // ========================================================
        // 🎬 3. 退场逻辑：拦截火堆的退出请求，再次降下黑幕！
        // ========================================================
        connect(campfireView, &EventBaseView::eventFinished, this, [this, campfireView]() {
            qDebug() << "[GameWindow] 🔥 玩家休息完毕，准备降下黑幕返回地图！";

            m_curtain->raise();
            m_curtain->show();
            m_fadeAnimation->stop();
            m_fadeAnimation->setStartValue(0.0);
            m_fadeAnimation->setEndValue(1.0);
            m_fadeAnimation->disconnect();

            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, campfireView]() {

                // 🗑️ 黑屏彻底遮挡后：销毁火堆视图
                campfireView->hide();
                campfireView->deleteLater();

                // 🗺️ 推进大地图进度
                m_mapManager->m_currentLayer = m_lastClickedNode.layer;
                m_mapManager->m_currentNodeId = m_lastClickedNode.id;
                m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
                m_mapManager->refreshNodeStates();

                // 🌟 重新揭开幕布，重见大地图
                m_fadeAnimation->disconnect();
                m_fadeAnimation->setStartValue(1.0);
                m_fadeAnimation->setEndValue(0.0);
                connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
                    m_curtain->hide();
                });
                m_fadeAnimation->start();
            });

            m_fadeAnimation->start();
        });

        // ========================================================
        // 🌟 4. 入场收尾：场景搭建完毕，拉开幕布，重见光明！
        // ========================================================
        m_fadeAnimation->disconnect();
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);
        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_curtain->hide();
        });
        m_fadeAnimation->start();

    });

    // 🚀 启动第一步的入场黑幕！
    m_fadeAnimation->start();
}