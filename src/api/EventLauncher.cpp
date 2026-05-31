#include "EventLauncher.h"
#include "../entities/Player.h"
#include "../logic/CardManager.h"
#include "../entities/relics/RelicManager.h"
#include "../ui/events/EventBaseView.h"
#include "../ui/events/CampfireView.h"
#include "../ui/events/ChestView.h"
#include "../ui/events/MerchantView.h"
#include "../ui/events/BigFishView.h"
#include "../ui/events/ClericView.h"
#include "../ui/events/DesignerView.h"
#include "../ui/events/SelfNoteView.h"
#include "../ui/events/GoldenWingView.h"
#include "battlelauncher.h"
#include <QDebug>
#include <QRandomGenerator>

EventLauncher::EventLauncher(QObject* parent)
    : QObject(parent) {}

EventLauncher::~EventLauncher() {
    if (m_view) delete m_view;
}

void EventLauncher::launch(const EventContext& context) {
    qDebug() << "[EventLauncher] Launching event:" << static_cast<int>(context.eventType)
             << "Subtype:" << context.eventSubtype;

    m_player = new Player("铁甲战士", context.currentHp, context.maxEnergy, context.gold, this);
    m_player->setHp(context.currentHp);

    m_cardManager = new CardManager(this);
    m_cardManager->initializeDeck(context.currentDeck);

    m_relicManager = new RelicManager(this);
    for (Relic* r : context.relics) {
        m_relicManager->addRelic(r);
    }

    switch (context.eventType) {
    case EventType::Campfire:
        launchCampfire(m_player, m_cardManager, m_relicManager, context);
        break;
    case EventType::Merchant:
        launchMerchant(m_player, m_cardManager, m_relicManager, context);
        break;
    case EventType::Chest:
        launchChest(m_player, m_relicManager, context);
        break;
    case EventType::QuestionMark:
        launchQuestionMark(m_player, m_cardManager, m_relicManager, context);
        break;
    }
}

void EventLauncher::emitResult(Player* player, CardManager* cardManager, RelicManager* relicManager,
                                const EventContext& context, EventResult result) {
    result.remainingHp = player->getHp();
    result.currentGold = player->getGold();

    result.resultDeck.clear();
    result.resultDeck.append(cardManager->getDrawPile());
    result.resultDeck.append(cardManager->getHand());
    result.resultDeck.append(cardManager->getDiscardPile());
    result.resultDeck.append(cardManager->getExhaustPile());
    result.resultDeck.append(cardManager->m_playedPowers);

    result.resultRelics = relicManager->getRelics();
    result.playerDead = player->isDead();

    emit eventConcluded(result);
}

// ============================================================
// 火堆
// ============================================================
void EventLauncher::launchCampfire(Player* player, CardManager* cardManager,
                                    RelicManager* relicManager, const EventContext& context) {
    m_view = new CampfireView(player, cardManager, relicManager);

    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });

    m_view->show();
}

// ============================================================
// 其余事件
// ============================================================
void EventLauncher::launchMerchant(Player* player, CardManager* cardManager,
                                    RelicManager* relicManager, const EventContext& context) {
    m_view = new MerchantView(player, cardManager, relicManager);

    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        result.deckChanged = true;
        result.goldChanged = true;
        result.relicsChanged = true;
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });

    m_view->show();
}

void EventLauncher::launchChest(Player* player, RelicManager* relicManager,
                                 const EventContext& context) {
    m_view = new ChestView(player, relicManager);

    connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
        EventResult result;
        emitResult(m_player, m_cardManager, m_relicManager, context, result);
        m_view->close();
        this->deleteLater();
    });

    m_view->show();
}

void EventLauncher::launchQuestionMark(Player* player, CardManager* cardManager,
                                        RelicManager* relicManager, const EventContext& context) {
    if (context.eventSubtype == "BigFish") {
        // ... (existing BigFish logic)
        m_view = new BigFishView(player, cardManager, relicManager);
        connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
            EventResult result;
            result.relicsChanged = true;
            result.hpChanged = true;
            emitResult(m_player, m_cardManager, m_relicManager, context, result);
            m_view->close();
            this->deleteLater();
        });
        m_view->show();
    }
    else if (context.eventSubtype == "Cleric") {
        qDebug() << "[EventLauncher] Launching QuestionMark -> Cleric";
        m_view = new ClericView(player, cardManager, relicManager);
        connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
            EventResult result;
            result.hpChanged = true;
            result.goldChanged = true;
            result.deckChanged = true;
            emitResult(m_player, m_cardManager, m_relicManager, context, result);
            m_view->close();
            this->deleteLater();
        });
        m_view->show();
    }
    else if (context.eventSubtype == "Designer") {
        // ... (DesignerView logic)
        m_view = new DesignerView(player, cardManager, relicManager);
        connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
            EventResult result;
            result.hpChanged = true;
            result.goldChanged = true;
            result.deckChanged = true;
            emitResult(m_player, m_cardManager, m_relicManager, context, result);
            m_view->close();
            this->deleteLater();
        });
        m_view->show();
    }
    else if (context.eventSubtype == "SelfNote") {
        // ... (SelfNoteView logic)
        m_view = new SelfNoteView(player, cardManager, relicManager);
        connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
            EventResult result;
            result.deckChanged = true;
            emitResult(m_player, m_cardManager, m_relicManager, context, result);
            m_view->close();
            this->deleteLater();
        });
        m_view->show();
    }
    else if (context.eventSubtype == "GoldenWing") {
        qDebug() << "[EventLauncher] Launching QuestionMark -> GoldenWing";
        m_view = new GoldenWingView(player, cardManager, relicManager);
        connect(m_view, &EventBaseView::eventFinished, this, [this, context]() {
            EventResult result;
            result.hpChanged = true;
            result.goldChanged = true;
            result.deckChanged = true;
            emitResult(m_player, m_cardManager, m_relicManager, context, result);
            m_view->close();
            this->deleteLater();
        });
        m_view->show();
    }
    else if (context.eventSubtype == "MonsterEncounter" || context.eventSubtype.isEmpty()) {
        qDebug() << "[EventLauncher] QuestionMark -> MonsterEncounter";

        BattleContext bCtx;
        bCtx.currentHp = player->getHp();
        bCtx.maxHp = player->getMaxHp();
        bCtx.gold = player->getGold();
        bCtx.maxEnergy = player->getMaxEnergy();
        
        // 收集所有位置的卡牌
        bCtx.currentDeck.append(cardManager->getDrawPile());
        bCtx.currentDeck.append(cardManager->getHand());
        bCtx.currentDeck.append(cardManager->getDiscardPile());
        bCtx.currentDeck.append(cardManager->getExhaustPile());
        bCtx.currentDeck.append(cardManager->m_playedPowers);

        bCtx.relics = relicManager->getRelics();
        
        QStringList encounters = {"Slime_Squad", "Single_Slime"};
        int idx = QRandomGenerator::global()->bounded(encounters.size());
        bCtx.enemySeedOrId = encounters[idx];

        BattleLauncher* bLauncher = new BattleLauncher(this);
        connect(bLauncher, &BattleLauncher::battleConcluded, this, 
            [this, player, cardManager, relicManager, context, bLauncher](BattleResult bResult) {
                EventResult eResult;
                eResult.playerDead = !bResult.isVictory;
                
                // 同步数据回外层引用
                player->setHp(bResult.currentHp);
                player->modifyGold(bResult.gold - player->getGold());
                
                // 目前 BattleResult 不支持返回 Deck 的变更，
                // 如果遭遇战中通过遗物/卡牌永久改变了牌组，此处会有同步空隙
                // 我们假设普通怪战斗不永久改变牌组。
                
                eResult.hpChanged = true;
                eResult.goldChanged = true;
                
                emitResult(player, cardManager, relicManager, context, eResult);
                
                bLauncher->deleteLater();
                // 延迟删除自身
                QTimer::singleShot(0, this, &QObject::deleteLater);
            });

        bLauncher->launch(bCtx);
    } else {
        qDebug() << "[EventLauncher] Other QuestionMark event types not implemented yet";
        EventResult result;
        emitResult(player, cardManager, relicManager, context, result);
    }
}

