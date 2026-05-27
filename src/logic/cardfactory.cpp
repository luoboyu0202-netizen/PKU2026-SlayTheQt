#include "CardFactory.h"
#include <QRandomGenerator>
#include <QDebug>

// ==========================================
// 📦 严格按照 main.cpp 中的真实路径引入所有卡牌！
// ==========================================
#include "cards/StrikeCard.h"
#include "cards/DefendCard.h"
#include "cards/BashCard.h"
#include "entities/cards/BurningPactCard.h"
#include "entities/cards/BloodlettingCard.h"
#include "entities/cards/ThunderclapCard.h"
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

// ❌ 负面/状态牌也必须引入，用来精准造物（比如怪物塞牌）
#include "entities/cards/DazedCard.h"
#include "entities/cards/BurnCard.h"
#include "entities/cards/WoundCard.h"
#include "entities/cards/SlimedCard.h"


// ========================================================
// 📚 全图鉴卡池 (仅包含玩家能正常获得/随机抽出的牌)
// ========================================================
QList<QString> CardFactory::getAllAvailableCardIds() {
    // 💡 注意：这里绝对不能放 "card_wound" 或 "card_slimed" 喵！
    return {
        "card_strike",
        "card_defend",
        "card_bash",
        "card_burning_pact",
        "card_bloodletting",
        "card_thunderclap",
        "card_inflame",
        "card_metallicize",
        "card_dark_embrace",
        "card_fire_source",
        "card_pommel_strike",
        "card_shrug_it_off",
        "card_pummel",
        "card_dark_shackles",
        "card_second_wind",
        "card_reaper",
        "card_barricade",
        "card_hell_fiend",
        "card_pour"
    };
}

// ========================================================
// 🛠️ 核心锻造舱：根据 ID 实例化卡牌对象
// ========================================================
Card* CardFactory::createCard(const QString& cardId, QObject* parent) {
    // ⚔️ 基础卡牌
    if (cardId == "card_strike") return new StrikeCard(parent);
    if (cardId == "card_defend") return new DefendCard(parent);
    if (cardId == "card_bash") return new BashCard(parent);

    // 🩸 战士专属卡牌
    if (cardId == "card_burning_pact") return new BurningPactCard(parent);
    if (cardId == "card_bloodletting") return new BloodlettingCard(parent);
    if (cardId == "card_thunderclap") return new ThunderclapCard(parent);
    if (cardId == "card_inflame") return new InflameCard(parent);
    if (cardId == "card_metallicize") return new MetallicizeCard(parent);
    if (cardId == "card_dark_embrace") return new DarkEmbraceCard(parent);
    if (cardId == "card_fire_source") return new FireSourceCard(parent);
    if (cardId == "card_pommel_strike") return new PommelStrikeCard(parent);
    if (cardId == "card_shrug_it_off") return new ShrugItOffCard(parent);
    if (cardId == "card_pummel") return new PummelCard(parent);
    if (cardId == "card_dark_shackles") return new DarkShacklesCard(parent);
    if (cardId == "card_second_wind") return new SecondWindCard(parent);
    if (cardId == "card_reaper") return new ReaperCard(parent);
    if (cardId == "card_barricade") return new BarricadeCard(parent);

    // 🌪️ 自制究极卡牌
    if (cardId == "card_hell_fiend") return new HellFiendCard(parent);
    if (cardId == "card_pour") return new PourCard(parent);

    // 🤢 状态与诅咒牌（不放入随机池，但怪物可以强制调用它来塞牌）
    if (cardId == "card_dazed") return new DazedCard(parent);
    if (cardId == "card_burn") return new BurnCard(parent);
    if (cardId == "card_wound") return new WoundCard(parent);
    if (cardId == "card_slimed") return new SlimedCard(parent);

    // ========================================================
    // 🛑 终极防崩溃保底
    // ========================================================
    qWarning() << "[CardFactory] ⚠️ 警报！未知的卡牌 ID:" << cardId << "，已强制发放保底【打击】以防闪退喵！";
    return new StrikeCard(parent);
}

// ========================================================
// 🎲 虚空造物：随机抽取一张正常卡牌
// ========================================================
Card* CardFactory::generateRandomCard(QObject* parent) {
    QList<QString> pool = getAllAvailableCardIds();
    if (pool.isEmpty()) return nullptr;

    // 完美的现代 C++ 高精度随机数发生器
    int randomIndex = QRandomGenerator::global()->bounded(pool.size());
    QString randomId = pool[randomIndex];

    qDebug() << "[CardFactory] 🎲 随机抽取到卡牌 ID:" << randomId;

    return createCard(randomId, parent);
}