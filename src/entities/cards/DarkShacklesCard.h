#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"

class DarkShacklesCard : public Card {
    Q_OBJECT
public:
    explicit DarkShacklesCard(QObject* parent = nullptr)
        : Card("card_dark_shackles", QStringLiteral("黑暗镣铐"), 0, false, parent) {

        // 🔴 状态层数统一使用副数值（Magic Number）
        m_secondaryValue = 9;

        // 智能模板注入：!M! 完美填入镣铐层数
        m_rawDescription = QStringLiteral("消耗 。 敌人 在本回合 失去 !M! 点 力量 。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::Enemy;
        m_exhaustOnUse = true;
        m_imagePath = ":/resources/images/cards/dark_shackles.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue = 15; // 升级直接修改副数值！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(source); Q_UNUSED(relics);
        if (target) {
            // 使用 m_secondaryValue 挂载负面效果！
            target->getStatusManager()->applyStatus(StatusType::Strength, -m_secondaryValue);
            target->getStatusManager()->applyStatus(StatusType::Shackled, m_secondaryValue);
        }
    }
};
