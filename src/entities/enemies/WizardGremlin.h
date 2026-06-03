#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class WizardGremlin : public Enemy {
    Q_OBJECT
public:
    explicit WizardGremlin(QObject* parent = nullptr)
        : Enemy("地精巫师", QRandomGenerator::global()->bounded(15, 17), ":/resources/images/enemies/wizard_gremlin.png", parent) {
        setId("Wizard_Gremlin");
    }

    void rollNextIntent() override {
        // 🧠 巫师大脑：蓄力机制！2回合积攒力量，第3回合释放毁灭性打击！
        if (m_charge < 2) {
            // 前两回合，意图显示为 Buff 强化自己
            m_currentIntent = Intent(IntentType::Buff, 1, StatusType::Strength);
            m_charge++;
        } else {
            // 第三回合，释放终极魔法！(加上之前攒的 2 点力量，实际伤害会达到 27！)
            m_currentIntent = Intent(IntentType::Attack, 25);
            m_charge = 0; // 释放完后由于疲惫，重新开始读条
        }
        Enemy::rollNextIntent();
    }
private:
    int m_charge = 0; // 专属的读条计数器！
};
