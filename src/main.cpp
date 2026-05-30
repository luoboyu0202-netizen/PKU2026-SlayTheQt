#include <QApplication>
#include <QScrollArea>
#include <QDebug>
#include "map/mapmanager.h"
#include "logic/GlobalSaveData.h"
#include "api/EventLauncher.h"
#include "api/EventAPI.h"
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"

#if 1  // Test: Merchant event (set to 0 for map testing)
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->initNewGame();
    save->gold = 200;

    EventContext ctx;
    ctx.eventType = EventType::Merchant;
    ctx.currentHp = save->currentHp;
    ctx.maxHp = save->maxHp;
    ctx.gold = save->gold;
    ctx.maxEnergy = save->maxEnergy;

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
        qDebug() << "Merchant event done. Gold:" << result.currentGold
                 << "HP:" << result.remainingHp << "Cards:" << result.resultDeck.size();
        a.quit();
    });
    launcher->launch(ctx);

    return a.exec();
}
#else
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    GlobalSaveData::getInstance()->initNewGame();

    MapManager* mapWidget = new MapManager();

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWindowTitle("Slay the Qt - 大地图测试");
    scrollArea->setWidget(mapWidget);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setFixedSize(1280, 720);
    scrollArea->show();

    return a.exec();
}
#endif
