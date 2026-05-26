#pragma once
#include "Card.h"
#include "Player.h"
#include "StatusManager.h"
#include <QDebug>

class MetallicizeCard : public Card {
    Q_OBJECT
public:
    explicit MetallicizeCard(QObject* parent = nullptr)
        : Card("card_metallicize", QStringLiteral("金属化"), 1, false, parent) {

        // 🔴 金属化层数交给副数值！
        m_secondaryValue = 3;

        // 🟢 智能模板注入：!M! 托管层数
        m_rawDescription = QStringLiteral("在你的回合结束时，获得 !M! 点格挡。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/metallicize.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 🔴 升级层数变 4
            qDebug() << "[Card]" << m_name << "升级完毕！当前每回合护甲：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);

        if (source) {
            // 🔴 读取 m_secondaryValue 挂载状态
            source->getStatusManager()->applyStatus(StatusType::Metallicize, m_secondaryValue);
            qDebug() << "[Logic]" << source->getName() << "的皮肤化作钢铁！获得了" << m_secondaryValue << "层金属化状态喵！";
        }
    }
};