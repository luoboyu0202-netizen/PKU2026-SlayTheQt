#pragma once
#include "Card.h"
#include "BattleEngine.h"
#include <QDebug>

class DoubtCard : public Card {
public:
    explicit DoubtCard(QObject* parent = nullptr)
        : Card("card_doubt", QStringLiteral("疑虑"), -1, false, parent) {
        
        m_rawDescription = QStringLiteral("不能打出。\n在你的回合结束时，获得 1 层 虚弱 。");
        m_description = m_rawDescription;
        m_type = CardType::Curse;
        m_target = CardTarget::None;
        m_rarity = CardRarity::Special;
        m_isUnplayable = true;
        m_imagePath = ":/resources/images/cards/doubt.png";
    }

    void play(Player*, Fighter*, RelicManager*) override {}

    void triggerOnEndOfTurn() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->getPlayer()) {
            qDebug() << "[Card] 疑虑发作！玩家获得 1 层虚弱喵！";
            engine->getPlayer()->getStatusManager()->applyStatus(StatusType::Weak, 1);
        }
    }
};
