#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class ShieldGremlin : public Enemy {
    Q_OBJECT
public:
    explicit ShieldGremlin(QObject* parent = nullptr)
        : Enemy("盾地精", QRandomGenerator::global()->bounded(13, 14), ":/resources/images/enemies/shield_gremlin.png", parent) {
        setId("Shield_Gremlin");
    }

    void rollNextIntent() override {
        // 🧠 盾地精大脑：保护同伴！使用群体护甲！
        m_currentIntent = Intent(IntentType::GroupDefend, 7);
        Enemy::rollNextIntent();
    }
};
