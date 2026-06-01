#include "ui/GameWindow.h"
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include "../logic/RelicFactory.h"
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "ui/TopBar.h"
#include "ui/CardBrowserOverlay.h"
#include "events/CampfireView.h"
#include "ui/events/MerchantView.h" // 🔴 引入商店系统图纸！
#include "ui/events/ChestView.h"
#include <QScrollArea> // 队友提供：滚动视口
#include <QScroller>   // 队友提供：触摸拖拽与物理惯性

GameWindow::GameWindow(QWidget *parent) : QWidget(parent), m_launcher(nullptr), m_currentBattleView(nullptr) {
    // 1. 设置游戏主窗口大小
    this->setFixedSize(1600, 900);
    this->setWindowTitle("Slay the Qt");

    // 2. 搬来电视机！
    m_stack = new QStackedWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // 消除白边
    layout->addWidget(m_stack);

    // ==========================================
    // 🎬 实例化开始界面，并塞进频道 0
    // ==========================================
    m_titleView = new TitleMenuView(this);
    m_stack->addWidget(m_titleView); // 📺 频道 0：开始界面

    // 3. 实例化大地图，并准备塞进频道 1
    m_mapManager = new MapManager();

    // ==========================================
    // 🗺️ 队友的魔法：用 QScroller 包装大地图，实现物理拖拽！
    // ==========================================
    QScrollArea* mapScroll = new QScrollArea();
    mapScroll->setWidget(m_mapManager);
    mapScroll->setAlignment(Qt::AlignCenter);
    mapScroll->setWidgetResizable(false);
    mapScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mapScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(mapScroll, QScroller::LeftMouseButtonGesture);

    QScroller *scroller = QScroller::scroller(mapScroll);
    QScrollerProperties props = scroller->scrollerProperties();
    props.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.6); // 增大减速阻力
    props.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.55);   // 限制最大甩动速度
    scroller->setScrollerProperties(props);

    m_stack->addWidget(mapScroll); // 📺 频道 1：大地图！

    // 默认显示频道 0（开始界面）
    m_stack->setCurrentWidget(m_titleView);

    // ==========================================
    // 🌟 全局悬浮图层 (包容 TopBar 和 你的完美 RelicTray)
    // ==========================================
    m_topBarView = new QGraphicsView(this);
    m_topBarView->setFixedSize(1600, 110);
    m_topBarView->move(0, 0);

    m_topBarView->setStyleSheet("background: transparent; border: none;");
    m_topBarView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_topBarView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_topBarView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QGraphicsScene* topScene = new QGraphicsScene(0, 0, 1600, 110, this);
    m_topBarView->setScene(topScene);

    // 1. 放入顶栏
    m_topBar = new TopBar();
    topScene->addItem(m_topBar);

    // 2. 放入遗物栏
    m_globalRelicTray = new RelicTray();
    m_globalRelicTray->setPos(10, 55);
    topScene->addItem(m_globalRelicTray);

    m_topBarView->raise();
    // 🔴 初始状态下隐藏顶栏，等开始游戏后再显示
    m_topBarView->hide();

    // ==========================================
    // 🎁 将战利品界面作为“全局幽灵悬浮层”！
    // ==========================================
    m_rewardScreen = new RewardScreen(this);
    m_rewardScreen->move(0, 0);
    m_rewardScreen->hide();

    m_rewardScreen->raise();
    m_topBarView->raise(); // 确保 TopBar 是全游戏绝对的最顶层！

    // ==========================================
    // 📡 绑定各大系统的通讯天线
    // ==========================================
    connect(m_rewardScreen, &RewardScreen::proceedRequested, this, &GameWindow::onRewardProceedRequested);
    connect(m_mapManager, &MapManager::nodeClicked, this, &GameWindow::onMapNodeClicked);
    // 🔴 监听开始界面的“开战信号”，执行启动游戏动画
    connect(m_titleView, &TitleMenuView::startGameRequested, this, &GameWindow::handleStartGameTransition);

    // ==========================================
    // ⬛ 初始化全局黑幕
    // ==========================================
    m_curtain = new QWidget(this);
    m_curtain->resize(1600, 900);
    m_curtain->setStyleSheet("background-color: #000000;");
    m_curtain->hide();

    m_curtainEffect = new QGraphicsOpacityEffect(m_curtain);
    m_curtainEffect->setOpacity(0.0);
    m_curtain->setGraphicsEffect(m_curtainEffect);

    m_fadeAnimation = new QPropertyAnimation(m_curtainEffect, "opacity", this);
    m_fadeAnimation->setDuration(350);
    m_curtain->raise();

    // =======================================================
    // 🎁 接收飞行抵达信号，更新底层 UI 与全局存档！
    // =======================================================
    connect(m_rewardScreen, &RewardScreen::relicFlightFinished, this, [this](QString relicId) {
        GlobalSaveData::getInstance()->relicIds.append(relicId);
        Relic* newRelic = RelicFactory::createRelic(relicId, this);
        m_globalRelicTray->onNewRelicAdded(newRelic);

        if (m_currentBattleView && m_currentBattleView->getEngine()) {
            m_currentBattleView->getEngine()->m_relicManager->addRelic(newRelic);
        }
    });

    connect(m_rewardScreen, &RewardScreen::goldFlightFinished, this, [this](int amount) {
        GlobalSaveData::getInstance()->gold += amount;
        m_topBar->updateGold(GlobalSaveData::getInstance()->gold);

        if (m_currentBattleView && m_currentBattleView->getEngine()->getPlayer()) {
            m_currentBattleView->getEngine()->getPlayer()->setGold(GlobalSaveData::getInstance()->gold);
        }
    });

    connect(m_rewardScreen, &RewardScreen::deckUpdated, this, [this]() {
        m_topBar->refreshDeckCount();
    });

    // ========================================================
    // 🌌 极其优雅的“呼吸式同步膨胀”！(牌库查看)
    // ========================================================
    connect(m_topBar, &TopBar::deckViewRequested, this, [this, topScene]() {
        m_topBarView->setFixedSize(1600, 900);
        topScene->setSceneRect(0, 0, 1600, 900);

        QList<QString> deckIds = GlobalSaveData::getInstance()->deckIds;
        QList<Card*> displayCards;
        for (const QString& id : deckIds) {
            displayCards.append(CardFactory::createCard(id));
        }

        CardBrowserOverlay* overlay = new CardBrowserOverlay(displayCards, "你的牌組", 1600, 900);
        overlay->setZValue(9999);
        topScene->addItem(overlay);

        connect(overlay, &CardBrowserOverlay::closed, this, [this, overlay, displayCards, topScene]() {
            overlay->hide();
            overlay->deleteLater();
            qDeleteAll(displayCards);

            m_topBarView->setFixedSize(1600, 110);
            topScene->setSceneRect(0, 0, 1600, 110);
        });
    });

    // ========================================================
    // 🛡️ 顶栏数据与遗物读档唤醒（改为统一管理）
    // ========================================================
    GlobalSaveData* save = GlobalSaveData::getInstance();
    if (m_topBar) {
        m_topBar->updatePlayerName(QStringLiteral("铁甲战士"));
        m_topBar->updateHp(save->currentHp, save->maxHp);
        m_topBar->updateGold(save->gold);
        m_topBar->refreshDeckCount();
    }

    // 🔴 1. 填充全局唯一的遗物指针列表
    for (const QString& relicId : save->relicIds) {
        Relic* loadedRelic = RelicFactory::createRelic(relicId, this);
        if (loadedRelic) {
            if (save->relicCounters.contains(relicId)) {
                loadedRelic->setCounter(save->relicCounters[relicId]);
            }
            m_globalRelics.append(loadedRelic);  // 👈 加入全局列表
        }
    }

    // 🔴 2. 一次性喂给 RelicTray（它会自动创建 UI）
    if (m_globalRelicTray) {
        m_globalRelicTray->setRelics(m_globalRelics);
    }

}

// ==========================================
// 🎬 新增：处理“开始游戏”的黑场转场动画
// ==========================================
void GameWindow::handleStartGameTransition() {
    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        // 🔴 切换到频道 1（大地图），并唤醒顶栏
        m_stack->setCurrentIndex(1);
        m_topBarView->show();
        m_topBarView->raise();

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

void GameWindow::onMapNodeClicked(const MapNode& node) {
    m_lastClickedNode = node;

    // 🔴 使用枚舉進行極速且安全的判斷
    if (node.type == NodeType::Monster || node.type == NodeType::Elite || node.type == NodeType::Boss) {
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

            for (const QString& id : save->deckIds) context.currentDeck.append(CardFactory::createCard(id, nullptr));
            context.relics = m_globalRelics; // 直接复用全局遗物指针

            context.nodeType = node.type;
            context.currentLayer = node.layer;

            m_launcher = new BattleLauncher(this);
            connect(m_launcher, &BattleLauncher::battleConcluded, this, &GameWindow::onBattleConcluded);

            m_currentBattleView = m_launcher->launch(context);
            m_stack->addWidget(m_currentBattleView);
            m_stack->setCurrentWidget(m_currentBattleView);

            if (m_currentBattleView && m_currentBattleView->getEngine()) {
                Player* player = m_currentBattleView->getEngine()->getPlayer();
                if (player) {
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
    }
    else if (node.type == NodeType::Campfire) {
        enterCampfireEvent();
    }
    // 🔴 乾淨俐落的商店跳轉
    else if (node.type == NodeType::Shop) {
        enterMerchantEvent();
    }else if (node.type == NodeType::Chest) {
        enterChestEvent();
    }
}

void GameWindow::onBattleConcluded(BattleResult result) {
    if (!result.isVictory) return;

    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->currentHp = result.currentHp;
    save->maxHp = result.maxHp;
    save->maxEnergy = result.maxEnergy;

    m_rewardScreen->loadRewards(result);
    m_rewardScreen->dropDown();
}

void GameWindow::onRewardProceedRequested() {
    m_rewardScreen->hide();

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        m_mapManager->m_currentLayer = m_lastClickedNode.layer;
        m_mapManager->m_currentNodeId = m_lastClickedNode.id;
        m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
        m_mapManager->refreshNodeStates();

        m_stack->setCurrentIndex(1); // 🔴 切回大地图 (频道1)

        m_stack->removeWidget(m_currentBattleView);
        m_currentBattleView->deleteLater();
        m_currentBattleView = nullptr;
        m_launcher = nullptr;

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

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        CampfireView* campfireView = new CampfireView(nullptr, nullptr, nullptr, this);
        campfireView->setGeometry(-5, -5, 1610, 910);

        campfireView->raise();
        m_topBarView->raise();
        m_curtain->raise();

        campfireView->show();

        connect(campfireView, &CampfireView::playerStatusChanged, this, [this]() {
            GlobalSaveData* save = GlobalSaveData::getInstance();
            m_topBar->updateHp(save->currentHp, save->maxHp);
        });

        connect(campfireView, &CampfireView::deckUpdated, this, [this]() {
            m_topBar->refreshDeckCount();
        });

        connect(campfireView, &EventBaseView::eventFinished, this, [this, campfireView]() {
            qDebug() << "[GameWindow] 🔥 玩家休息完毕，准备降下黑幕返回地图！";

            m_curtain->raise();
            m_curtain->show();
            m_fadeAnimation->stop();
            m_fadeAnimation->setStartValue(0.0);
            m_fadeAnimation->setEndValue(1.0);
            m_fadeAnimation->disconnect();

            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, campfireView]() {
                campfireView->hide();
                campfireView->deleteLater();

                m_mapManager->m_currentLayer = m_lastClickedNode.layer;
                m_mapManager->m_currentNodeId = m_lastClickedNode.id;
                m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
                m_mapManager->refreshNodeStates();

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

void GameWindow::enterMerchantEvent() {
    qDebug() << "[GameWindow] 发现商人！准备降下黑幕进入商店...";

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // ========================================================
        // 💰 幕后布置：生成商店场景
        // ========================================================
        MerchantView* merchantView = new MerchantView(nullptr, nullptr, nullptr, this);
        merchantView->setGeometry(-5, -5, 1610, 910);

        merchantView->raise();
        m_topBarView->raise(); // 🔴 极其关键：顶栏永远压在最上面，实时显示扣款！
        m_curtain->raise();

        merchantView->show();

        // 📡 通讯天线：监听商店里的消费和删牌动作，让顶栏实时跳字！
        connect(merchantView, &MerchantView::shopDataChanged, this, [this]() {
            GlobalSaveData* save = GlobalSaveData::getInstance();
            m_topBar->updateGold(save->gold);  // 刷新金币
            m_topBar->refreshDeckCount();      // 刷新卡组数量
        });

        // 🎬 退场逻辑：拦截商人界面的离开信号，再次降下黑幕！
        connect(merchantView, &EventBaseView::eventFinished, this, [this, merchantView]() {
            qDebug() << "[GameWindow] 🛍️ 购物结束，准备降下黑幕返回地图！";

            m_curtain->raise();
            m_curtain->show();
            m_fadeAnimation->stop();
            m_fadeAnimation->setStartValue(0.0);
            m_fadeAnimation->setEndValue(1.0);
            m_fadeAnimation->disconnect();

            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, merchantView]() {
                // 销毁商店视图
                merchantView->hide();
                merchantView->deleteLater();

                // 推进大地图进度
                m_mapManager->m_currentLayer = m_lastClickedNode.layer;
                m_mapManager->m_currentNodeId = m_lastClickedNode.id;
                m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
                m_mapManager->refreshNodeStates();

                // 重新揭开幕布，重见大地图
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

        connect(merchantView, &MerchantView::relicBought, this, [this](Relic* relic) {
            relic->setParent(this);  // 或者 relic->setParent(nullptr); 再自己管理
            m_globalRelics.append(relic);

            // 2. 刷新顶栏遗物盘
            m_globalRelicTray->setRelics(m_globalRelics);
        });

        // 🌟 入场收尾：场景搭建完毕，拉开幕布，正式营业！
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

void GameWindow::enterChestEvent() {
    qDebug() << "[GameWindow] 发现宝箱！准备降下黑幕进入宝箱房间...";

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // ========================================================
        // 🎁 幕后布置：生成宝箱场景
        // ========================================================
        // 注意：如果你 ChestView 的构造函数需要 player 和 relicManager，请填入对应的指针
        ChestView* chestView = new ChestView(nullptr, nullptr, this);
        chestView->setGeometry(-5, -5, 1610, 910);

        chestView->raise();
        m_topBarView->raise(); // 🔴 极其关键：顶栏永远压在最上面，准备迎接流星入账！
        m_curtain->raise();

        chestView->show();

        // 📡 通讯天线：监听宝箱里获取遗物的动作，存入大名单并刷新顶栏！
        connect(chestView, &ChestView::relicObtained, this, [this](Relic* relic) {
            relic->setParent(this);  // 移交内存管理权给 GameWindow
            m_globalRelics.append(relic);

            // 🌟 刷新顶栏遗物盘，保持和你商人事件一模一样的逻辑！
            // (如果你的 Tray 有直接添加单件的方法，也可以写 m_globalRelicTray->onNewRelicAdded(relic);)
            m_globalRelicTray->setRelics(m_globalRelics);
        });

        // 🎬 退场逻辑：拦截宝箱界面的离开信号，再次降下黑幕！
        connect(chestView, &EventBaseView::eventFinished, this, [this, chestView]() {
            qDebug() << "[GameWindow] 🎁 宝箱事件结束，准备降下黑幕返回地图！";

            m_curtain->raise();
            m_curtain->show();
            m_fadeAnimation->stop();
            m_fadeAnimation->setStartValue(0.0);
            m_fadeAnimation->setEndValue(1.0);
            m_fadeAnimation->disconnect();

            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, chestView]() {
                // 销毁宝箱视图
                chestView->hide();
                chestView->deleteLater();

                // 推进大地图进度 (完美复用你写好的地图逻辑)
                m_mapManager->m_currentLayer = m_lastClickedNode.layer;
                m_mapManager->m_currentNodeId = m_lastClickedNode.id;
                m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
                m_mapManager->refreshNodeStates();

                // 重新揭开幕布，重见大地图
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

        // 🌟 入场收尾：场景搭建完毕，拉开幕布，宝箱出现在眼前！
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