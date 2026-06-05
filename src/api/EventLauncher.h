#pragma once
#include <QObject>
#include "EventAPI.h"
#include "BattleLauncher.h" // 🔴 修正 1：引入包含 BattleResult 完整定義的標頭檔！

class Player;
class CardManager;
class RelicManager;
class EventBaseView;
class BattleView;
// class BattleResult; ❌ 刪除這行前置宣告

class EventLauncher : public QObject {
    Q_OBJECT
public:
    explicit EventLauncher(QObject* parent = nullptr);
    ~EventLauncher();

    void launch(const EventContext& context);

    // 🔴 奧卡姆剃刀暴露的指標
    EventBaseView* getView() const { return m_view; }
    Player* getPlayer() const { return m_player; }
    RelicManager* getRelicManager() const { return m_relicManager; }
    CardManager* getCardManager() const { return m_cardManager; }

signals:
    void eventConcluded(EventResult result);
    void showEventViewRequest(EventBaseView* view);
    void showBattleViewRequest(BattleView* view);
    void battleEncounterFinished(BattleResult result);

private:
    void launchQuestionMark(const EventContext& context);

    // ❌ 修正 2：刪除 launchCampfire, launchMerchant, launchChest 的宣告！
    // 這些函數我們在 .cpp 裡已經刪掉了，留在這裡編譯器會不高興的喵！

    void emitResult(Player* player, CardManager* cardManager, RelicManager* relicManager, const EventContext& context, EventResult result);

    EventBaseView* m_view = nullptr;
    Player* m_player = nullptr;
    CardManager* m_cardManager = nullptr;
    RelicManager* m_relicManager = nullptr;
};