#pragma once
#include "Card.h"
#include "../Player.h"
#include "../relics/RelicManager.h"
#include <QDebug>

class BloodlettingCard : public Card {
    Q_OBJECT
public:
    explicit BloodlettingCard(QObject* parent = nullptr)
        : Card("card_bloodletting", QStringLiteral("放血"), 0, false, parent) {

        // 🔴 失去的 HP 永远是 3，直接写进文本；只有能量是动态的！
        m_secondaryValue = 2;  // 获得 2 能量

        // 🟢 智能模板注入：保留 \n 换行，!M! 托管动态能量！
        m_rawDescription = QStringLiteral("失去 3 点生命值。\n获得 !M! 点能量。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/bloodletting.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 🔴 升级直接 +1，文案全自动更新！
            qDebug() << "[Card]" << m_name << "升级完毕！当前回费：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);
        if (source) {
            source->loseHp(3); // 固定扣 3 血
            source->addEnergy(m_secondaryValue); // 动态加能量
            qDebug() << "[Card Engine] 打出放血：玩家自残 3 HP，获得" << m_secondaryValue << "能量喵！";
        }
    }
};