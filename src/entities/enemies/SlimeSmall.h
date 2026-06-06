#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class SlimeSmall : public Enemy {
    Q_OBJECT
public:
    explicit SlimeSmall(QObject* parent = nullptr)
        : Enemy("酸液小史莱姆", 12, ":/resources/images/enemies/slime_small.png", parent) {
        setId("Slime_Small");
    }

    void rollNextIntent() override {
        // 🧠 大脑逻辑：防止连续做同一件事
        if (m_moveHistory[0] == 0) {
            // 上回合攻击了，这回合必定虚弱 (1~2层)
            int weakAmount = QRandomGenerator::global()->bounded(1, 3);
            m_currentIntent = Intent(IntentType::Debuff, weakAmount, StatusType::Weak);
            recordMove(1); // 记录动作：1 代表上虚弱
        } else {
            // 第一回合，或上回合刚上完虚弱，这回合必定攻击 (7~8点)
            int dmg = QRandomGenerator::global()->bounded(4, 6);
            m_currentIntent = Intent(IntentType::Attack, dmg);
            recordMove(0); // 记录动作：0 代表攻击
        }

        Enemy::rollNextIntent();
    }
};
