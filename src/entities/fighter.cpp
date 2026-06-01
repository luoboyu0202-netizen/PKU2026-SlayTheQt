#include "Fighter.h"
#include <algorithm>
#include <QDebug> // 记得引入 QDebug

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

    // 剩余伤害扣除血量
    if (amount > 0) {
        m_hp -= amount;
        if (m_hp <= 0) {
            m_hp = 0;
            die();
        }

        qDebug() << "[Combat]" << m_name << "took" << amount << "damage. Current HP:" << m_hp;
        emit hpChanged(m_hp, m_maxHp);
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