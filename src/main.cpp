#include <QApplication>
#include "ui/GameWindow.h"
#include "logic/GlobalSaveData.h"
#include "api/EventLauncher.h"
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"
#include <QDebug>

// ============================================================
// 测试开关：设为 1 启用对应事件独立测试，设为 0 则走大地图
// ============================================================
#define TEST_EVENT 0          // 0=地图模式, 1=独立测试事件
#define TEST_WHICH 5          // 1=Campfire, 2=Chest, 3=Merchant, 4=QuestionMark(GoldenWing), 5=QuestionMark(BigFish)

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->initNewGame();
    save->gold = 999;

#if TEST_EVENT
    // ========================================================
    // 🧪 队友的沙盒独立测试模式！极简启动！
    // ========================================================
    EventContext ctx;
    ctx.currentHp = save->currentHp;
    ctx.maxHp = save->maxHp;
    ctx.gold = save->gold;
    ctx.maxEnergy = save->maxEnergy;

#if TEST_WHICH == 1
    ctx.eventType = EventType::Campfire;
#elif TEST_WHICH == 2
    ctx.eventType = EventType::Chest;
#elif TEST_WHICH == 3
    ctx.eventType = EventType::Merchant;
#elif TEST_WHICH == 4 || TEST_WHICH == 5
    ctx.eventType = EventType::QuestionMark;
#if TEST_WHICH == 4
    ctx.eventSubtype = "GoldenWing";
#else
    ctx.eventSubtype = "BigFish";
#endif
#endif

    for (const QString& id : save->deckIds) {
        Card* c = CardFactory::createCard(id);
        if (c) ctx.currentDeck.append(c);
    }
    for (const QString& id : save->relicIds) {
        Relic* r = RelicFactory::createRelic(id);
        if (r) ctx.relics.append(r);
    }

    EventLauncher* launcher = new EventLauncher();
    QObject::connect(launcher, &EventLauncher::eventConcluded, [&a](EventResult result) {
        qDebug() << "Event done. HP:" << result.remainingHp
                 << "Gold:" << result.currentGold
                 << "Cards:" << result.resultDeck.size();
        a.quit();
    });
    launcher->launch(ctx);

    return a.exec();
#else
    // ========================================================
    // 🎮 我们的 3A 级大地图正式游戏模式！
    // ========================================================
    GameWindow window;
    window.show();

    return a.exec();
#endif
}