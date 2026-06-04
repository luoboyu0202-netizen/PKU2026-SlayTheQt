#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class FatGremlin : public Enemy {
    Q_OBJECT
public:
    explicit FatGremlin(QObject* parent = nullptr)
        : Enemy("胖地精", QRandomGenerator::global()->bounded(9, 13), ":/resources/images/enemies/fat_gremlin.png", parent) {
        setId("Fat_Gremlin");
    }

    void rollNextIntent() override {
        // 🧠 胖地精大脑：永远打 4 并附带 1 层虚弱！
        m_currentIntent = Intent(IntentType::AttackAndDebuff, 4, StatusType::Weak, 2);
        Enemy::rollNextIntent();
    }
};
