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

    #if 1  // Test: QuestionMark GoldenWing
        GlobalSaveData* save = GlobalSaveData::getInstance();
        save->initNewGame();
        save->currentHp = 80; 
        save->maxHp = 80;
        save->gold = 0; // 初始没钱，测试摧毁获得金币

        EventContext ctx;
        ctx.eventType = EventType::QuestionMark;
        ctx.eventSubtype = "GoldenWing";
        ctx.currentHp = save->currentHp;
        ctx.maxHp = save->maxHp;
        ctx.gold = save->gold;
        ctx.maxEnergy = save->maxEnergy;

        // 填充初始卡组 (默认已有打击，满足摧毁条件)
        for (const QString& id : save->deckIds) {
            Card* c = CardFactory::createCard(id);
            if (c) ctx.currentDeck.append(c);
        }

        EventLauncher* launcher = new EventLauncher();
        QObject::connect(launcher, &EventLauncher::eventConcluded, [&a](EventResult result) {
            qDebug() << "--- GoldenWing Test End ---";
            qDebug() << "Final HP:" << result.remainingHp;
            qDebug() << "Final Gold:" << result.currentGold;
            a.quit();
        });
        launcher->launch(ctx);
    #endif

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
