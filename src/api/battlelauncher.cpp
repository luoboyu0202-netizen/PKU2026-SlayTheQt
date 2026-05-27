#include "BattleLauncher.h"
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include <QDebug>
#include "logic/enemyfactory.h"

BattleLauncher::BattleLauncher(QObject* parent)
    : QObject(parent), m_view(nullptr), m_engine(nullptr) {}

BattleLauncher::~BattleLauncher() {
    if (m_view) delete m_view;
    if (m_engine) delete m_engine;
}

void BattleLauncher::launch(const BattleContext& context) {
    qDebug() << "[API] Launching battle with enemy:" << context.enemySeedOrId;

    // 1. 将外部的冷数据 (Context)，实例化为我们战斗模块内部的活生生肉体
    Player* player = new Player("铁甲战士", context.maxHp, context.maxEnergy, context.gold);
    player->setHp(context.currentHp);

    // ========================================================
    // 🔴【群殴编制重构】：把生成的怪物装进大军列表里！
    // ========================================================
    // ========================================================
    // 🟢 现在你直接呼叫遭遇战工厂，一键生成整编军团！
    // 假设你的 context.enemySeedOrId 传进来的是 "Slime_Squad"
    // ========================================================
    QList<Enemy*> enemyList = EnemyFactory::createEncounter("Slime_Squad");

    // 然后把这支军队塞进大脑里（假设你的 BattleEngine 有一个初始化或者塞入敌人的接口喵）
    // 比如：m_engine->initBattle(player, enemyList);
    // 或者如果你是直接赋值：
    // m_engine->m_enemies = enemyList; (如果在引擎内部写的话)

    // 实例化卡牌和遗物
    CardManager* cardManager = new CardManager();
    cardManager->initializeDeck(context.currentDeck);

    RelicManager* relicManager = new RelicManager();
    for (Relic* r : context.relics) relicManager->addRelic(r);

    // ========================================================
    // 🔴【核心闭环】：把装填好的 enemyList 递给大脑！
    // ========================================================
    // 以前是传 enemy，现在传 enemyList！
    m_engine = new BattleEngine(player, enemyList, cardManager, relicManager);

    // 填入背景图片路径（必须在 bindEngine 之前）
    m_engine->setBackgroundPath(":/resources/images/thunder_beach.png");

    m_view = new BattleView();
    m_view->bindEngine(m_engine);
    m_view->bindRelics(relicManager);


    // =========================================================
    // 3. 🔴【最核心的黑盒闭环】：监听内部战斗结束，生成结算报告并销毁自己
    // =========================================================
    // 🟢 正确写法：在方括号里显式写上 context 和 relicManager，把它们复印保存下来！
    connect(m_engine, &BattleEngine::battleEnded, this, [this, player, context, relicManager](bool victory) {
        BattleResult result;
        result.isVictory = victory;
        result.currentHp = player->getHp();
        result.gold = player->getGold();
        result.maxHp=player->getMaxHp();
        result.maxEnergy=player->getMaxEnergy();
        // 现在用复印下来的 context 就绝对安全了喵！
        result.relics = context.relics;

        m_view->close();
        emit battleConcluded(result);

        relicManager->deleteLater();
        this->deleteLater();
    });

    // 4. 显示舞台，吹响战斗号角！
    m_view->show();
    qDebug() << "正式亮相后的真实窗口大小:" << m_view->size(); // 👈 此时的大小 100% 准确！
    m_engine->startBattle();
}
