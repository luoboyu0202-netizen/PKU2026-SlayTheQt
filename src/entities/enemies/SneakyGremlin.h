#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class SneakyGremlin : public Enemy {
    Q_OBJECT
public:
    explicit SneakyGremlin(QObject* parent = nullptr)
        : Enemy("狡诈地精", QRandomGenerator::global()->bounded(10, 12), ":/resources/images/enemies/sneaky_gremlin.png", parent) {
        setId("Sneaky_Gremlin");
    }

    void rollNextIntent() override {
        // 🧠 狡诈地精大脑：简单粗暴，高额伤害
        m_currentIntent = Intent(IntentType::Attack, QRandomGenerator::global()->bounded(9, 11));
        Enemy::rollNextIntent();
    }
};
