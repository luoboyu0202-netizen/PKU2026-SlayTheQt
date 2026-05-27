#pragma once
#include <QObject>
#include "EventAPI.h"

class Player;
class CardManager;
class RelicManager;
class EventBaseView;

class EventLauncher : public QObject {
    Q_OBJECT
public:
    explicit EventLauncher(QObject* parent = nullptr);
    ~EventLauncher();

    void launch(const EventContext& context);

signals:
    void eventConcluded(EventResult result);

private:
    void launchCampfire(Player* player, CardManager* cardManager, RelicManager* relicManager, const EventContext& context);
    void launchMerchant(Player* player, CardManager* cardManager, RelicManager* relicManager, const EventContext& context);
    void launchChest(Player* player, RelicManager* relicManager, const EventContext& context);
    void launchQuestionMark(Player* player, CardManager* cardManager, RelicManager* relicManager, const EventContext& context);

    void emitResult(Player* player, CardManager* cardManager, RelicManager* relicManager, const EventContext& context, EventResult result);

    EventBaseView* m_view = nullptr;
    Player* m_player = nullptr;
    CardManager* m_cardManager = nullptr;
    RelicManager* m_relicManager = nullptr;
};
