#include "StatusManager.h"
#include <QDebug>
#include "Fighter.h"

int StatusManager::calculateDamage(Fighter* source, Fighter* target, int baseDamage) {
    if (!source || !target) return baseDamage;

    int finalDamage = baseDamage;

    // 1. 结算力量（🔴 现在力量可能是负数，直接加上去就会减少基础伤害！）
    if (source && source->getStatusManager()) {
        int strength = source->getStatusManager()->getStatus(StatusType::Strength);
        finalDamage += strength;
    }

    // 🔴 极其重要：伤害保底机制！无论力量多低，基础攻击力绝对不能变成负数去给敌人回血！
    if (finalDamage < 0) {
        finalDamage = 0;
    }

    StatusManager* srcStatus = source->getStatusManager();
    StatusManager* tgtStatus = target->getStatusManager();

    // 2. 攻击方减益：虚弱 (-25% 伤害，向下取整)
    if (srcStatus->getStatus(StatusType::Weak) > 0) {
        finalDamage = static_cast<int>(finalDamage * 0.75);
    }

    // 3. 防御方减益：易伤 (+50% 伤害，向下取整)
    if (tgtStatus->getStatus(StatusType::Vulnerable) > 0) {
        finalDamage = static_cast<int>(finalDamage * 1.50);
    }

    return finalDamage > 0 ? finalDamage : 0; // 伤害不能是负数喵！
}

int StatusManager::calculateBlock(Fighter* source, int baseBlock) {
    if (!source) return baseBlock;

    int finalBlock = baseBlock;
    StatusManager* srcStatus = source->getStatusManager();

    // 1. 增益：敏捷直接增加护甲量
    finalBlock += srcStatus->getStatus(StatusType::Dexterity);

    // =======================================================
    // 🔴 2. 【全新减益】：脆弱 (-25% 护甲，向下取整)
    // =======================================================
    if (srcStatus->getStatus(StatusType::Frail) > 0) {
        // static_cast<int> 会自动舍去小数部分，完美复刻杀戮尖塔的向下取整逻辑！
        finalBlock = static_cast<int>(finalBlock * 0.75);
    }

    return finalBlock > 0 ? finalBlock : 0; // 护甲不能是负数
}

// 🟢 1. 核心查询：获取某状态的层数（就是它报的错！）
int StatusManager::getStatus(StatusType type) const {
    // Qt 的 QMap 非常优雅，用 value() 查字典，如果找不到这个状态，直接安全返回 0 层！
    return m_statuses.value(type, 0);
}

void StatusManager::applyStatus(StatusType type, int amount) {
    // 🔴【核心修复】：允许负数通过！只有当增量为 0 时才拦截！
    if (amount == 0) return;

    m_statuses[type] += amount;

    // 🔴 看看到底是谁在加力量！
    if (type == StatusType::Strength) {
        qDebug() << "[Trace] applyStatus 被触发！增量：" << amount << " 累加前总计：" << m_statuses[type];
    }

    // 下方原本的保底和移除逻辑保持完全不变喵！
    if (type != StatusType::Strength && type != StatusType::Dexterity) {
        if (m_statuses[type] < 0) {
            m_statuses[type] = 0;
        }
    }

    if (m_statuses[type] == 0) {
        m_statuses.remove(type);
    }

    emit statusChanged(type, m_statuses[type]);
}

// 🟢 3. 减少状态层数（比如回合结束，虚弱减少 1 层）
void StatusManager::decreaseStatus(StatusType type, int amount) {
    if (!m_statuses.contains(type)) return; // 根本没这病，不用治

    m_statuses[type] -= amount;

    if (m_statuses[type] <= 0) {
        // 层数扣光了，直接把这个状态从背包里彻底扔掉！
        m_statuses.remove(type);
        emit statusChanged(type, 0);
    } else {
        emit statusChanged(type, m_statuses[type]);
    }
}

// 🟢 4. 彻底净化：直接清空某种状态（比如用了人工制品，或者清 Debuff 的牌）
void StatusManager::clearStatus(StatusType type) {
    if (m_statuses.remove(type) > 0) {
        emit statusChanged(type, 0);
    }
}

// 🟢 5. 回合结束自动衰减系统
void StatusManager::tickEndOfTurnStatuses() {
    // 在 STS 中，力量和敏捷通常不衰减，但虚弱和易伤每回合结束都要减 1
    // 如果以后有【中毒】，还要在这里写扣血的逻辑喵！
    decreaseStatus(StatusType::Weak, 1);
    decreaseStatus(StatusType::Vulnerable, 1);
    decreaseStatus(StatusType::Frail, 1);

    // ========================================================
    // ⛓️【时序钩子：镣铐解除（Shackled）】
    // ========================================================
    if (m_statuses.contains(StatusType::Shackled) && m_statuses[StatusType::Shackled] > 0) {
        int shackledAmount = m_statuses[StatusType::Shackled];

        // 1. 把之前被扣掉的力量还回去
        applyStatus(StatusType::Strength, shackledAmount);
        qDebug() << "[Status] 镣铐解除！目标恢复了" << shackledAmount << "点力量喵！";

        // 2. 镣铐是一次性的，用完当场粉碎！
        clearStatus(StatusType::Shackled);
    }

    if (m_statuses.contains(StatusType::Ritual) && m_statuses[StatusType::Ritual] > 0) {
        int ritualAmount = m_statuses[StatusType::Ritual];

        // 咔咔！力量暴涨！
        applyStatus(StatusType::Strength, ritualAmount);
        qDebug() << "[Status] 咔咔！仪式生效！目标获得了" << ritualAmount << "点力量喵！";
    }

}


