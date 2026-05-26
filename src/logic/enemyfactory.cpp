#pragma once
#include <QString>
#include "../entities/Enemy.h"

class EnemyFactory {
public:
    // 核心工厂加工厂方法
    static Enemy* createEnemy(const QString& enemyId) {
        if (enemyId == "Slime_01") {
            // 如果是酸液小史莱姆
            Enemy* slime = new Enemy(QStringLiteral("酸液小史莱姆"), 30);

            // 为其注入标志性的行动意图序列
            QList<Intent> ai;
            ai.append({IntentType::Attack, 8});
            ai.append({IntentType::Defend, 5});
            slime->setIntentSequence(ai);

            return slime;
        }
        else if (enemyId == "Mad_Gremlin") {
            // 如果是疯狂地精
            Enemy* gremlin = new Enemy(QStringLiteral("疯狂地精"), 24);
            QList<Intent> ai;
            ai.append({IntentType::Attack, 12}); // 攻击极高！
            enemy->setIntentSequence(ai);
            return gremlin;
        }

        // 兜底防御性代码，防止拼写错误导致游戏崩溃
        return new Enemy(QStringLiteral("未知的野生黑泥怪"), 10);
    }
};
