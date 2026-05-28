#include "BattleLauncher.h"
#include "../entities/Player.h"
#include "../entities/Enemy.h"
#include <QDebug>
#include "logic/enemyfactory.h"

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
    // 🎁 结算升级：生成战报与战利品包裹！
    // ========================================================
    connect(m_engine, &BattleEngine::battleEnded, this, [this, player, context, relicManager, cardManager](bool victory) {
        BattleResult result;
        result.isVictory = victory;
        result.currentHp = player->getHp();
        result.maxHp = player->getMaxHp();
        result.maxEnergy = player->getMaxEnergy();
        result.currentGold = player->getGold(); // ⚠️ 这是战斗结算前的本金

        // 1. 🛡️ 内存安全转化：提取战斗结束时的卡牌与遗物实体 ID
        // （防止战斗中有卡牌被销毁/增加，或者有遗物被损坏）
        QList<Card*> allSurvivingCards;
        allSurvivingCards.append(cardManager->getHand());
        allSurvivingCards.append(cardManager->getDrawPile());
        allSurvivingCards.append(cardManager->getDiscardPile());
        allSurvivingCards.append(cardManager->getExhaustPile()); // 被消耗的牌只是本场战斗不能用，并没有被撕毁
        allSurvivingCards.append(cardManager->m_playedPowers);   // 打出的能力牌也要回收

        for (Card* c : allSurvivingCards) {
            if (c) result.finalDeckIds.append(c->getId());
        }

        for (Relic* r : relicManager->getRelics()) {
            if (r) result.finalRelicIds.append(r->getId());
        }

        // 2. 🎁 战利品生成逻辑（完全依赖 context 的抽象标签！）
        if (victory) {
            result.hasCardReward = true;
            result.cardRewardCount = 3;

            if (context.nodeType == "Boss") {
                result.rewardGold = 100;
                result.rewardRelicIds.append("relic_snecko_eye"); // 以后换成用工厂随机掉落
            } else if (context.nodeType == "Elite") {
                result.rewardGold = 30;
                result.rewardRelicIds.append("relic_pen_nib"); // 以后换成用工厂随机掉落
            } else {
                result.rewardGold = 15; // 普通怪保底金币
            }
        }

        emit battleConcluded(result);
    });

    qDebug() << "正式亮相后的真实窗口大小:" << m_view->size();
    m_engine->startBattle();
    return m_view;
}
