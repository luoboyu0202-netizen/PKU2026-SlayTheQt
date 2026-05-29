#include "BattleLauncher.h"
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include <QDebug>
#include "logic/enemyfactory.h"
#include "entities/relics/relicmanager.h"
#include "globalsavedata.h"
#include "logic/RelicFactory.h"

BattleLauncher::BattleLauncher(QObject* parent)
    : QObject(parent), m_view(nullptr), m_engine(nullptr) {}

BattleLauncher::~BattleLauncher() {
    // 🟢 析构函数保持清空，全靠 Qt 父子树自动管理！
}

BattleView* BattleLauncher::launch(const BattleContext& context) {
    m_view = new BattleView();

    Player* player = new Player("铁甲战士", context.maxHp, context.maxEnergy, context.gold);
    player->setHp(context.currentHp);
    player->setParent(m_view);

    CardManager* cardManager = new CardManager();
    cardManager->setParent(m_view);
    cardManager->initializeDeck(context.currentDeck);

    RelicManager* relicManager = new RelicManager();
    relicManager->setParent(m_view);
    for (Relic* r : context.relics) relicManager->addRelic(r);

    // ========================================================
    // 🔴 核心解耦：以后工厂只看类型（如 "Elite"）和层数，自己去卡池摇号！
    // ========================================================
    QList<Enemy*> enemyList = EnemyFactory::createEncounter(context.nodeType, context.currentLayer);

    m_engine = new BattleEngine(player, enemyList, cardManager, relicManager);
    m_engine->setParent(m_view);

    m_engine->setBackgroundPath(":/resources/images/thunder_beach.png");
    m_view->bindEngine(m_engine);
    m_view->bindRelics(relicManager);

    // ========================================================
    // 🎁 结算升级：极其纯净的数据回传 (Read-Only Deck Architecture)
    // ========================================================
    connect(m_engine, &BattleEngine::battleEnded, this, [this, player, context, relicManager](bool victory) {
        BattleResult result;
        result.isVictory = victory;
        result.currentHp = player->getHp();
        result.maxHp = player->getMaxHp();
        result.maxEnergy = player->getMaxEnergy();
        result.currentGold = player->getGold();

        // ❌ 删除了所有 allSurvivingCards 的收集逻辑！
        // ❌ 删除了对状态牌的“安检”逻辑！
        // ✅ 现在的卡组是绝对安全的，因为我们根本就不去碰全局档案！

        GlobalSaveData* save = GlobalSaveData::getInstance();

        // 依然只回收遗物（因为遗物在战斗中可能会改变计数器，比如“钢笔尖”叠了9层）
        // 如果你的遗物不需要记录战中变化，这里甚至连遗物都不用回传！
        for (Relic* r : relicManager->getRelics()) {
            if (r) {
                result.finalRelicIds.append(r->getId()); // 依然回传 ID 列表

                // 【核心】：把最新的计数器写进字典！
                save->relicCounters[r->getId()] = r->getCounter();
            }
        }

        // 2. 🎁 战利品生成逻辑
        if (victory) {
            result.hasCardReward = true;
            result.cardRewardCount = 3;

            // 🔴 呼叫全局档案，查明玩家现在身上有哪些遗物？
            GlobalSaveData* save = GlobalSaveData::getInstance();

            if (context.nodeType == "Boss") {
                result.rewardGold = 100;
                // 🎲 Boss 掉落高级遗物盲盒
                QString droppedRelic = RelicFactory::generateRandomRelic(save->relicIds);
                if (!droppedRelic.isEmpty()) {
                    result.rewardRelicIds.append(droppedRelic);
                }
            }
            else if (context.nodeType == "Elite") {
                result.rewardGold = 30;
                // 🎲 精英怪掉落遗物盲盒
                QString droppedRelic = RelicFactory::generateRandomRelic(save->relicIds);
                if (!droppedRelic.isEmpty()) {
                    result.rewardRelicIds.append(droppedRelic);
                }
            }
            else {
                // 普通怪 (Monster) 通常只掉落金币和卡牌，不掉遗物
                result.rewardGold = 15;
            }
        }

        emit battleConcluded(result);

    });

    qDebug() << "正式亮相后的真实窗口大小:" << m_view->size();
    m_engine->startBattle();
    return m_view;
}
