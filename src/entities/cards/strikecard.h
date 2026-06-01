#pragma once
#include "Card.h"
#include "../Player.h"
#include "../relics/RelicManager.h"
#include "../StatusManager.h"
#include <QDebug>

class StrikeCard : public Card {
    Q_OBJECT
public:
    explicit StrikeCard(QObject* parent = nullptr)
        : Card("card_strike", QStringLiteral("打击"), 1, false, parent) {

        m_baseValue = 6;

        // 🟢 智能模板注入：最纯正的 !D! 标签！
        m_rawDescription = QStringLiteral("造成 !D! 点伤害。");
        m_description = m_rawDescription;

        m_type = CardType::Attack;
        m_target = CardTarget::Enemy;
        m_rarity = CardRarity::Starter;
        m_imagePath = ":/resources/images/cards/strike.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();

            // 🔴 极致精简：升级直接修改基础值！
            m_baseValue += 3;

            qDebug() << "[Card]" << m_name << "升级完毕！当前伤害：" << m_baseValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        if (target) {
            // 走伤害计算管道
            int finalDamage = StatusManager::calculateDamage(source, target, m_baseValue);

            if (relics) {
                finalDamage = relics->modifyAttackDamage(finalDamage);
            }

            target->takeDamage(finalDamage);
        }
    }
};