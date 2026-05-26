#pragma once
#include "Card.h"
#include "../Player.h"
#include "BattleEngine.h"
#include "../StatusManager.h"

class SecondWindCard : public Card {
    Q_OBJECT
public:
    explicit SecondWindCard(QObject* parent = nullptr)
        : Card("card_second_wind", QStringLiteral("重振精神"), 1, false, parent) {

        m_baseValue = 5;

        // 🔴 智能模板注入：完美衔接单张牌的格挡收益 !B!
        m_rawDescription = QStringLiteral("消耗 手牌中所有非攻击牌。每消耗一张，获得 !B! 点格挡。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/second_wind.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue = 7; // 🔴 升级只管数值，文本自动跟进！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);
        auto engine = BattleEngine::getInstance();
        if (!engine || !engine->getCardManager()) return;

        auto cardMgr = engine->getCardManager();
        QList<Card*> currentHand = cardMgr->getHand();
        int exhaustCount = 0;

        for (Card* card : currentHand) {
            if (card != this && card->getType() != CardType::Attack) {
                cardMgr->exhaustCard(card);
                exhaustCount++;
            }
        }

        if (exhaustCount > 0) {
            int singleBlock = StatusManager::calculateBlock(source, m_baseValue);
            source->addBlock(singleBlock * exhaustCount);
        }
    }
};
