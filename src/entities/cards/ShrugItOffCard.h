#pragma once
#include "Card.h"
#include "../Player.h"
#include "BattleEngine.h"
#include "../StatusManager.h"

class ShrugItOffCard : public Card {
    Q_OBJECT
public:
    explicit ShrugItOffCard(QObject* parent = nullptr)
        : Card("card_shrug_it_off", QStringLiteral("耸肩无视"), 1, false, parent) {

        m_baseValue = 8;
        m_secondaryValue = 1;

        // 🔴 智能模板注入：!B! 代表格挡，!M! 代表抽牌数
        m_rawDescription = QStringLiteral("获得 !B! 点格挡。抽 !M! 张牌。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/shrug_it_off.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue += 3;
            // 🟢 同样，文本更新完全删掉！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target);
        int finalBlock = StatusManager::calculateBlock(source, m_baseValue);
        source->addBlock(finalBlock);

        if (BattleEngine::getInstance() && BattleEngine::getInstance()->getCardManager()) {
            BattleEngine::getInstance()->getCardManager()->drawCards(m_secondaryValue);
        }
    }
};
