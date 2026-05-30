#include "ui/GameWindow.h"
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include "../logic/RelicFactory.h"
#include <QVBoxLayout>
#include <QScrollArea> // 提供滚动视口
#include <QScroller>   // 提供触摸拖拽与物理惯性

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
    // 🔴 实例化开始界面，并塞进频道 0
    // ==========================================
    m_titleView = new TitleMenuView(this);
    m_stack->addWidget(m_titleView); // 📺 频道 0：开始界面！

    // 3. 实例化大地图，并准备塞进频道 1
    m_mapManager = new MapManager();

    // 用 ScrollArea 把地图包起来，作为摄像机视口
    QScrollArea* mapScroll = new QScrollArea();
    mapScroll->setWidget(m_mapManager);
    mapScroll->setAlignment(Qt::AlignCenter);

    // 🌟 魔法步骤 1：禁止 ScrollArea 压缩地图，保持地图原始长度以产生滚动区间
    mapScroll->setWidgetResizable(false);

    // 🌟 魔法步骤 2：隐藏丑陋的原生滚动条（依然可以滚，只是变纯净了）
    mapScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mapScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 🌟 魔法步骤 3：QScroller 强行接管！开启鼠标左键按住拖拽与物理惯性
    QScroller::grabGesture(mapScroll, QScroller::LeftMouseButtonGesture);

    // 🔧 魔法步骤 4：精调物理引擎，增加摩擦力，减小惯性！
    QScroller *scroller = QScroller::scroller(mapScroll);
    QScrollerProperties props = scroller->scrollerProperties();
    props.setScrollMetric(QScrollerProperties::DecelerationFactor, 0.6); // 增大减速阻力
    props.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.55);   // 限制最大甩动速度
    scroller->setScrollerProperties(props);

    // ==========================================
    // 🔴 将地图加入电视机！注意，现在它是频道 1！
    // ==========================================
    m_stack->addWidget(mapScroll); // 📺 频道 1：大地图！

    // 默认显示频道 0（开始界面）
    m_stack->setCurrentWidget(m_titleView);

    // ==========================================
    // 🎁 将战利品界面作为“全局幽灵悬浮层”！
    // ==========================================
    m_rewardScreen = new RewardScreen(this); // 直接挂在 GameWindow 下
    m_rewardScreen->move(0, 0); // 从屏幕左上角铺满
    m_rewardScreen->hide(); // 默认隐藏！
    connect(m_rewardScreen, &RewardScreen::proceedRequested, this, &GameWindow::onRewardProceedRequested);

    // 4. 接好情报通讯线！
    connect(m_mapManager, &MapManager::battleRequested, this, &GameWindow::onBattleRequested);

    // ==========================================
    // 🔴 初始化全局黑幕 (复用你写好的动画系统)
    // ==========================================
    m_curtain = new QWidget(this);
    m_curtain->resize(1600, 900); // 绝对覆盖全屏！
    m_curtain->setStyleSheet("background-color: #000000;"); // 纯正的黑！
    m_curtain->hide(); // 默认藏起来

    // 给幕布附魔透明度效果
    m_curtainEffect = new QGraphicsOpacityEffect(m_curtain);
    m_curtainEffect->setOpacity(0.0); // 初始完全透明
    m_curtain->setGraphicsEffect(m_curtainEffect);

    // 绑定动画引擎，修改的属性叫 "opacity" (透明度)
    m_fadeAnimation = new QPropertyAnimation(m_curtainEffect, "opacity", this);
    m_fadeAnimation->setDuration(350); // 🌟 350毫秒的丝滑时长

    // 🔴 监听开始界面的“开战信号”，执行启动游戏动画
    connect(m_titleView, &TitleMenuView::startGameRequested, this, &GameWindow::handleStartGameTransition);

    // =======================================================
    // 🎁 接收飞行抵达信号，更新底层 UI 与全局存档！
    // =======================================================
    connect(m_rewardScreen, &RewardScreen::relicFlightFinished, this, [this](QString relicId) {
        GlobalSaveData::getInstance()->relicIds.append(relicId);
        qDebug() << "[GameWindow] 🏺 遗物飞行抵达！已正式入账：" << relicId;

        if (m_currentBattleView) {
            // 1. 造出实体
            Relic* newRelic = RelicFactory::createRelic(relicId, m_currentBattleView);

            // 2. 🔴 极其精准的层级穿透：View -> Engine -> RelicManager !
            BattleEngine* engine = m_currentBattleView->getEngine();
            if (engine && engine->m_relicManager) {
                engine->m_relicManager->addRelic(newRelic);
            }
        }
    });

    connect(m_rewardScreen, &RewardScreen::goldFlightFinished, this, [this](int amount) {
        // 1. 金币正式入账！
        GlobalSaveData::getInstance()->gold += amount;
        qDebug() << "[GameWindow] 💰 金币飞行抵达！当前余额：" << GlobalSaveData::getInstance()->gold;

        // 2. 更新底层的 TopBar 余额显示
        if (m_currentBattleView) {
            Player* p = m_currentBattleView->getEngine()->getPlayer();
            if (p) p->setGold(GlobalSaveData::getInstance()->gold);
        }
    });
}

// ==========================================
// 🔴 新增：处理“开始游戏”的黑场转场动画
// ==========================================
void GameWindow::handleStartGameTransition() {
    // 1. 盖上黑布
    m_curtain->raise();
    m_curtain->show();
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->disconnect(); // 断开之前的回调

    // 当完全变黑时...
    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {

        // 🔴 趁着屏幕全黑，偷偷把频道切换到 1（大地图）
        m_stack->setCurrentIndex(1);

        // 执行淡出动画，让大地图亮起来
        m_fadeAnimation->disconnect();
        m_fadeAnimation->setStartValue(1.0);
        m_fadeAnimation->setEndValue(0.0);

        connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
            m_curtain->hide(); // 彻底隐藏幕布
        });
        m_fadeAnimation->start();
    });

    // 启动淡入动画
    m_fadeAnimation->start();
}

void GameWindow::onBattleRequested(const MapNode& node) {
    m_lastClickedNode = node;

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

        // 🔴 核心解耦：抹除硬编码！我们只把“标签”塞进密令里发往战斗引擎！
        context.nodeType = node.type;       // "Monster", "Elite", "Boss" 等
        context.currentLayer = node.layer;  // 0 ~ 14 层

        m_launcher = new BattleLauncher(this);
        connect(m_launcher, &BattleLauncher::battleConcluded, this, &GameWindow::onBattleConcluded);

        m_currentBattleView = m_launcher->launch(context);
        m_stack->addWidget(m_currentBattleView);
        m_stack->setCurrentWidget(m_currentBattleView);

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

        // ==========================================
        // 🔴 极其重要的修复：切回大地图，地图现在在频道 1！
        // ==========================================
        m_stack->setCurrentIndex(1);

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