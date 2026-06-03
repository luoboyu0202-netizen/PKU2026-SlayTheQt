#pragma once
#include "Enemy.h"
#include <QRandomGenerator>

class JawWorm : public Enemy {
    Q_OBJECT
private:
    // 給這隻怪物的專屬技能編個號，方便寫入記憶體！
    enum Move { CHOMP = 0, BELLOW = 1, THRASH = 2 };

public:
    explicit JawWorm(QObject* parent = nullptr)
        : Enemy("大颚虫", 25, ":/resources/images/enemies/jaw_worm.png", parent) {
        setId("Jaw_Worm");
    }

    // ========================================================
    // 🧠 核心重寫：大顎蟲的權重隨機決策樹
    // ========================================================
    void rollNextIntent()  {
        // 1. 首回合固定邏輯 (歷史記錄還是空的)
        if (m_moveHistory[0] == -1) {
            setIntent(Move::CHOMP, Intent(IntentType::Attack, 11));
            return;
        }

        // 2. 啟動搖號機！(0 ~ 99)
        int roll = QRandomGenerator::global()->bounded(100);

        // 3. 根據權重和歷史記憶，決策下一步！
        // 🐾 45% 機率：咆哮 (加護甲、加力量) - 不能連放 2 次
        if (roll < 45 && !lastMoveWas(Move::BELLOW)) {
            setIntent(Move::BELLOW, Intent(IntentType::DefendAndBuff, 6, StatusType::Strength, 3));
        }
        // 🐾 30% 機率 (即 45~74)：橫衝直撞 (攻擊+護甲) - 不能連放 3 次
        else if (roll < 75 && !lastTwoMovesWere(Move::THRASH)) {
            // 🔴 這裡用到了我們剛剛新加的 AttackAndDefend 複合意圖！
            // value = 7 是傷害，statusValue = 5 作為護甲值傳遞 (需要你引擎裡對接一下喵)
            setIntent(Move::THRASH, Intent(IntentType::AttackAndDefend, 7, StatusType::None, 5));
        }
        // 🐾 25% 機率 (即 75~99)：咬 (純攻擊) - 不能連放 2 次
        else if (!lastMoveWas(Move::CHOMP)) {
            setIntent(Move::CHOMP, Intent(IntentType::Attack, 11));
        }
        else {
            // ========================================================
            // 🛡️ 兜底機制：如果隨機到的技能剛好觸發了「防連續限制」，
            // 直接重新搖號！直到搖出合法技能為止！
            // ========================================================
            rollNextIntent();
        }
    }

private:
    // 幫助函數：設定意圖並同步寫入大腦記憶
    void setIntent(int moveId, Intent intent) {
        m_currentIntent = intent;
        recordMove(moveId);
    }
};
