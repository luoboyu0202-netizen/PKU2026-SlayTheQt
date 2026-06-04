#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>
#include <QDebug>

class MadGremlin : public Enemy {
    Q_OBJECT
public:
    explicit MadGremlin(QObject* parent = nullptr)
        // 血量随机在 20 到 24 之间
        : Enemy("疯狂地精", QRandomGenerator::global()->bounded(8, 11), ":/resources/images/enemies/mad_gremlin.png", parent) {
        setId("Mad_Gremlin");
    }
    // ========================================================
    // 🟢 重写入场技能：此时 UI 已全部连接完毕，尽情广播吧！
    // ========================================================
    // ========================================================
    // 😡 开局被动：愤怒！
    // 确保状态管家 (StatusManager) 在基类构造时已经就绪
    // ========================================================
    void onBattleStart() override {
        int initialAngry = QRandomGenerator::global()->bounded(1, 3); // 1 或 2 层愤怒

        qDebug() << "[🔬 Diagnostic - Monster] MadGremlin::onBattleStart() 被成功触发！";
        qDebug() << "[🔬 Diagnostic - Monster] 准备赋予愤怒层数:" << initialAngry;

        getStatusManager()->applyStatus(StatusType::Angry, initialAngry);

        qDebug() << "[🔬 Diagnostic - Monster] 赋予后，管家里实际愤怒层数:"
                 << getStatusManager()->getStatus(StatusType::Angry);
    }

    void rollNextIntent() override {
        m_currentIntent = Intent(IntentType::Attack, 4);
        Enemy::rollNextIntent();
    }

};
