#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class Cultist : public Enemy {
    Q_OBJECT
public:
    explicit Cultist(QObject* parent = nullptr)
        : Enemy("邪教徒", QRandomGenerator::global()->bounded(48, 55), ":/resources/images/enemies/cultist.png", parent) {
        setId("Cultist");
        setScaleFactor(1.3);
    }

    void rollNextIntent() override {
        // ========================================================
        // 🔴 修复：直接判断数组第一个记录是不是初始值 -1 即可！
        // ========================================================
        if (m_moveHistory[0] == -1) {
            // 第一回合：必定使用“觉醒 (Incantation)”！
            // 给自己上 3 层仪式
            m_currentIntent = Intent(IntentType::Buff, 6, StatusType::Ritual);
            recordMove(0);
        } else {
            // 之后的每一回合：必定使用“黑暗打击 (Dark Strike)”！
            m_currentIntent = Intent(IntentType::Attack, 0);
            recordMove(1);
        }

        Enemy::rollNextIntent();
    }
};
