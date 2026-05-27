#include <QApplication>
#include <QDebug>
#include "api/EventLauncher.h"
#include "api/EventAPI.h"
#include "cards/StrikeCard.h"
#include "cards/DefendCard.h"
#include "cards/BashCard.h"
#include "entities/relics/PenNibRelic.h"
#include "entities/relics/OrichalcumRelic.h"

/* 
// 原 main.cpp 备份 (战斗模块入口)
#include <QApplication>
#include <QDebug>
#include "api/BattleLauncher.h"
#include "api/BattleAPI.h"
... (省略具体 include)

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    BattleContext context;
    ...
    BattleLauncher launcher;
    launcher.launch(context);
    return a.exec();
}
*/

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ==========================================
    // 火堆事件测试入口
    // ==========================================
    EventContext context;
    context.currentHp = 40;   // 设置较低血量以测试“休息”效果
    context.maxHp = 80;
    context.gold = 300;
    context.maxEnergy = 3;
    context.eventType = EventType::Campfire;

    // 填充一些初始卡牌
    for (int i = 0; i < 5; ++i) context.currentDeck.append(new StrikeCard(&a));
    for (int i = 0; i < 5; ++i) context.currentDeck.append(new DefendCard(&a));
    context.currentDeck.append(new BashCard(&a));

    // 填充一些遗物
    context.relics.append(new PenNibRelic());
    context.relics.append(new OrichalcumRelic());

    // 实例化事件启动器
    EventLauncher* launcher = new EventLauncher();

    // 监听结算信号
    QObject::connect(launcher, &EventLauncher::eventConcluded, [](EventResult result) {
        qDebug() << "===========================================";
        qDebug() << "[Test] Event Concluded!";
        qDebug() << "Remaining HP:" << result.remainingHp;
        qDebug() << "Current Gold:" << result.currentGold;
        qDebug() << "Deck Size:" << result.resultDeck.size();
        qDebug() << "===========================================";
    });

    // 启动火堆事件
    launcher->launch(context);

    return a.exec();
}
