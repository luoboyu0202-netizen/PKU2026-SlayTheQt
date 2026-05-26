#include "Player.h"
#include <QDebug>

Player::Player(const QString& name, int maxHp, int maxEnergy, int startingGold, QObject* parent)
    : Fighter(name, maxHp, parent), m_energy(maxEnergy), m_maxEnergy(maxEnergy), m_gold(startingGold) {
}

bool Player::useEnergy(int amount) {
    if (m_energy >= amount) {
        m_energy -= amount;
        emit energyChanged(m_energy, m_maxEnergy);
        return true;
    }
    return false;
}

void Player::resetEnergy() {
    m_energy = m_maxEnergy;
    emit energyChanged(m_energy, m_maxEnergy);
}

void Player::modifyGold(int amount) {
    m_gold += amount;
    if (m_gold < 0) m_gold = 0; // 金币不能为负
    emit goldChanged(m_gold);
}

// 🟢【实现充能逻辑】
void Player::addEnergy(int amount) {
    if (amount > 0) {
        m_energy += amount;

        // 🔴 呼叫 UI 更新！注意：费用可以超过上限，这在尖塔里是完全合法的喵！
        emit energyChanged(m_energy, m_maxEnergy);

        qDebug() << "[Player] 能量激增！当前能量：" << m_energy << "/" << m_maxEnergy;
    }
}

// 🟢【实现自残逻辑（无视格挡，直接扣血）】
void Player::loseHp(int amount) {
    if (amount > 0 && !isDead()) {
        m_hp -= amount;
        if (m_hp < 0) m_hp = 0;

        // 🔴 记得发射血量变化的信号，让底下的红色血条跟着掉血喵！
        // (假设你在 Fighter 里有类似 hpChanged 的信号，没有的话换成你实际更新 UI 的信号)
        emit hpChanged(m_hp, m_maxHp);

        qDebug() << "[Player] 失去生命！当前生命：" << m_hp << "/" << m_maxHp;

        if (m_hp <= 0) {
            die(); // 触发死亡逻辑
        }
    }
}
