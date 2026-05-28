#pragma once
#include <QString>
#include <QStringList>
#include <QList>
#include "../entities/Enemy.h"
#include <QDebug>
#include <QRandomGenerator>

class EnemyFactory {
public:
    // ========================================================
    // 🎭 摇号与统筹车间 (全新对外接口！)
    // 根据地图传来的节点类型和层数，全自动摇号并生成经过强化的军团！
    // ========================================================
    static inline QList<Enemy*> createEncounter(const QString& nodeType, int currentLayer) {
        QString encounterId;

        // 🎲 1. 盲盒摇号逻辑 (Random Pool Selection)
        if (nodeType == "Boss") {
            QStringList bossPool = {"Slime_Boss"}; // 以后可以加入 "The_Guardian", "Hexaghost" 等
            encounterId = bossPool[QRandomGenerator::global()->bounded(bossPool.size())];
        }
        else if (nodeType == "Elite") {
            QStringList elitePool = {"Gremlin_Nob", "Three_Sentries"}; // 假设有地精大块头和三柱神
            // 兜底：如果你还没写这俩，就强行让它出个狂暴地精群
            encounterId = "Mad_Gremlin_Gang";
        }
        else {
            // 普通怪池 (Monster)
            QStringList monsterPool = {"Slime_Squad", "Single_Slime", "Mad_Gremlin_Gang"};
            encounterId = monsterPool[QRandomGenerator::global()->bounded(monsterPool.size())];
        }

        qDebug() << "[EnemyFactory] 🎲 节点类型:" << nodeType << "-> 摇中图纸:" << encounterId;

        // 🛠️ 2. 送入造兵车间，按图纸组装肉体
        QList<Enemy*> squad = buildSquad(encounterId);

        // 💉 3. 强化车间：层数动态补正 (Dynamic Difficulty Scaling)
        // 让第 10 层的史莱姆比第 1 层的史莱姆血量更厚！
        for (Enemy* e : squad) {
            int hpBuff = currentLayer * 3; // 每深入一层，怪物最大生命值 +3
            e->setMaxHp(e->getMaxHp() + hpBuff);
            e->setHp(e->getMaxHp()); // 满血复活

            // 未来甚至可以在这里给怪物的 Intent 伤害增加 currentLayer * 1 的修正
        }

        return squad;
    }

public:
    // ========================================================
    // 🛠️ 造兵车间 (你以前的 createEncounter)
    // 只负责根据具体的图纸 ID，把怪摆到对应的槽位上
    // ========================================================
    static inline QList<Enemy*> buildSquad(const QString& encounterId) {
        QList<Enemy*> squad;

        if (encounterId == "Slime_Squad") {
            Enemy* leftSmall = createEnemy("Slime_Small");
            leftSmall->setSlotIndex(0);

            Enemy* centerBoss = createEnemy("Slime_01");
            centerBoss->setSlotIndex(1);

            Enemy* rightSmall = createEnemy("Slime_Small");
            rightSmall->setSlotIndex(2);

            squad.append(leftSmall);
            squad.append(centerBoss);
            squad.append(rightSmall);
        }
        else if (encounterId == "Single_Slime") {
            Enemy* boss = createEnemy("Slime_01");
            boss->setSlotIndex(0);
            squad.append(boss);
        }
        else if (encounterId == "Mad_Gremlin_Gang") {
            // 新增遭遇战图纸：两只疯狂地精
            Enemy* g1 = createEnemy("Mad_Gremlin");
            g1->setSlotIndex(0);
            Enemy* g2 = createEnemy("Mad_Gremlin");
            g2->setSlotIndex(1);
            squad.append(g1);
            squad.append(g2);
        }
        else if (encounterId == "Slime_Boss") {
            // 假装它是第一层的大 Boss 史莱姆王
            Enemy* bigBoss = createEnemy("Slime_01");
            bigBoss->setMaxHp(150); // 给点 Boss 专属特权
            bigBoss->setHp(150);
            bigBoss->setSlotIndex(1); // 坐中间
            squad.append(bigBoss);
        }

        return squad;
    }

    // ========================================================
    // 🧬 细胞车间 (你以前的 createEnemy，保持原样封装成 private)
    // ========================================================
    static inline Enemy* createEnemy(const QString& enemyId) {
        if (enemyId == "Slime_01") {
            Enemy* slime = new Enemy(QStringLiteral("酸液大史莱姆"), 80, ":/resources/images/slime_acid.png");
            slime->setId(enemyId);
            QList<Intent> ai;
            ai.append(Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Slime_Small"));
            ai.append(Intent(IntentType::InsertStatus, 5, StatusType::None, 0, "card_slimed", ""));
            ai.append(Intent(IntentType::AttackAndDebuff, 10, StatusType::Vulnerable, 3));
            ai.append(Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));
            slime->setIntentSequence(ai);
            return slime;
        }
        else if (enemyId == "Slime_Small") {
            Enemy* slime = new Enemy(QStringLiteral("酸液小史莱姆"), 12, ":/resources/images/slime_small.png");
            slime->setId(enemyId);
            QList<Intent> ai;
            ai.append(Intent(IntentType::Attack, QRandomGenerator::global()->bounded(7, 9)));
            ai.append(Intent(IntentType::Debuff, QRandomGenerator::global()->bounded(1, 2), StatusType::Weak));
            slime->setIntentSequence(ai);
            return slime;
        }
        else if (enemyId == "Mad_Gremlin") {
            Enemy* gremlin = new Enemy(QStringLiteral("疯狂地精"), 24);
            gremlin->setId(enemyId);
            QList<Intent> ai;
            ai.append(Intent(IntentType::Attack, 12));
            gremlin->setIntentSequence(ai);
            return gremlin;
        }

        return new Enemy(QStringLiteral("未知的野生黑泥怪"), 10);
    }
};