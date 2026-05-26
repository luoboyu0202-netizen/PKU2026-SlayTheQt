#include "Enemy.h"
#include <QDebug>

// 🔴 构造函数初始化列表里，把 imagePath 存下来喵！
Enemy::Enemy(const QString& name, int maxHp, const QString& imagePath, QObject* parent)
    : Fighter(name, maxHp, parent), m_currentIntentIndex(0), m_imagePath(imagePath) {
    // 🔴 幽灵变量 m_currentIntent 和多余的 m_sequenceIndex 已经被彻底清除！
}

void Enemy::setIntentSequence(const QList<Intent>& sequence) {
    m_intentSequence = sequence;
    m_currentIntentIndex = 0; // 战斗刚开始，稳稳锁定在第 0 个意图

    // 🔴 修复跳帧 Bug：
    // 刚刚注入序列时，不需要“翻页(roll)”，只需要直接把第 0 个意图广播给 UI 即可！
    if (!m_intentSequence.isEmpty()) {
        Intent firstIntent = getCurrentIntent();
        emit intentChanged(firstIntent.type, firstIntent.value);
    }
}

void Enemy::rollNextIntent() {
    if (!m_intentSequence.isEmpty()) {
        // 🔴 只有在回合真正结束时，引擎大脑才会调用这里，拨动齿轮走向下一步
        m_currentIntentIndex = (m_currentIntentIndex + 1) % m_intentSequence.size();

        Intent nextIntent = getCurrentIntent();
        emit intentChanged(nextIntent.type, nextIntent.value);
    }
}
// ==========================================================
// 🔴【新增】：护甲逻辑实现
// ==========================================================

void Enemy::addBlock(int b) {
    if (b > 0) {
        // m_block 是继承自 Fighter 的变量（如果报错说找不到，去 Fighter.h 里把它改成 protected: int m_block; 即可喵）
        m_block += b;

        qDebug() << "[Enemy]" << m_name << "gained" << b << "block! Total:" << m_block;

        // 核心：发射信号，让 UI 层的盾牌图标立刻画出来！
        emit blockChanged(m_block);
    }
}

void Enemy::loseBlock() {
    if (m_block > 0) {
        m_block = 0;

        qDebug() << "[Enemy]" << m_name << "lost all block at the start of turn.";

        // 核心：发射信号，让 UI 层的盾牌图标消失！
        emit blockChanged(m_block);
    }
}