#include "EventLauncher.h"
#include "../entities/Player.h"
#include "../ui/events/BigFishView.h"
#include "../ui/events/ClericView.h"
#include "../ui/events/DesignerView.h"
#include "../ui/events/SelfNoteView.h"
#include "../ui/events/GoldenWingView.h"
#include "battlelauncher.h".h"
#include "../logic/GlobalSaveData.h" // 🔴 引入唯一真神！
#include "../logic/CardFactory.h"
#include <QDebug>
#include <QRandomGenerator>
#include "logic/RelicFactory.h"
#include "battlelauncher.h"

EventLauncher::EventLauncher(QObject* parent)
    : QObject(parent), m_view(nullptr) {}

EventLauncher::~EventLauncher() {}

void EventLauncher::launch(const EventContext& context) {
    qDebug() << "[EventLauncher] Launching QuestionMark Event -> Subtype:" << context.eventSubtype;

    // ========================================================
    // 🔴 刪除了舊版所有關於火堆、商店、寶箱的 Switch 分支！
    // 這些現在全部歸 GameWindow 的黑幕轉場管！
    // 這裡只處理純粹的問號事件 (QuestionMark)！
    // ========================================================
    launchQuestionMark(context);
}

// ============================================================
// ❓ 問號事件核心分配器
// ============================================================
void EventLauncher::launchQuestionMark(const EventContext& context) {
    GlobalSaveData* save = GlobalSaveData::getInstance();

    m_player = new Player("铁甲战士", save->maxHp, save->maxEnergy, save->gold, this);
    m_player->setHp(save->currentHp);

    m_cardManager = new CardManager(this);
    QList<Card*> tempDeck;
    for (const QString& id : save->deckIds) {
        Card* c = CardFactory::createCard(id, m_cardManager);
        c->setOwner(m_player); // 👈 上一回合的防禦裝備，必須保留！
        tempDeck.append(c);
    }
    m_cardManager->initializeDeck(tempDeck);

    m_relicManager = new RelicManager(this);
    for (Relic* r : context.relics) {
        m_relicManager->addRelic(RelicFactory::createRelic(r->getId(), m_relicManager));
    }

    // ========================================================
    // 🚨 終極救命樞紐：建立「和平沙盒」虛擬引擎！
    // 讓 BattleEngine 發揮它 Router (路由器) 的作用，將孤立的 Player 和 CardManager 完美綁定！
    // 這樣 CardItem 在繪製時，就能拿到合法的上下文，徹底消滅 QList 記憶體越界！
    // ========================================================
    BattleEngine* dummyEngine = new BattleEngine(m_player, QList<Enemy*>(), m_cardManager, m_relicManager);
    dummyEngine->setParent(this); // 掛載在沙盒上，隨沙盒無痕銷毀！

    if (context.eventSubtype == "BigFish") {
        m_view = new BigFishView(m_player, m_cardManager, m_relicManager);
    }
    else if (context.eventSubtype == "Cleric") {
        m_view = new ClericView(m_player, m_cardManager, m_relicManager);
    }
    else if (context.eventSubtype == "Designer") {
        m_view = new DesignerView(m_player, m_cardManager, m_relicManager);
    }
    else if (context.eventSubtype == "SelfNote") {
        m_view = new SelfNoteView(m_player, m_cardManager, m_relicManager);
    }
    else if (context.eventSubtype == "GoldenWing") {
        m_view = new GoldenWingView(m_player, m_cardManager, m_relicManager);
    }
    else if (context.eventSubtype == "MonsterEncounter" || context.eventSubtype.isEmpty()) {
        // ========================================================
        // ⚔️ 問號裡的怪物遭遇戰：全面升級對接最新的 BattleLauncher！
        // ========================================================
        qDebug() << "[EventLauncher] QuestionMark -> MonsterEncounter";

        BattleContext bCtx;
        bCtx.currentHp = save->currentHp;
        bCtx.maxHp = save->maxHp;
        bCtx.gold = save->gold;
        bCtx.maxEnergy = save->maxEnergy;

        // 從檔案直接讀取卡牌，享受 Read-Only 乾淨架構
        for (const QString& id : save->deckIds) {
            bCtx.currentDeck.append(CardFactory::createCard(id, nullptr));
        }

        // 這裡如果你在 GameWindow 裡有傳入 m_globalRelics，可以放在 context 裡接過來
        bCtx.relics = context.relics;

        // 偽裝成普通怪節點！
        bCtx.nodeType = NodeType::Monster;
        bCtx.currentLayer = context.currentLayer;

        BattleLauncher* bLauncher = new BattleLauncher(this);
        connect(bLauncher, &BattleLauncher::battleConcluded, this, [this, bLauncher](BattleResult bResult) {

            // 這裡不再需要發射舊的 EventResult，
            // 因為我們的 GameWindow 已經有統一的 onBattleConcluded 結算出口了！
            // 這裡只需要把訊號轉發出去，讓外層去處理戰利品介面即可！

            emit battleEncounterFinished(bResult);

            bLauncher->deleteLater();
            QTimer::singleShot(0, this, &QObject::deleteLater);
        });

        // 🔴 這裡的 bLauncher->launch 會返回一個 BattleView，
        // 你需要把它 emit 出去，讓 GameWindow 把它加到 Stack 裡！
        BattleView* bView = bLauncher->launch(bCtx);
        emit showBattleViewRequest(bView);

        return; // 戰鬥事件不需要往下走了
    }
    else {
        qDebug() << "[EventLauncher] 未知的問號事件！";
        emit eventConcluded(EventResult());
        return;
    }

    // ========================================================
    // 📝 純文字問號事件的通用結算邏輯
    // ========================================================
    if (m_view) {
        connect(m_view, &EventBaseView::eventFinished, this, [this]() {
            EventResult result;

            // 🔴 1. 拒絕垃圾記憶體！把沙盒裡的真實屬性打包進合同！
            result.remainingHp = m_player->getHp();
            result.finalMaxHp = m_player->getMaxHp(); // 裝入最大血量
            result.currentGold = m_player->getGold();

            // 🔴 2. 收集沙盒裡的所有卡牌（打亂重組）
            result.resultDeck.append(m_cardManager->getDrawPile());
            result.resultDeck.append(m_cardManager->getHand());
            result.resultDeck.append(m_cardManager->getDiscardPile());
            result.resultDeck.append(m_cardManager->getExhaustPile());

            // 🔴 3. 收集沙盒裡的遺物
            result.resultRelics = m_relicManager->getRelics();

            // 標記狀態已改變
            result.hpChanged = true;
            result.goldChanged = true;
            result.deckChanged = true;
            result.relicsChanged = true;

            // 🚀 將裝滿真實數據的合同發射給 GameWindow！
            emit eventConcluded(result);
            this->deleteLater();
        });

        // 把純文字事件的視圖發送給 GameWindow 顯示！
        emit showEventViewRequest(m_view);
    }
}
