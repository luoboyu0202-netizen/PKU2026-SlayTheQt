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
#include "api/EventLauncher.h"
#include <QGraphicsBlurEffect>
#include <QRandomGenerator>
#include <cmath>


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
    // 🔴 【新增】：监听开始界面的“继续游戏”信号 (读档开局)
    connect(m_titleView, &TitleMenuView::continueGameRequested, this, &GameWindow::handleContinueGameTransition);

    // 🔴 【新增】：监听顶栏 TopBar 的“保存并退出”信号 (写盘并退回主菜单)
    connect(m_topBar, &TopBar::returnToTitleRequested, this, &GameWindow::handleReturnToTitle);
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
        // 🟢 1. 既然 RewardScreen 已经在起飞时把名字写进户口本了，我们直接呼叫万能刷新！
        refreshTopBarRelics();

        // 🟢 2. 如果当前还在战斗状态（比如精英怪战利品还没退回地图），
        // 顺手给战斗沙盒也塞个克隆体，保证它立刻生效
        if (m_currentBattleView && m_currentBattleView->getEngine()) {
            Relic* combatClone = RelicFactory::createRelic(relicId, this);
            m_currentBattleView->getEngine()->m_relicManager->addRelic(combatClone);
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

    // 🔴 呼叫万能大招！初始化顶栏遗物！
    refreshTopBarRelics();

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
        // 🔴 关键修复 1：强行重置大地图，抹除上一次玩的痕迹！
        m_mapManager->resetMap();

        // 🔴 关键修复 2：将顶栏的显示数据刷新为初始状态！
        GlobalSaveData* save = GlobalSaveData::getInstance();
        m_topBar->updateHp(save->currentHp, save->maxHp);
        m_topBar->updateGold(save->gold);
        m_topBar->refreshDeckCount();
        refreshTopBarRelics();

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
    }else if (node.type == NodeType::Event) { // 請根據你的枚舉名稱調整
        enterQuestionMarkEvent(node);
    }
}

void GameWindow::onBattleConcluded(BattleResult result) {
    if (!result.isVictory) {
        qDebug() << "[GameWindow] 💀 玩家阵亡！正在抹除存档并退回主界面...";

        // 抹除存档！
        GlobalSaveData::getInstance()->deleteSaveFile();

        // 降下黑幕
        m_curtain->raise();
        m_curtain->show();
        m_fadeAnimation->stop();
        m_fadeAnimation->setStartValue(0.0);
        m_fadeAnimation->setEndValue(1.0);
        m_fadeAnimation->disconnect();

        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            // 切回频道 0 (主界面)
            m_stack->setCurrentIndex(0);
            m_topBarView->hide();
            m_rewardScreen->hide();

            // 🔴 刷新主界面的“继续游戏”按钮（让它变灰！）
            m_titleView->refreshSaveState();

            // 销毁战斗尸体
            if (m_currentBattleView) {
                m_stack->removeWidget(m_currentBattleView);
                m_currentBattleView->deleteLater();
                m_currentBattleView = nullptr;
                m_launcher = nullptr;
            }

            // 拉开黑幕
            m_fadeAnimation->disconnect();
            m_fadeAnimation->setStartValue(1.0);
            m_fadeAnimation->setEndValue(0.0);
            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_curtain->hide();
            });
            m_fadeAnimation->start();
        });

        m_fadeAnimation->start();
        return; // 死了就不执行下面的胜利逻辑了
    }

    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->currentHp = result.currentHp;
    save->maxHp = result.maxHp;
    save->maxEnergy = result.maxEnergy;

    m_rewardScreen->loadRewards(result);
    m_rewardScreen->dropDown();
}

void GameWindow::onRewardProceedRequested() {
    m_rewardScreen->hide();

    // ========================================================
    // 🔴 结局大拦截：如果是打败了 Boss，直接切入“终末之诗”！
    // ========================================================
    if (m_lastClickedNode.type == NodeType::Boss) {
        playEndingAnimation();
        return; // 直接 return，不执行下方返回地图的代码！
    }

    // ========================================================
    // ⬇️ 常规怪物离场：降下黑幕、推进大地图进度！
    // ========================================================
    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // 🔴🔴🔴 极其关键：修复丢失的地图推进逻辑！必须把这四行补回来！
        m_mapManager->m_currentLayer = m_lastClickedNode.layer;
        m_mapManager->m_currentNodeId = m_lastClickedNode.id;
        m_mapManager->m_visitedNodes.append(m_lastClickedNode.id);
        m_mapManager->refreshNodeStates();
        // 🔴🔴🔴

        m_stack->setCurrentIndex(1); // 切回大地图

        if (m_currentBattleView) {
            m_stack->removeWidget(m_currentBattleView);
            m_currentBattleView->deleteLater();
            m_currentBattleView = nullptr;
            m_launcher = nullptr;
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
// ========================================================
// 🎬 史诗级结局：缓慢黑屏 + 终末之诗滚动字幕
// ========================================================
// ========================================================
// 🎬 史诗级结局：缓慢黑屏 + 终末之诗与制作人员名单
// ========================================================
void GameWindow::playEndingAnimation() {
    qDebug() << "[GameWindow] 👑 玩家击败了最终 Boss！开始播放终末之诗...";

    // 1. 缓慢黑屏：直接复用你写好的全局黑幕！
    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setDuration(3000); // 3秒缓慢变黑
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    // 等屏幕彻底、缓慢地黑透之后，再开始出字幕！
    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // 2. 准备终末之诗的文本标签 (长文本排版)
        QString creditsText =
            "探索尚未结束\n"
            "敬请期待...\n\n\n\n\n\n"
            "—— 制作团队 ——\n\n\n"
            "策划：\n"
            "apple\n\n\n"
            "战斗系统（遗物、卡牌...）：\n"
            "apple\n\n\n"
            "事件系统（火堆，商店...）：\n"
            "我心永属斯卡蒂\n\n\n"
            "地图、主界面：\n"
            "十一分之三\n\n\n"
            "灵感来源：\n"
            "Slay the Spire\n"
            "（杀戮尖塔）\n\n\n\n\n\n"
            "感谢您的游玩！";

        QLabel* textLabel = new QLabel(creditsText, m_curtain);

        // 字体大小调成32，居中对齐
        textLabel->setStyleSheet("color: white; font-size: 32px; font-family: 'FangSong'; font-weight: bold; background: transparent;");
        textLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        textLabel->resize(1600, 3000); // 🔴 留出巨大的高度容纳长文本
        textLabel->move(0, 900);       // 设置初始位置：沉在屏幕底部之外
        textLabel->show();

        // 3. 创建极度平滑的滚动动画
        QPropertyAnimation* scrollAnim = new QPropertyAnimation(textLabel, "pos", textLabel);
        scrollAnim->setDuration(20000); // 🔴 滚动时长设为 20 秒，慢慢放
        scrollAnim->setStartValue(QPoint(0, 900));
        scrollAnim->setEndValue(QPoint(0, -1500)); // 🔴 终点拉得更高，确保字完全滚出去

        // 4. 动画结束后的清场收尾逻辑
        connect(scrollAnim, &QPropertyAnimation::finished, this, [this, textLabel]() {
            // 文字滚完后，让黑屏再停顿 2 秒钟
            QTimer::singleShot(2000, this, [this, textLabel]() {

                // 💀 抹除当前存档
                GlobalSaveData::getInstance()->deleteSaveFile();

                // 🧹 清理战场遗留尸体
                if (m_currentBattleView) {
                    m_stack->removeWidget(m_currentBattleView);
                    m_currentBattleView->deleteLater();
                    m_currentBattleView = nullptr;
                    m_launcher = nullptr;
                }

                // 隐藏战斗顶栏，刷新开始界面
                m_topBarView->hide();
                m_titleView->refreshSaveState();
                m_stack->setCurrentIndex(0);

                // 🌅 5. 缓慢亮起，重见主菜单
                m_fadeAnimation->disconnect();
                m_fadeAnimation->setDuration(1500); // 1.5秒缓慢重现光明
                m_fadeAnimation->setStartValue(1.0);
                m_fadeAnimation->setEndValue(0.0);
                connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, textLabel]() {
                    m_curtain->hide();
                    textLabel->deleteLater(); // 烧毁字幕标签

                    // 🔴 极度重要：把转场速度重置回日常的 350 毫秒！
                    m_fadeAnimation->setDuration(350);
                });
                m_fadeAnimation->start();

            });
        });

        // 🎬 点火开机！(开始滚字幕)
        scrollAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // 🎬 启动缓慢黑屏！
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

void GameWindow::enterQuestionMarkEvent(const MapNode& node) {

    // ========================================================
    // 🎲 1. 頂層截胡：商店與寶箱的絕對機率
    // ========================================================
    int roll = QRandomGenerator::global()->bounded(100);
    if (roll < 3) {
        qDebug() << "🎲 驚喜！問號節點偽裝成了【商店】！";
        enterMerchantEvent();
        return;
    }
    else if (roll < 5) {
        qDebug() << "🎲 驚喜！問號節點偽裝成了【寶箱】！";
        enterChestEvent();
        return;
    }

    qDebug() << "[GameWindow] 踏入未知！准备降下黑幕...";
    GlobalSaveData* save = GlobalSaveData::getInstance();

    // 🔴 核心修复 1：创建一个专门的变量来存储最终决定的事件 ID
    QString chosenEventId;

    // ========================================================
    // ⚔️ 2. 動態怪物機率 (殺戮尖塔正宗 PRD)
    // ========================================================
    int monsterRoll = QRandomGenerator::global()->bounded(100);
    if (monsterRoll < save->questionMarkMonsterChance) {
        chosenEventId = "MonsterEncounter";
        save->questionMarkMonsterChance = 10; // 重置機率
        qDebug() << "🎲 運氣不好！觸發遭遇戰。下次怪物機率重置為 10%";
    }
    else {
        save->questionMarkMonsterChance += 10;

        // ========================================================
        // 📜 3. 故事事件的「抽獎袋 (Grab Bag)」機制
        // ========================================================
        if (save->availableEvents.isEmpty()) {
            qDebug() << "🎲 事件袋已空，重新進貨洗牌！";
            save->availableEvents = {
                "BigFish", "Cleric", "Designer", "SelfNote", "GoldenWing", "WorldOfGoop", "Ssssserpent"
            };
        }

        int eventIndex = QRandomGenerator::global()->bounded(save->availableEvents.size());
        chosenEventId = save->availableEvents[eventIndex];
        save->availableEvents.removeAt(eventIndex);

        qDebug() << "🎲 盲盒開啟！本次搖中：" << chosenEventId
                 << " | 剩餘未觸發事件數：" << save->availableEvents.size()
                 << " | 下次怪物機率升至：" << save->questionMarkMonsterChance << "%";
    }

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    // 🔴 核心修复 2：把 chosenEventId 塞进方括号里，让里面的闭包能使用它！
    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this, node, chosenEventId]() {
        // 构建输入合同 (Context)
        GlobalSaveData* save = GlobalSaveData::getInstance();
        EventContext ctx;
        ctx.currentHp = save->currentHp;
        ctx.maxHp = save->maxHp;
        ctx.gold = save->gold;
        ctx.maxEnergy = save->maxEnergy;
        ctx.eventType = EventType::QuestionMark;
        ctx.currentLayer = node.layer;

        // 🔴 核心修复 3：直接使用外面精心算好的结果！删掉下面那堆没用的随机池！
        ctx.eventSubtype = chosenEventId;

        for (const QString& id : save->deckIds) {
            ctx.currentDeck.append(CardFactory::createCard(id, nullptr));
        }
        for (const QString& id : save->relicIds) {
            ctx.relics.append(RelicFactory::createRelic(id, nullptr));
        }

        EventLauncher* launcher = new EventLauncher(this);

        if (launcher->getPlayer()) {
            m_topBar->bindPlayer(launcher->getPlayer());
        }

        if (launcher->getCardManager()) {
            connect(launcher->getCardManager(), &CardManager::cardInsertedToDiscard, this, [this](Card* c) {
                Q_UNUSED(c);
                m_topBar->refreshDeckCount();
            });
        }

        // ========================================================
        // 📡 接收器 1：纯文字事件界面
        // ========================================================
        connect(launcher, &EventLauncher::showEventViewRequest, this, [this](EventBaseView* view) {
            view->setParent(this);
            view->setGeometry(-5, -5, 1610, 910);
            m_stack->addWidget(view);
            m_stack->setCurrentWidget(view);
            view->raise();
            m_topBarView->raise();
            m_curtain->raise();
            view->show();

            m_fadeAnimation->disconnect();
            m_fadeAnimation->setStartValue(1.0);
            m_fadeAnimation->setEndValue(0.0);
            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_curtain->hide();
            });
            m_fadeAnimation->start();
        });

        // ========================================================
        // 📡 接收器 2：触发了遭遇战！
        // ========================================================
        connect(launcher, &EventLauncher::showBattleViewRequest, this, [this](BattleView* view) {
            m_currentBattleView = view;
            m_stack->addWidget(view);
            m_stack->setCurrentWidget(view);

            if (view->getEngine() && view->getEngine()->getPlayer()) {
                m_topBar->bindPlayer(view->getEngine()->getPlayer());
            }

            m_topBarView->raise();
            m_curtain->raise();

            m_fadeAnimation->disconnect();
            m_fadeAnimation->setStartValue(1.0);
            m_fadeAnimation->setEndValue(0.0);
            connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
                m_curtain->hide();
            });
            m_fadeAnimation->start();
        });

        // ========================================================
        // 📡 接收器 3：纯文字事件结束
        // ========================================================
        connect(launcher, &EventLauncher::eventConcluded, this, [this](EventResult result) {
            GlobalSaveData* save = GlobalSaveData::getInstance();
            save->currentHp = result.remainingHp;
            save->maxHp = result.finalMaxHp;
            save->gold = result.currentGold;

            if (result.deckChanged) {
                save->deckIds.clear();
                for (Card* c : result.resultDeck) save->deckIds.append(c->getId());
            }
            if (result.relicsChanged) {
                save->relicIds.clear();
                for (Relic* r : result.resultRelics) save->relicIds.append(r->getId());
            }

            m_topBar->updateGold(save->gold);
            m_topBar->updateHp(save->currentHp, save->maxHp);
            m_topBar->refreshDeckCount();

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

                QWidget* currentEventWidget = m_stack->currentWidget();
                m_stack->setCurrentIndex(1);
                if (currentEventWidget != m_stack->widget(1)) {
                    m_stack->removeWidget(currentEventWidget);
                    currentEventWidget->deleteLater();
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
        });

        // 📡 接收器 4：战斗打赢了！
        connect(launcher, &EventLauncher::battleEncounterFinished, this, &GameWindow::onBattleConcluded);

        // 发射启动！
        launcher->launch(ctx);

        // 🌟 遗物监听天线
        if (launcher->getRelicManager()) {
            connect(launcher->getRelicManager(), &RelicManager::relicAdded, this, [this](Relic* r) {
                QPointF startPos(800, 450);
                int trayStartX = 10;
                int trayStartY = 55;
                int spacing = 8;
                int currentIndex = m_globalRelics.size();
                QPointF endPos(trayStartX + currentIndex * (48 + spacing) + 24, trayStartY + 24);

                playGlobalParticleEffect(startPos, endPos, "Relic", [this, r]() {
                    r->setParent(this);
                    m_globalRelics.append(r);
                    m_globalRelicTray->setRelics(m_globalRelics);
                });
            });
        }
    });

    m_fadeAnimation->start();
}

void GameWindow::playLootMeteor(const QString& imagePath, QPoint startPos, QPoint endPos, std::function<void()> onLanded) {
    // 1. 創造幽靈圖層
    QLabel* meteor = new QLabel(this);
    QPixmap pix(imagePath);
    if(pix.isNull()) pix = QPixmap(":/resources/images/ui/default_meteor.png"); // 防呆機制

    // 縮放到合適的流星大小 (比如 64x64)
    meteor->setPixmap(pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    meteor->setAttribute(Qt::WA_TransparentForMouseEvents); // 滑鼠穿透，不阻擋玩家操作
    meteor->setGeometry(startPos.x(), startPos.y(), 64, 64);

    // 確保流星在最頂層
    meteor->show();
    meteor->raise();
    m_topBarView->raise(); // 確保頂欄永遠壓在上面，這樣流星會有一種「飛進去」的感覺

    // 2. 賦予靈魂：飛行軌跡動畫
    QPropertyAnimation* anim = new QPropertyAnimation(meteor, "pos");
    anim->setDuration(500); // 飛行時間 0.5秒
    anim->setStartValue(startPos);
    anim->setEndValue(endPos);
    anim->setEasingCurve(QEasingCurve::InQuad); // 漸進加速，打擊感更強！

    // 3. 抵達終點：銷毀幽靈，執行真正的數據入帳！
    connect(anim, &QPropertyAnimation::finished, this, [meteor, onLanded]() {
        meteor->hide();
        meteor->deleteLater();
        if (onLanded) onLanded(); // 🔴 執行入帳！
    });

    // 點火發射！
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void GameWindow::playGlobalParticleEffect(QPointF startPos, QPointF endPos, const QString& type, std::function<void()> onLanded) {
    // ========================================================
    // 🔮 跨次元結界！覆蓋在整個 GameWindow 之上
    // ========================================================
    QGraphicsView* fxView = new QGraphicsView(this); // 直接掛在 GameWindow 上
    fxView->resize(1600, 900);
    fxView->setStyleSheet("background: transparent; border: none;");
    fxView->setAttribute(Qt::WA_TransparentForMouseEvents);
    fxView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fxView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* fxScene = new QGraphicsScene(0, 0, 1600, 900, fxView);
    fxView->setScene(fxScene);
    fxView->show();
    fxView->raise(); // 🔴 絕對壓制，蓋過所有事件視窗和 TopBar！

    // 拋物線控制點 (向上拋的弧度)
    QPointF ctrlPos(startPos.x() + (endPos.x() - startPos.x()) * 0.4, startPos.y() - 300);

    const int totalSteps = 50;
    const int interval = 16;

    // 🌟 光球與顏色設定
    auto* glowOrb = new QGraphicsEllipseItem(-25, -25, 50, 50);
    QRadialGradient gradient(0, 0, 25);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));

    QColor particleColor;
    if (type == "Relic") {
        gradient.setColorAt(0.3, QColor(50, 200, 255, 255));
        gradient.setColorAt(1.0, QColor(0, 100, 255, 0));
        particleColor = QColor(100, 220, 255, 200);
    } else if (type == "Gold") {
        gradient.setColorAt(0.3, QColor(255, 200, 50, 255));
        gradient.setColorAt(1.0, QColor(255, 100, 0, 0));
        particleColor = QColor(255, 220, 100, 200);
    } else { // Card 或其他
        gradient.setColorAt(0.3, QColor(200, 50, 255, 255));
        gradient.setColorAt(1.0, QColor(100, 0, 255, 0));
        particleColor = QColor(220, 100, 255, 200);
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
        qreal easedT = t * t * (3 - 2 * t);
        qreal u = 1.0 - easedT;
        QPointF currentPos = u * u * startPos + 2 * u * easedT * ctrlPos + easedT * easedT * endPos;
        glowOrb->setPos(currentPos);
        glowOrb->setScale(1.0 + std::sin(t * 3.14159) * 0.3);

        for(int i = 0; i < 2; i++) {
            auto* dot = new QGraphicsEllipseItem(-4, -4, 8, 8);
            dot->setBrush(particleColor);
            dot->setPen(Qt::NoPen);
            dot->setPos(currentPos + QPointF((QRandomGenerator::global()->generateDouble()-0.5)*20, (QRandomGenerator::global()->generateDouble()-0.5)*20));
            fxScene->addItem(dot);

            qreal dx = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            qreal dy = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            trails->append({dot, dx, dy, 0, 10 + QRandomGenerator::global()->bounded(8)});
        }

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

        // 💥 抵達終點
        if (s >= totalSteps) {
            timer->stop();

            // 🔴 抵達終點的瞬間，執行入帳回呼！(完美解決延遲 800ms 的硬編碼問題)
            if (onLanded) onLanded();

            for(int i = 0; i < 10; i++) {
                auto* spark = new QGraphicsEllipseItem(-3, -3, 6, 6);
                spark->setBrush(Qt::white);
                spark->setPen(Qt::NoPen);
                spark->setPos(endPos);
                fxScene->addItem(spark);

                qreal angle = i * (3.14159 * 2 / 10.0);
                qreal speed = 5.0 + QRandomGenerator::global()->generateDouble() * 3.0;
                qreal vX = std::cos(angle) * speed;
                qreal vY = std::sin(angle) * speed;

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

            for (auto& tr : *trails) { fxScene->removeItem(tr.dot); delete tr.dot; }
            delete trails;
            fxScene->removeItem(glowOrb);
            delete glowOrb;
            delete pStep;
            timer->deleteLater();

            QTimer::singleShot(500, fxView, [fxView]() {
                fxView->hide();
                fxView->deleteLater();
            });
        }
    });
    timer->start(interval);
}

// 在 GameWindow 中新增这个函数
void GameWindow::refreshTopBarRelics() {
    GlobalSaveData* save = GlobalSaveData::getInstance();

    // 1. 清空旧面子账（注意内存安全，把旧图标都删掉）
    for (Relic* r : m_globalRelics) {
        r->deleteLater();
    }
    m_globalRelics.clear();

    // 2. 严格按照“天道户口本”，重新生成一模一样的克隆体！
    for (const QString& id : save->relicIds) {
        Relic* clonedRelic = RelicFactory::createRelic(id, this);
        if (clonedRelic) {
            m_globalRelics.append(clonedRelic);
        }
    }

    // 3. 把这份绝对正确、不会有任何遗漏的名单交给托盘！
    m_globalRelicTray->setRelics(m_globalRelics);
}

// ==========================================
// 💾 新增：处理“读档继续游戏”的黑场转场动画
// ==========================================
void GameWindow::handleContinueGameTransition() {
    qDebug() << "[GameWindow] 收到继续游戏请求，正在根据读档数据重建世界...";

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        // 🔴 1. 此时内存中的 GlobalSaveData 已经被读档刷新了！
        // 我们要让 GameWindow 的顶栏立刻同步这些老玩家的数据！
        GlobalSaveData* save = GlobalSaveData::getInstance();
        m_topBar->updateHp(save->currentHp, save->maxHp);
        m_topBar->updateGold(save->gold);
        m_topBar->refreshDeckCount();
        refreshTopBarRelics(); // 重新生成上次存档的遗物！

        // 🔴 2. 切换到频道 1（大地图），并唤醒顶栏
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

// ==========================================
// 💾 新增：处理“保存并退出”返回主菜单
// ==========================================
void GameWindow::handleReturnToTitle() {
    qDebug() << "[GameWindow] 收到保存退出请求，准备隐藏UI并退回主界面...";

    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect();

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        // 🔴 1. 隐藏战斗用的悬浮顶栏
        m_topBarView->hide();
        m_rewardScreen->hide();

        // 🔴 2. 暴力切回频道 0（主菜单）
        m_stack->setCurrentIndex(0);

        // 🔴 关键修复：刷新开始界面的按钮状态（点亮继续游戏）
        m_titleView->refreshSaveState();

        // 🔴 3. 清理一下有可能遗留的战斗画面尸体
        if (m_currentBattleView) {
            m_stack->removeWidget(m_currentBattleView);
            m_currentBattleView->deleteLater();
            m_currentBattleView = nullptr;
            m_launcher = nullptr;
        }

        // 🔥 清理可能遗留的篝火画面
        for (auto* view : this->findChildren<CampfireView*>()) {
            view->hide();
            view->deleteLater();
        }

        // 🛍️ 清理可能遗留的商店画面
        for (auto* view : this->findChildren<MerchantView*>()) {
            view->hide();
            view->deleteLater();
        }

        // 🎁 清理可能遗留的宝箱画面
        for (auto* view : this->findChildren<ChestView*>()) {
            view->hide();
            view->deleteLater();
        }

        // ❓ 清理可能遗留的问号纯文本事件画面
        for (auto* view : this->findChildren<EventBaseView*>()) {
            view->hide();
            view->deleteLater();
        }

        m_fadeAnimation->disconnect();
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);

        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_curtain->hide();
            // 注意：如果你希望不关游戏直接亮起继续按钮，可以在这里调用 m_titleView 的某个刷新函数
            // 如果不写，玩家需要关掉窗口重开游戏，继续按钮才会根据本地 JSON 亮起，也是可以的。
        });
        m_fadeAnimation->start();
    });

    m_fadeAnimation->start();
}