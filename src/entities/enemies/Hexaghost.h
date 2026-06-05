#pragma once
#include "../../entities/Enemy.h"
#include "../../logic/BattleEngine.h"

class Hexaghost : public Enemy {
    Q_OBJECT
private:
    int m_turnCount = 0;
    int m_flameCount = 0;
    bool m_hasUsedInferno = false; // 🔴 终极狂暴标记：是否放过地狱火了？

public:
    explicit Hexaghost(QObject* parent = nullptr)
        : Enemy("六火亡魂", 250, ":/resources/images/enemies/hexaghost.png", parent) {
        setId("Hexaghost");
        setScaleFactor(1.5);
    }

    void onBattleStart() override {
        m_turnCount = 0;
        m_flameCount = 0;
        m_hasUsedInferno = false; // 开局重置
        getStatusManager()->clearStatus(StatusType::HexaLevel);
    }

    void rollNextIntent() override {
        if (m_turnCount == 0) {
            m_currentIntent = Intent(IntentType::Summon, 0, StatusType::None, 0, "", "Slime_Small");
            m_flameCount = 0;
        }
        else if (m_turnCount == 1) {
            int dividerDamage = 2; // 默认保底伤害（防空指针）

            // ========================================================
            // 🔴 动态伤害计算：连接战斗引擎，读取主角当前生命值！
            // ========================================================
            BattleEngine* engine = BattleEngine::getInstance();
            if (engine && engine->getPlayer()) {
                int playerHp = engine->getPlayer()->getHp();
                // 完美还原原版公式：(当前血量 / 12) + 1
                dividerDamage = (playerHp / 12) + 1;

                qDebug() << "[Enemy] 👻 六火亡魂凝视着主角的" << playerHp << "点生命值，生成了单发" << dividerDamage << "点的 6 段伤害！";
            }

            // 生成 6 段连击意图
            m_currentIntent = Intent(IntentType::Attack, dividerDamage, StatusType::None, 0, "", "", 6);
            m_flameCount = 1;
        }
        else {
            // ========================================================
            // 🔴 核心修复：原版的循环是 6 步（不是 7 步）！
            // ========================================================
            int cycle = (m_turnCount - 2) % 6;

            // 根据是否狂暴，决定塞入的灼伤数量！
            int burnAmount = m_hasUsedInferno ? 2 : 1;

            if (cycle == 5) {
                // 🌋 第 6 步 (索引 5) 必定是地狱火：2x6 伤害，直接往弃牌堆塞 3 张灼伤！
                m_currentIntent = Intent(IntentType::AttackAndInsertStatus, 2, StatusType::None, 3, "card_burn", "", 6);
                m_flameCount = 6;
                m_hasUsedInferno = true; // 开启狂暴模式！
            } else {
                switch (cycle) {
                case 0: // Sear
                case 2: // Sear
                    // 🔥 灼烧：造成 6 点伤害，并洗入灼伤牌！
                    m_currentIntent = Intent(IntentType::AttackAndInsertStatus, 6, StatusType::None, burnAmount, "card_burn");
                    break;
                case 1: // Tackle
                case 4: // Tackle
                    m_currentIntent = Intent(IntentType::Attack, 5, StatusType::None, 0, "", "", 2);
                    break;
                case 3: // Inflame
                    m_currentIntent = Intent(IntentType::DefendAndBuff, 12, StatusType::Strength, 2);
                    break;
                }

                // 🔴 判定阈值提前：Turn 8 就进入第二轮循环了！
                if (m_turnCount < 8) {
                    // 第一阶段：火焰从 2 逐渐涨到 6
                    m_flameCount = cycle + 2;
                } else {
                    // 第二阶段以后：火焰从 1 涨到 5 (因为第6步固定是地狱火的6)
                    m_flameCount = cycle + 1;
                }
            }
        }

        getStatusManager()->clearStatus(StatusType::HexaLevel);
        if (m_flameCount > 0) {
            getStatusManager()->applyStatus(StatusType::HexaLevel, m_flameCount);
        }

        m_turnCount++;
        Enemy::rollNextIntent();
    }
    // 恢复清白，不要在这里乱灭火！
    void clearIntent()  {
        Enemy::clearIntent();
    }

    // 🟢 真正的防诈尸结界：只有死亡的瞬间才会执行！
    void die() override {
        // 先灭火！
        if (getStatusManager()) {
            getStatusManager()->clearStatus(StatusType::HexaLevel);
            qDebug() << "[Enemy] 👻 六火亡魂被击杀！地狱火彻底熄灭！";
        }
        // 然后呼叫基类的死亡逻辑，安详去世
        Enemy::die();
    }
};