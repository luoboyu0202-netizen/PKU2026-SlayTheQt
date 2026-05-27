#pragma once
#include "Fighter.h"

class Player : public Fighter {
    Q_OBJECT

public:
    explicit Player(const QString& name, int maxHp, int maxEnergy, int startingGold, QObject* parent = nullptr);

    // Getter
    int getEnergy() const { return m_energy; }
    int getMaxEnergy() const { return m_maxEnergy; }
    int getGold() const { return m_gold; }
    int getMaxHp() const { return m_maxHp;    }

    // 玩家专属操作接口
    bool useEnergy(int amount); // 返回 false 表示费用不足
    void resetEnergy();         // 回合开始时重置费用
    void modifyGold(int amount); // 正数为获得，负数为被偷取/消费

    // ========================================================
    // 🔴【新增插座】：增加能量接口（专门为放血、双重能量等卡牌准备喵！）
    // ========================================================
    void addEnergy(int amount);
    // ========================================================
    // 🔴【喵军师防坑预警】：真实伤害自残接口
    // ========================================================
    // 【放血】是“失去生命”，不能用普通的 takeDamage（会扣格挡）。
    // 如果你的 Fighter 类里没有 loseHp，建议在这里或者 Fighter 里补上一个！
    void loseHp(int amount);

signals:
    void energyChanged(int currentEnergy, int maxEnergy);
    void goldChanged(int currentGold);

private:
    int m_energy;
    int m_maxEnergy;
    int m_gold;
};
