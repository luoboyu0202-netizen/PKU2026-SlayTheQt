#pragma once
#include <QString>
#include "../entities/Enemy.h"
#include <QDebug>
#include <QRandomGenerator>

class EnemyFactory {
public:
    // 核心工厂加工厂方法
    static Enemy* createEnemy(const QString& enemyId) {
        if (enemyId == "Slime_01") {
            // 如果是酸液大史莱姆
            QString slimePath = ":/resources/images/slime_acid.png";

            // 🔴 2. 打印出来，让控制台告诉我们路径有没有写错喵！
            qDebug() << "[Diagnostics - Logic] 准备制造史莱姆，给它的照片路径是:" << slimePath;

            // 🔴 3. 用这个创建好的 slimePath 变量，去替换原来硬编码的字符串！
            Enemy* slime = new Enemy(QStringLiteral("酸液大史莱姆"), 120, slimePath);

            slime->setId(enemyId);

            QList<Intent> ai;

            ai.append(Intent(IntentType::Summon, 2, StatusType::None, 0, "", "Slime_Small"));

            ai.append(Intent(IntentType::InsertStatus, 5, StatusType::None, 0, "card_slimed", ""));

            ai.append(Intent(IntentType::AttackAndDebuff, 10, StatusType::Vulnerable, 3));

            ai.append(Intent(IntentType::Attack, 6, StatusType::None, 0, "", "", 3));

            // // // 第2回合：吐酸水，给你挂上 2 层脆弱
            // // ai.append(Intent(IntentType::Debuff, 2, StatusType::Frail));

            // // 第3回合：重重咬你一口，造成 18 点伤害
            // ai.append(Intent(IntentType::Attack, 18));

            // // // 第4回合：史莱姆发怒！给自己上 2 层力量并格挡 12 点！
            // // ai.append(Intent(IntentType::DefendAndBuff, 12, StatusType::Strength, 2));

            slime->setIntentSequence(ai);

            return slime;
        }
        else if (enemyId == "Slime_Small") {
            // ⚠️ 记得去搞一张小史莱姆的图片放进 qrc 里喵！
            QString path = ":/resources/images/slime_small.png";

            // 血量只有 12，很脆！
            Enemy* slime = new Enemy(QStringLiteral("酸液小史莱姆"), 12, path);

            slime->setId(enemyId);

            QList<Intent> ai;
            // 它的 AI 很简单：第一回合撞你 5 滴血，第二回合给你上 1 层虚弱，循环！

            int randomDmg = QRandomGenerator::global()->bounded(7, 9);
            ai.append(Intent(IntentType::Attack, randomDmg));

            int randomTimes = QRandomGenerator::global()->bounded(1, 2);
            ai.append(Intent(IntentType::Debuff, randomTimes, StatusType::Weak));
            slime->setIntentSequence(ai);

            return slime;
        }
        else if (enemyId == "Mad_Gremlin") {
            // 如果是疯狂地精
            Enemy* gremlin = new Enemy(QStringLiteral("疯狂地精"), 24);

            gremlin->setId(enemyId);

            QList<Intent> ai;
            ai.append({IntentType::Attack, 12}); // 攻击极高！
            gremlin->setIntentSequence(ai);
            return gremlin;
        }

        // 兜底防御性代码，防止拼写错误导致游戏崩溃
        return new Enemy(QStringLiteral("未知的野生黑泥怪"), 10);
    }

    static QList<Enemy*> createEncounter(const QString& encounterId) {
        QList<Enemy*> squad;

        if (encounterId == "Slime_Squad") {
            // 经典配置：左边一只小史莱姆(Slot 0)，中间大史莱姆(Slot 1)，右边小史莱姆(Slot 2)
            Enemy* leftSmall = createEnemy("Slime_Small");
            leftSmall->setSlotIndex(0); // 🔴 坐 0 号位

            Enemy* centerBoss = createEnemy("Slime_01");
            centerBoss->setSlotIndex(1); // 🔴 坐 1 号位

            Enemy* rightSmall = createEnemy("Slime_Small");
            rightSmall->setSlotIndex(2); // 🔴 坐 2 号位

            squad.append(leftSmall);
            squad.append(centerBoss);
            squad.append(rightSmall);
        }
        else if (encounterId == "Single_Slime") {
            Enemy* boss = createEnemy("Slime_01");
            boss->setSlotIndex(0); // 独狼直接坐 0 号位
            squad.append(boss);
        }
        // 未来可以加：Cultist_Group (3个邪教徒) 等等...

        return squad;
    }
};
