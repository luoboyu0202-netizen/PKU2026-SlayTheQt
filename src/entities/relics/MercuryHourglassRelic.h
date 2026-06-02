#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../../entities/Enemy.h" // 确保引入了怪物类
#include <QDebug>

class MercuryHourglassRelic : public Relic {
    Q_OBJECT
public:
    explicit MercuryHourglassRelic(QObject* parent = nullptr)
        : Relic("relic_mercury_hourglass",
                QStringLiteral("水银沙漏"),
                QStringLiteral("在你的回合结束时，对全体敌人造成 3 点伤害。"),
                parent) {
        m_counter = -1;
    }

    // ========================================================
    // 🌙 钩子：回合结束时发动全场横扫
    // ========================================================
    void onTurnEnd() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine) return;

        // 假设你的 BattleEngine 有 getEnemies() 接口返回当前战场上的活怪 QList<Enemy*>
        QList<Enemy*> aliveEnemies = engine->getEnemies();
        if (aliveEnemies.isEmpty()) return;

        qDebug() << "[Relic] ⏳ 水银沙漏 漏完！对全场" << aliveEnemies.size() << "个敌人造成 3 点真实伤害！";

        emit relicActivated();

        for (Enemy* enemy : aliveEnemies) {
            if (enemy) {
                // 呼叫怪物的受击/扣血接口（注意：有些架构里如果是真实伤害可以直接 enemy->loseHp(3)）
                // 这里用标准的 takeDamage 或根据你的具体战斗函数微调喵
                enemy->takeDamage(3);
            }
        }
    }
};
