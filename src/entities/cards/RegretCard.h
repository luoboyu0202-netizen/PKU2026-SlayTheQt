#pragma once
#include "Card.h"
#include "BattleEngine.h"
#include <QDebug>

class RegretCard : public Card {
public:
    explicit RegretCard(QObject* parent = nullptr)
        : Card("card_regret", QStringLiteral("悔恨"), -1, false, parent) {
        
        m_rawDescription = QStringLiteral("不能打出。\n在你的回合结束时，手牌中每有一张牌，失去 1 点生命值。");
        m_description = m_rawDescription;
        m_type = CardType::Curse;
        m_target = CardTarget::None;
        m_rarity = CardRarity::Special;
        m_isUnplayable = true;
        m_imagePath = ":/resources/images/cards/regret.png";
    }

    void play(Player*, Fighter*, RelicManager*) override {}

    void triggerOnEndOfTurn() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->getPlayer() && engine->getCardManager()) {
            int handCount = engine->getCardManager()->getHandCount();
            if (handCount > 0) {
                qDebug() << "[Card] 悔恨发作！手牌数:" << handCount << "，玩家失去等量生命喵！";
                // 悔恨是失去生命值，绕过护甲
                int currentHp = engine->getPlayer()->getHp();
                engine->getPlayer()->setHp(currentHp - handCount);
            }
        }
    }
};
