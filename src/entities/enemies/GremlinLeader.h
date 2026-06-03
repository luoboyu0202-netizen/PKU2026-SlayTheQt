#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>
#include "../../logic/BattleEngine.h" // 🔴 必须引入天眼引擎！(请根据你的实际路径微调喵)

class GremlinLeader : public Enemy {
    Q_OBJECT
private:
    enum Move { RALLY = 0, ENCOURAGE = 1, STAB = 2 };

public:
    explicit GremlinLeader(QObject* parent = nullptr)
        : Enemy("地精头子", 50, ":/resources/images/enemies/gremlin_leader.png", parent) {
        setId("Gremlin_Leader");
        setScaleFactor(1.5); // 大哥的排面！
    }

    void rollNextIntent() override {
        // ========================================================
        // 📡 战场雷达：扫描全场，到底还有几个活着的小弟？
        // ========================================================
        int aliveMinions = 0;
        if (BattleEngine* engine = BattleEngine::getInstance()) {
            for (Enemy* e : engine->getEnemies()) {
                // 如果这个实体存在、没死，且【不是老大自己】！
                if (e && !e->isDead() && e != this) {
                    aliveMinions++;
                }
            }
        }

        // 首回合必定召唤小弟护驾！（无视场上人数）
        if (m_moveHistory[0] == -1) {
            setIntent(Move::RALLY, Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Random_Gremlin"));
            return;
        }

        // 🎲 启动摇号机
        int roll = QRandomGenerator::global()->bounded(100);

        // ========================================================
        // 🧠 进阶领袖 AI：根据场上人数，动态切换战斗风格！
        // ========================================================
        if (aliveMinions >= 3) {
            // 🔴【满员状态】：坑位满了（场上最多4怪），绝对不摇人！
            // 战术切换：50% 概率疯狂强化，50% 概率带头冲锋！
            if (roll < 50 && !lastMoveWas(Move::ENCOURAGE)) {
                setIntent(Move::ENCOURAGE, Intent(IntentType::GroupBuff, 3, StatusType::Strength));
            } else if (!lastMoveWas(Move::STAB)) {
                setIntent(Move::STAB, Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));
            } else {
                rollNextIntent(); // 兜底重摇
            }
        }
        else if (aliveMinions == 0) {
            // 🔴【光杆司令状态】：小弟死光了！放群体 Buff 毫无意义！
            // 战术切换：75% 极大概率摇人，25% 概率气急败坏捅玩家！
            if (roll < 75 && !lastMoveWas(Move::RALLY)) {
                setIntent(Move::RALLY, Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Random_Gremlin"));
            } else if (!lastMoveWas(Move::STAB)) {
                setIntent(Move::STAB, Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));
            } else {
                rollNextIntent(); // 兜底重摇
            }
        }
        else {
            // 🟢【正常状态】：场上有 1~2 只小弟，按经典的三分天下逻辑运转！
            if (roll < 33 && !lastMoveWas(Move::ENCOURAGE)) {
                setIntent(Move::ENCOURAGE, Intent(IntentType::GroupBuff, 3, StatusType::Strength));
            } else if (roll < 66 && !lastMoveWas(Move::RALLY)) {
                setIntent(Move::RALLY, Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Random_Gremlin"));
            } else if (!lastMoveWas(Move::STAB)) {
                setIntent(Move::STAB, Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));
            } else {
                rollNextIntent(); // 兜底重摇
            }
        }
    }

private:
    void setIntent(int moveId, Intent intent) {
        m_currentIntent = intent;
        recordMove(moveId);
        Enemy::rollNextIntent();
    }
};