#pragma once
#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>
#include <QRandomGenerator>

// 🔴 引入强类型节点枚举
#include "map/MapManager.h"

// 🔴 引入所有拥有独立大脑的怪物实体！
#include "enemies/SlimeLarge.h"
#include "enemies/SlimeSmall.h"
#include "enemies/MadGremlin.h"
#include "enemies/JawWorm.h" // 你的新怪！
#include "enemies/FatGremlin.h"
#include "enemies/ShieldGremlin.h"
#include "enemies/SneakyGremlin.h"
#include "enemies/WizardGremlin.h"
#include "enemies/GremlinLeader.h"
#include "enemies/Cultist.h"
#include "enemies/Hexaghost.h"

class EnemyFactory {
public:
    // ========================================================
    // 🎭 摇号与统筹车间
    // ========================================================
    static inline QList<Enemy*> createEncounter(NodeType nodeType, int currentLayer) {
        QString encounterId;

        if (nodeType == NodeType::Boss) {
            QStringList bossPool = {"Hexaghost_Encounter"};
            encounterId = bossPool[QRandomGenerator::global()->bounded(bossPool.size())];
        }
        else if (nodeType == NodeType::Elite) {
            QStringList elitePool = {"Slime_Squad","Gremlin_Leader_Encounter"}; // 把大哥加进精英池！
            encounterId = elitePool[QRandomGenerator::global()->bounded(elitePool.size())];
        }
        else {
            // 🎲 普怪池：把 Jaw_Worm 加进来了！
            QStringList monsterPool = {"Single_Slime", "Single_Cultist", "Single_Jaw_Worm"};
            encounterId = monsterPool[QRandomGenerator::global()->bounded(monsterPool.size())];
        }

        qDebug() << "[EnemyFactory] 🎲 节点类型:" << static_cast<int>(nodeType) << "-> 摇中图纸:" << encounterId;

        // 🛠️ 送入造兵车间，按图纸组装肉体
        QList<Enemy*> squad = buildSquad(encounterId);

        // 💉 强化车间：层数动态补正
        for (Enemy* e : squad) {
            int hpBuff = currentLayer * 0.2;
            e->setMaxHp(e->getMaxHp() + hpBuff);
            e->setHp(e->getMaxHp());
        }

        return squad;
    }

public:
    // ========================================================
    // 🛠️ 造兵车间 (图纸拼装)
    // 🔴 核心修改：全面应用 1, 2, 3, 0 的视觉平衡占位逻辑！
    // ========================================================
    static inline QList<Enemy*> buildSquad(const QString& encounterId) {
        QList<Enemy*> squad;

        if (encounterId == "Slime_Squad") {
            // 3只怪物：占据 1, 2, 3 号位（视觉重心居中偏右，避开最左边的 0 号边缘位）
            Enemy* leftSmall = createEnemy("Slime_Small");   leftSmall->setSlotIndex(1);
            Enemy* centerBoss = createEnemy("Slime_01");     centerBoss->setSlotIndex(2);
            Enemy* rightSmall = createEnemy("Slime_Small");  rightSmall->setSlotIndex(3);
            squad << leftSmall << centerBoss << rightSmall;
        }
        else if (encounterId == "Single_Slime") {
            // 单只怪物：绝对的 C 位（1 号位，也就是视觉上的 2 号中心位）
            Enemy* boss = createEnemy("Slime_01");
            boss->setSlotIndex(2);
            boss->setMaxHp(boss->getMaxHp()*0.75);
            boss->setHp(boss->getMaxHp());
            squad << boss;
        }
        else if (encounterId == "Mad_Gremlin_Gang") {
            // 两只怪物：分列中央两侧（1 号位 和 2 号位）
            Enemy* g1 = createEnemy("Mad_Gremlin"); g1->setSlotIndex(1);
            Enemy* g2 = createEnemy("Mad_Gremlin"); g2->setSlotIndex(2);
            squad << g1 << g2;
        }
        else if (encounterId == "Slime_Boss") {
            // Boss战：毫无疑问的 C 位！
            Enemy* bigBoss = createEnemy("Slime_01");
            bigBoss->setMaxHp(150); bigBoss->setHp(150); // Boss 级血量覆写
            bigBoss->setSlotIndex(2);
            squad << bigBoss;
        }
        else if (encounterId == "Single_Jaw_Worm") {
            // 大颚虫单挑：稳坐 C 位！
            Enemy* jawWorm = createEnemy("Jaw_Worm");
            jawWorm->setSlotIndex(2);
            squad << jawWorm;
        }// 🔴 新增：精英战 —— 地精领袖与他的随从们！
        else if (encounterId == "Gremlin_Leader_Encounter") {
            // 老大绝对的 C 位！(Slot 1，画面正中心)
            Enemy* leader = createEnemy("Gremlin_Leader");
            leader->setSlotIndex(3);
            squad << leader;

            // 开局自带两只随机小地精，分列左右！(Slot 0 和 Slot 2)
            Enemy* minion1 = createEnemy("Random_Gremlin"); minion1->setSlotIndex(1);
            Enemy* minion2 = createEnemy("Random_Gremlin"); minion2->setSlotIndex(2);
            squad << minion1 << minion2;
        }
        else if (encounterId == "Single_Cultist") {
            Enemy* cultist = createEnemy("Cultist");
            cultist->setSlotIndex(2); // 稳坐屏幕 C 位
            squad << cultist;
        }else if (encounterId == "Hexaghost_Encounter") {
            Enemy* hexaghost = createEnemy("Hexaghost");
            hexaghost->setSlotIndex(2); // 绝对的压迫感，独占 2号 C位！
            squad << hexaghost;
        }

        return squad;
    }

    // ========================================================
    // 🧬 细胞车间 (彻底摆脱硬编码逻辑，纯粹的实例化！)
    // ========================================================
    static inline Enemy* createEnemy(const QString& enemyId) {
        if (enemyId == "Slime_01")    return new SlimeLarge();
        if (enemyId == "Slime_Small") return new SlimeSmall();
        if (enemyId == "Mad_Gremlin") return new MadGremlin();
        if (enemyId == "Jaw_Worm")    return new JawWorm();
        if (enemyId == "Mad_Gremlin")    return new MadGremlin();
        if (enemyId == "Fat_Gremlin")    return new FatGremlin();
        if (enemyId == "Shield_Gremlin") return new ShieldGremlin();
        if (enemyId == "Sneaky_Gremlin") return new SneakyGremlin();
        if (enemyId == "Wizard_Gremlin") return new WizardGremlin();
        if (enemyId == "Gremlin_Leader") return new GremlinLeader();
        if (enemyId == "Cultist") return new Cultist();
        if (enemyId == "Hexaghost") return new Hexaghost();

        // ========================================================
        // 🎁 盲盒解析系统：当收到 "Random_Gremlin" 指令时，随机吐出一只！
        // ========================================================
        if (enemyId == "Random_Gremlin") {
            int roll = QRandomGenerator::global()->bounded(5);
            switch (roll) {
            case 0: return new MadGremlin();
            case 1: return new FatGremlin();
            case 2: return new ShieldGremlin();
            case 3: return new SneakyGremlin();
            case 4: return new WizardGremlin();
            }
        }

        // 兜底防崩溃
        return new Enemy("未知的野生黑泥怪", 10);
    }
};