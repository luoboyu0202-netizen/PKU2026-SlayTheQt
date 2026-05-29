#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class DarkEmbraceCard : public Card {
    Q_OBJECT
public:
    explicit DarkEmbraceCard(QObject* parent = nullptr)
        : Card("card_dark_embrace", QStringLiteral("黑暗之拥"), 2, false, parent) {

        // 🔴 抽牌数是魔法数字，交给副数值！
        m_secondaryValue = 1;

        // 🟢 智能模板注入：完美接管 !M!
        m_rawDescription = QStringLiteral("每当有一张牌被消耗时，抽 !M! 张牌。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/darkembrace.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();

            // 🔴 纯净的机制进化：费用直接变 1，完全不用管那该死的文本拼接啦！
            m_cost = 1;
            qDebug() << "[Card]" << m_name << "升级完毕！当前费用：" << m_cost;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);

        if (source) {
            // 读取 m_secondaryValue 挂载状态
            source->getStatusManager()->applyStatus(StatusType::DarkEmbrace, m_secondaryValue);
            qDebug() << "[Logic]" << source->getName() << "拥抱了虚空！获得了" << m_secondaryValue << "层黑暗之拥状态喵！";
        }
    }
};
