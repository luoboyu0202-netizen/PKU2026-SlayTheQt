#pragma once
#include <QObject>
#include <QMap>

class Fighter;

// 1. 统一定义所有可能的状态枚举
enum class StatusType {
    Strength,   // 力量：增加攻击伤害
    Dexterity,  // 敏捷：增加获得护甲
    Weak,       // 虚弱：造成的攻击伤害减少 25%
    Vulnerable,
    Frail,        // 易伤：受到的攻击伤害增加 50%
    // ========================================================
    // 🔴【全新加入】：三大时序能力状态！
    // ========================================================
    Metallicize,  // 金属化：回合结束加格挡
    FireSource,   // 薪火之源：回合开始加能量
    DarkEmbrace,   // 黑暗之拥：消耗卡牌时抽牌
    GainStrength, // 🔴 恢复力量（回合结束时，将此层数转化为力量，并清空自身）
    Barricade,
    Shackled,// 🔴 镣铐：回合结束时恢复等量的力量，并解除此状态
    HellFiend,
    Confusion,
    Angry,
    Ritual,  // 仪式：在你的回合结束时，获得特定层数的力量！
    None
    // 未来可以无限扩展：中毒、荆棘、再生...
};

class StatusManager : public QObject {
    Q_OBJECT
public:
    explicit StatusManager(QObject* parent = nullptr) : QObject(parent) {}

    static int calculateDamage(Fighter* source, Fighter* target, int baseDamage);
    static int calculateBlock(Fighter* source, int baseBlock);

    // 🔴 核心接口：添加/减少状态层数
    void applyStatus(StatusType type, int amount);
    void decreaseStatus(StatusType type, int amount);
    void clearStatus(StatusType type);
    int getStatus(StatusType type) const;

    // 回合结束时，自动递减那些“按回合衰减”的Debuff（比如虚弱、易伤）
    void tickEndOfTurnStatuses();

signals:
    // 通知 UI 层更新状态图标！
    void statusChanged(StatusType type, int newAmount);


private:
    // 用一个极其轻量级的哈希表来存储当前生效的状态和层数
    QMap<StatusType, int> m_statuses;

};
