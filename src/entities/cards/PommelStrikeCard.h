#pragma once
#include "Card.h"
#include "../Player.h"
#include "BattleEngine.h"
#include "../StatusManager.h"

class PommelStrikeCard : public Card {
    Q_OBJECT
public:
    explicit PommelStrikeCard(QObject* parent = nullptr)
        : Card("card_pommel_strike", QStringLiteral("剑柄打击"), 1, false, parent) {

        m_baseValue = 9;
        m_secondaryValue = 1;

        // 🔴 智能模板注入：!D! 代表伤害，!M! 代表抽牌数
        m_rawDescription = QStringLiteral("造成 !D! 点伤害。抽 !M! 张牌。");
        m_description = m_rawDescription; // 保底初始化

        m_type = CardType::Attack;
        m_target = CardTarget::Enemy;
        m_imagePath = ":/resources/images/cards/pommel_strike.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue += 1;
            m_secondaryValue += 1;
            // 🟢 极其舒爽：这里什么文本都不用改了！解析器全自动接管！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        if (target) {
            int finalDamage = StatusManager::calculateDamage(source, target, m_baseValue);
            if (relics) finalDamage = relics->modifyAttackDamage(finalDamage);
            target->takeDamage(finalDamage);

            if (BattleEngine::getInstance() && BattleEngine::getInstance()->getCardManager()) {
                BattleEngine::getInstance()->getCardManager()->drawCards(m_secondaryValue);
            }
        }
    }
};
