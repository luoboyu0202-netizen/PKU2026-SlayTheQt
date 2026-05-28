#include "ui/GameWindow.h"
#include "../logic/GlobalSaveData.h"
#include "../logic/CardFactory.h"
#include "../logic/RelicFactory.h"
#include <QVBoxLayout>

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
    // 🔴 必须用 ScrollArea 把地图包起来，因为地图很长！
    QScrollArea* mapScroll = new QScrollArea();
    mapScroll->setWidget(m_mapManager);
    mapScroll->setAlignment(Qt::AlignCenter);

    m_stack->addWidget(mapScroll); // 📺 频道 0：大地图！

    // ==========================================
    // 🎁 将战利品界面作为“全局幽灵悬浮层”！
    // ==========================================
    m_rewardScreen = new RewardScreen(this); // 直接挂在 GameWindow 下
    m_rewardScreen->move(0, 0); // 从屏幕左上角铺满
    m_rewardScreen->hide(); // 默认隐藏！
    connect(m_rewardScreen, &RewardScreen::proceedRequested, this, &GameWindow::onRewardProceedRequested);
    // 4. 接好情报通讯线！
    connect(m_mapManager, &MapManager::battleRequested, this, &GameWindow::onBattleRequested);

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
    m_fadeAnimation->setDuration(350); // 🌟 350毫秒的丝滑时长，不拖沓也不突兀

    // =======================================================
    // 🎁 接收飞行抵达信号，更新底层 UI 与全局存档！
    // =======================================================
    // 🎁 接收飞行抵达信号，更新底层 UI 与全局存档！
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
            // 同理，假设你的 BattleView 有获取 Player 的方法
            // Player 改变金币后，TopBar 会自动接收信号刷新！
            Player* p = m_currentBattleView->getEngine()->getPlayer();
            if (p) p->setGold(GlobalSaveData::getInstance()->gold);
        }
    });

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