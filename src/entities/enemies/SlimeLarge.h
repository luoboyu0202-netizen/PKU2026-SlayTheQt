#pragma once
#include "../../entities/Enemy.h"

class SlimeLarge : public Enemy {
    Q_OBJECT
public:
    explicit SlimeLarge(QObject* parent = nullptr)
        : Enemy("酸液大史莱姆", 68, ":/resources/images/enemies/slime_acid.png", parent) {
        setId("Slime_01");
    }

    void rollNextIntent() override {
        // 🧠 大脑逻辑：严格遵循 4 步循环！
        int turn = m_turnCounter % 4;
        switch (turn) {
        case 0:
            m_currentIntent = Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Slime_Small");
            break;
        case 1:
            m_currentIntent = Intent(IntentType::InsertStatus, 5, StatusType::None, 0, "card_slimed", "");
            break;
        case 2:
            m_currentIntent = Intent(IntentType::AttackAndDebuff, 10, StatusType::Vulnerable, 3);
            break;
        case 3:
            m_currentIntent = Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3);
            break;
        }

        m_turnCounter++;

        // 🔴 极其重要：算完意图后，呼叫基类发射 UI 刷新信号！
        Enemy::rollNextIntent();
    }

private:
    int m_turnCounter = 0; // 专属的循环计数器
};