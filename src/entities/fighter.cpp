#include "Fighter.h"
#include <algorithm>
#include <QDebug> // 记得引入 QDebug
#include "../logic/BattleEngine.h" // 🔴 必须引入战斗引擎，用来顺藤摸瓜找到遗物
#include "../entities/relics/RelicManager.h" // 确保能调用遗物管理器

Fighter::Fighter(const QString& name, int maxHp, QObject* parent)
    : QObject(parent), m_name(name), m_hp(maxHp), m_maxHp(maxHp), m_block(0), m_isDead(false) {
    m_statusManager = new StatusManager(this);
}

void Fighter::takeDamage(int amount) {
    if (m_isDead || amount <= 0) return;

    // 优先扣除护甲
    if (m_block > 0) {
        if (m_block >= amount) {
            m_block -= amount;
            amount = 0;
        } else {
            amount -= m_block;
            m_block = 0;
        }
        emit blockChanged(m_block);
    }

    // ========================================================
    // ⛩️ 鸟居结界拦截点：剩余伤害扣除血量前
    // ========================================================
    if (amount > 0) {
        // 判定当前挨打的是不是玩家主角
        if (dynamic_cast<Player*>(this)) {
            BattleEngine* engine = BattleEngine::getInstance();
            if (engine) {
                // 🔴 直接呼叫我们刚刚在引擎里架设的代理插座
                amount = engine->modifyIncomingDamage(amount);
            }
        }

        // 剩余血量扣除核心判定
        m_hp -= amount;
        if (m_hp <= 0) {
            m_hp = 0;
            die();
        }

        qDebug() << "[Combat]" << m_name << "took" << amount << "damage. Current HP:" << m_hp;
        emit hpChanged(m_hp, m_maxHp);
    }

    // ========================================================
    // 😡【状态系统钩子：受击触发 (On Attacked)】
    // ========================================================
    // 检查身上是否有“愤怒”状态
    if (m_statusManager) {
        int angryStacks = m_statusManager->getStatus(StatusType::Angry);
        if (angryStacks > 0) {
            // 只要挨打，立刻增加等同于愤怒层数的力量！
            m_statusManager->applyStatus(StatusType::Strength, angryStacks);
            qDebug() << "[Fighter] 😡" << m_name << "愤怒了！获得了" << angryStacks << "点力量！";

            // 🔴 魔法共鸣：
            // applyStatus 会自动发射 statusChanged 信号！
            // EnemyItem 监听到后会自动触发 update()！
            // 我们的 paint() 渲染管线会自动读取这层新的力量，并瞬间放大头顶的伤害数字和刀的图标！
        }
    }

}

void Fighter::addBlock(int amount) {
    if (m_isDead || amount <= 0) return;
    m_block += amount;
    emit blockChanged(m_block);
}

void Fighter::loseBlock() {
    if (m_block > 0) {
        m_block = 0;
        emit blockChanged(m_block);
    }
}

void Fighter::heal(int amount) {
    if (m_isDead || amount <= 0) return;
    m_hp = std::min(m_hp + amount, m_maxHp);
    emit hpChanged(m_hp, m_maxHp);
}

void Fighter::die() {
    if (m_isDead) return;
    m_isDead = true;
    m_hp = 0;

    qDebug() << "[Combat]" << m_name << "has died!";
    emit hpChanged(m_hp, m_maxHp);
    emit died(this);
}

void Fighter::setHp(int hp) {
    if (m_isDead) return; // 已经阵亡的角色不能被强制设置血量

    // 🔴 核心防御：确保血量绝对不会小于 0，也绝对不会超过上限
    m_hp = std::max(0, std::min(hp, m_maxHp));

    // 触发 UI 血条刷新信号
    emit hpChanged(m_hp, m_maxHp);

    // 🔴 致命判定：如果强制设置血量导致血量归零，立刻触发死亡结算！
    if (m_hp == 0) {
        die();
    }
}

void Fighter::setMaxHp(int maxHp) {
    m_maxHp = std::max(1, maxHp);
    // 如果当前血量超过了新上限，则同步扣减当前血量
    if (m_hp > m_maxHp) {
        m_hp = m_maxHp;
    }
    emit hpChanged(m_hp, m_maxHp);
}