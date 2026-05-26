#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"

class HellFiendCard : public Card {
    Q_OBJECT
public:
    explicit HellFiendCard(QObject* parent = nullptr)
        : Card("card_hell_fiend", QStringLiteral("地狱狂徒"), 2, false, parent) {

        // 🔴 同样收编：状态层数交给魔法数字！
        m_secondaryValue = 1;

        // 文本中没有数字变化，所以不需要 !M!
        m_rawDescription = QStringLiteral("每当你抽到一张名字中含有“打击”的牌时，立即将其打出。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/hell_fiend.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_cost = 1; // 纯纯的减费升级，同样不需要动任何字符串！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);
        if (source) {
            // 🔴 拒绝写死常数，直接读取 m_secondaryValue！
            source->getStatusManager()->applyStatus(StatusType::HellFiend, m_secondaryValue);
        }
    }
};
