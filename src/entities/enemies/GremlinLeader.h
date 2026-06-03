#pragma once
#include "../../entities/Enemy.h"
#include <QRandomGenerator>

class GremlinLeader : public Enemy {
    Q_OBJECT
private:
    enum Move { RALLY = 0, ENCOURAGE = 1, STAB = 2 };

public:
    explicit GremlinLeader(QObject* parent = nullptr)
        : Enemy("地精头子", 140, ":/resources/images/enemies/gremlin_leader.png", parent) {
        setId("Gremlin_Leader");

        // 🔴 大哥必须有排面，直接放大 1.5 倍！
        setScaleFactor(1.5);
    }

    void rollNextIntent() override {
        // 首回合必定召唤小弟护驾！
        if (m_moveHistory[0] == -1) {
            setIntent(Move::RALLY, Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Random_Gremlin"));
            return;
        }

        int roll = QRandomGenerator::global()->bounded(100);

        // 🐾 33% 概率：鼓舞 (全员加 3 力量) - 不能连放
        if (roll < 33 && !lastMoveWas(Move::ENCOURAGE)) {
            // GroupBuff 意图：参数2是层数，参数3是状态类型
            setIntent(Move::ENCOURAGE, Intent(IntentType::GroupBuff, 3, StatusType::Strength));
        }
        // 🐾 33% 概率：集结 (召唤 2 只随机小地精) - 不能连放
        else if (roll < 66 && !lastMoveWas(Move::RALLY)) {
            // 🔴 核心魔法：召唤的目标填入 "Random_Gremlin"！
            setIntent(Move::RALLY, Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Random_Gremlin"));
        }
        // 🐾 34% 概率：连环刺击 (6 伤害 x 3 段) - 不能连放
        else if (!lastMoveWas(Move::STAB)) {
            // Attack 意图，最后一个参数是 multiHitCount (多段攻击次数)
            setIntent(Move::STAB, Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));
        }
        else {
            // 兜底重摇
            rollNextIntent();
        }
    }

private:
    // 帮助函数：设定意图并同步写入大脑记忆
    void setIntent(int moveId, Intent intent) {
        m_currentIntent = intent;
        recordMove(moveId);
        Enemy::rollNextIntent(); // 🔴 算完后呼叫基类发射 UI 刷新信号
    }
};
