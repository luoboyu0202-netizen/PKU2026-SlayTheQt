#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class FireSourceCard : public Card {
    Q_OBJECT
public:
    explicit FireSourceCard(QObject* parent = nullptr)
        : Card("card_fire_source", QStringLiteral("薪火之源"), 2, false, parent) {

        // 🔴 能量获取属于非战斗缩放的魔法数字，交给 m_secondaryValue
        m_secondaryValue = 1;

        // 🟢 智能模板注入：!M! 完美填入
        m_rawDescription = QStringLiteral("在你的回合开始时，获得 !M! 点能量。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/firesource.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 🔴 就这一句话，升级就做完了！太舒服了喵！
            qDebug() << "[Card]" << m_name << "升级完毕！当前每回合回费：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);

        if (source) {
            source->getStatusManager()->applyStatus(StatusType::FireSource, m_secondaryValue);
            qDebug() << "[Logic]" << source->getName() << "点燃了无尽的薪火！获得了" << m_secondaryValue << "层薪火之源状态喵！";
        }
    }
};