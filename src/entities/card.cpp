#include "entities/cards/Card.h"
#include "logic/BattleEngine.h" // 🔴 再次接通与前线指挥部的通讯！
#include <QDebug>

QString Card::getDynamicDescription(Player* source, Fighter* target) {
    QString finalDesc = m_rawDescription;

    bool isInCombat = (source != nullptr);

    // ⚔️ 1. 解析伤害标签 !D!
    if (finalDesc.contains("!D!")) {
        int currentDmg = m_baseValue;

        if (isInCombat) {
            // ========================================================
            // 🔴 终极恢复：完美调用引擎的快照计算！钢笔尖起死回生！
            // ========================================================
            BattleEngine* engine = BattleEngine::getInstance();
            if (engine) {
                currentDmg = engine->calculateSnapshotDamage(source, target, m_baseValue);
            } else {
                currentDmg = StatusManager::calculateDamage(source, target, m_baseValue);
            }
        }

        QString dmgStr = QString::number(currentDmg);
        if (currentDmg > m_baseValue) {
            dmgStr = QString("<font color='#7fff00'>%1</font>").arg(currentDmg);
        } else if (currentDmg < m_baseValue) {
            dmgStr = QString("<font color='#ff6563'>%1</font>").arg(currentDmg);
        }
        finalDesc.replace("!D!", dmgStr);
    }

    // 🛡️ 2. 解析格挡标签 !B!
    if (finalDesc.contains("!B!")) {
        int currentBlock = m_baseValue;

        if (isInCombat) {
            currentBlock = StatusManager::calculateBlock(source, m_baseValue);
        }

        QString blockStr = QString::number(currentBlock);
        if (currentBlock > m_baseValue) {
            blockStr = QString("<font color='#7fff00'>%1</font>").arg(currentBlock);
        } else if (currentBlock < m_baseValue) {
            blockStr = QString("<font color='#ff6563'>%1</font>").arg(currentBlock);
        }
        finalDesc.replace("!B!", blockStr);
    }

    // 🌟 3. 解析副数值标签 !M!
    if (finalDesc.contains("!M!")) {
        finalDesc.replace("!M!", QString::number(m_secondaryValue));
    }

    return finalDesc;
}