#pragma once
#include "Card.h"
#include "BattleEngine.h"
#include <QDebug>

class BurnCard : public Card {
    Q_OBJECT
public:
    explicit BurnCard(QObject* parent = nullptr)
        : Card("card_burn", QStringLiteral("灼伤"), -1, false, parent) {

        // 🔴 状态牌伤害属于“魔法数字”，统一放进 secondaryValue！
        m_secondaryValue = 2;

        // 🟢 智能模板注入
        m_rawDescription = QStringLiteral("不能打出。\n在你的回合结束时，受到 !M! 点伤害。");
        m_description = m_rawDescription;

        m_type = CardType::Status;
        m_target = CardTarget::None;
        m_isUnplayable = true;

        m_imagePath = ":/resources/images/cards/burn.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 2; // 🔴 伤害变为 4
            qDebug() << "[Card]" << m_name << "升级完毕！当前回合结束扣血：" << m_secondaryValue;
        }
    }

    void play(Player*, Fighter*, RelicManager*) override {} // 不可打出

    void triggerOnEndOfTurn() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->getPlayer()) {
            qDebug() << "[Logic] 灼伤发作！玩家受到" << m_secondaryValue << "点伤害喵！";

            // 🔴 直接读取 m_secondaryValue 造成真实伤害
            engine->getPlayer()->takeDamage(m_secondaryValue);
        }
    }
};
