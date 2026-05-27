#include <QApplication>
#include <QDebug>
#include "api/BattleLauncher.h"
#include "api/BattleAPI.h"

// 🔴 魔法核心：只需要引入这两个工厂！无需引入任何具体的卡牌和遗物！
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ==========================================
    // 1. 组装契约规定的输入数据
    // ==========================================
    BattleContext context;
    context.currentHp = 75;
    context.maxHp = 80;
    context.gold = 120;
    context.maxEnergy = 3;
    context.enemySeedOrId = "Slime_Squad";

    // ==========================================
    // 🃏 2. 数据驱动的卡组配置 (极其清爽！)
    // ==========================================
    QList<QString> testDeckIds = {
        "card_strike", "card_strike", "card_strike", // 3张打击
        "card_defend", "card_defend", "card_defend", // 3张防御
        "card_bash",
        "card_thunderclap",
        "card_dark_embrace",
        "card_fire_source",
        "card_bloodletting",
        "card_burning_pact",
        "card_metallicize",
        "card_inflame",
        "card_barricade",
        "card_hell_fiend",
        "card_pour"
    };

    // 自动让工厂发牌并全部升级
    for (const QString& id : testDeckIds) {
        Card* newCard = CardFactory::createCard(id, &a);
        if (newCard) {
            newCard->upgrade(); // 锻造升级
            context.currentDeck.append(newCard);
        }
    }

    // ==========================================
    // 🎒 3. 数据驱动的遗物配置
    // ==========================================
    QList<QString> testRelicIds = {
        "relic_pen_nib",
        "relic_orichalcum",
        "relic_bag_of_preparation",
        "relic_anchor"
    };

    for (const QString& id : testRelicIds) {
        Relic* newRelic = RelicFactory::createRelic(id, &a);
        if (newRelic) {
            context.relics.append(newRelic);
        }
    }

    // ==========================================
    // 4. 发射战斗系统！
    // ==========================================
    BattleLauncher launcher;
    QObject::connect(&launcher, &BattleLauncher::battleConcluded, [](BattleResult result) {
        qDebug() << "[External System] Battle Finished! Victory:" << result.isVictory;
    });

    launcher.launch(context);
    return a.exec();
}