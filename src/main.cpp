#include <QApplication>
#include <QDebug>
#include "api/BattleLauncher.h"
#include "api/BattleAPI.h" // 确保引入了合同结构体定义
#include "cards/StrikeCard.h"
#include "cards/DefendCard.h"
#include "entities/relics/Relic.h" // 假设你的遗物类在这里
#include "cards/BashCard.h"
#include "entities/cards/BurningPactCard.h"
#include "entities/cards/BloodlettingCard.h"
#include "entities/cards/ThunderclapCard.h"
#include "entities/cards/DazedCard.h"
#include "entities/cards/BurnCard.h"
#include "entities/cards/WoundCard.h"
#include "entities/cards/SlimedCard.h"
#include "entities/cards/InflameCard.h"
#include "entities/cards/MetallicizeCard.h"
#include "entities/cards/DarkEmbraceCard.h"
#include "entities/cards/FireSourceCard.h"
#include "entities/cards/PommelStrikeCard.h"
#include "entities/cards/ShrugItOffCard.h"
#include "entities/cards/PummelCard.h"
#include "entities/cards/DarkShacklesCard.h"
#include "entities/cards/SecondWindCard.h"
#include "entities/cards/ReaperCard.h"
#include "entities/cards/BarricadeCard.h"
#include "entities/cards/HellFiendCard.h"
#include "entities/cards/PourCard.h"
#include "entities/relics/PenNibRelic.h"
#include "entities/relics/OrichalcumRelic.h"
#include "entities/relics/BagOfPreparationRelic.h"
#include "entities/relics/AnchorRelic.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ==========================================
    // 1. 组装契约规定的输入数据 (BattleContext)
    // ==========================================
    BattleContext context;
    context.currentHp = 75;
    context.maxHp = 80;
    context.gold = 120;
    context.maxEnergy = 3;
    context.enemySeedOrId = "Slime_Squad"; // 传入怪物 ID 契约

    // 🛠️ 喵喵特制：一行代码生成升级牌的兵工厂！
    auto addUpgraded = [&](Card* card) {
        card->upgrade();
        context.currentDeck.append(card);
    };

    // 然后你的测试代码就可以变得极其清爽：
    for (int i = 0; i < 3; ++i) addUpgraded(new StrikeCard(&a));
    for (int i = 0; i < 3; ++i) context.currentDeck.append(new DefendCard(&a)); // 普通防御

    addUpgraded(new BashCard(&a));          // 塞入【痛击+】
    addUpgraded(new ThunderclapCard(&a));   // 塞入【闪电霹雳+】
    addUpgraded(new DarkEmbraceCard(&a));   // 塞入【黑暗之拥+】(变成1费啦)
    addUpgraded(new FireSourceCard(&a));    // 塞入【薪火之源+】
    addUpgraded(new BloodlettingCard(&a));
    addUpgraded(new BurningPactCard(&a));
    addUpgraded(new MetallicizeCard(&a));
    addUpgraded(new InflameCard(&a));
    addUpgraded(new PommelStrikeCard(&a));
    addUpgraded(new ShrugItOffCard(&a));
    addUpgraded(new PummelCard(&a));
    addUpgraded(new DarkShacklesCard(&a));
    addUpgraded(new SecondWindCard(&a));
    addUpgraded(new ReaperCard(&a));
    addUpgraded(new BarricadeCard(&a));
    addUpgraded(new HellFiendCard(&a));
    addUpgraded(new PourCard(&a));


    // 塞入初始测试遗物
    context.relics.append(new PenNibRelic());
    context.relics.append(new OrichalcumRelic());
    context.relics.append(new BagOfPreparationRelic());
    context.relics.append(new AnchorRelic());

    // ==========================================
    // 2. 实例化唯一的外部代理人，并监听它的“承诺”
    // ==========================================
    BattleLauncher launcher;

    // 当战斗圆满结束，无论生死，这个 Lambda 表达式都会全自动触发
    QObject::connect(&launcher, &BattleLauncher::battleConcluded, [](BattleResult result) {
        qDebug() << "===========================================";
        qDebug() << "[External System] Receive Battle Report!";
        qDebug() << "Victory:" << result.isVictory;
        qDebug() << "Remaining HP:" << result.remainingHp;
        qDebug() << "Final Gold:" << result.currentGold;
        qDebug() << "Relics Count:" << result.battledRelics.size();
        qDebug() << "===========================================";

        // 大地图组接收到这些数据后，可以无缝写入全局存档，或者刷新大地图UI喵！
    });

    // ==========================================
    // 3. 轰鸣起飞，开启战斗场景！
    // ==========================================
    launcher.launch(context);

    return a.exec();
}