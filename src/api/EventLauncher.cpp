#include "EventLauncher.h"
#include "../entities/Player.h"
#include "../logic/CardManager.h"
#include "../entities/relics/RelicManager.h"
#include "../ui/events/EventBaseView.h"
#include "../ui/events/CampfireView.h"
#include <QDebug>

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
// 其余事件（待实现）
// ============================================================
void EventLauncher::launchMerchant(Player* player, CardManager* cardManager,
                                    RelicManager* relicManager, const EventContext& context) {
    Q_UNUSED(player) Q_UNUSED(cardManager) Q_UNUSED(relicManager)
    qDebug() << "[EventLauncher] Merchant event - view not yet implemented";
    EventResult result;
    emitResult(player, cardManager, relicManager, context, result);
}

void EventLauncher::launchChest(Player* player, RelicManager* relicManager,
                                 const EventContext& context) {
    Q_UNUSED(player) Q_UNUSED(relicManager)
    qDebug() << "[EventLauncher] Chest event - view not yet implemented";
    EventResult result;
    emitResult(player, m_cardManager, relicManager, context, result);
}

void EventLauncher::launchQuestionMark(Player* player, CardManager* cardManager,
                                        RelicManager* relicManager, const EventContext& context) {
    Q_UNUSED(player) Q_UNUSED(cardManager) Q_UNUSED(relicManager)
    qDebug() << "[EventLauncher] QuestionMark event - view not yet implemented";
    EventResult result;
    emitResult(player, cardManager, relicManager, context, result);
}
