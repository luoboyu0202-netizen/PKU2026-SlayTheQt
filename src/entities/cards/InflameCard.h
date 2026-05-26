#pragma once
#include "Card.h"
#include "BattleEngine.h"
#include <QDebug>

class InflameCard : public Card {
    Q_OBJECT
public:
    explicit InflameCard(QObject* parent = nullptr)
        : Card("card_inflame", QStringLiteral("燃烧"), 1, false, parent) {

        // 🔴 状态层数是不吃属性加成的“魔法数字”，交给副数值！
        m_secondaryValue = 2;

        // 🟢 智能模板注入：!M! 托管力量层数
        m_rawDescription = QStringLiteral("获得 !M! 点力量。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/inflame.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 🔴 升级力量变 3
            qDebug() << "[Card]" << m_name << "升级完毕！当前提供力量：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);

        if (source) {
            // 🔴 看看 play 到底被谁呼叫了，呼叫了几次！
            qDebug() << "[Trace] 燃烧 Card::play 被触发！准备给予力量：" << m_secondaryValue;

            source->getStatusManager()->applyStatus(StatusType::Strength, m_secondaryValue);
        }
    }
};
