#include "entities/cards/Card.h"
#include "logic/BattleEngine.h" // 🔴 在这里包含，因为我们已经在 .cpp 里了，很安全！
#include <QDebug>

Card::Card(const QString& id, const QString& name, int cost, bool isEthereal, QObject* parent)
    : QObject(parent), m_id(id), m_name(name), m_cost(cost), m_isEthereal(isEthereal),
    m_description(""), m_type(CardType::Attack), m_target(CardTarget::None) {}

void Card::upgrade() {
    if (!m_isUpgraded) {
        m_isUpgraded = true;
        m_name += "+";
    }
}

QString Card::getDynamicDescription(Player* source, Fighter* target) {
    QString finalDesc = m_rawDescription;

    // ⚔️ 1. 解析伤害标签 !D!
    if (finalDesc.contains("!D!")) {
        int currentDmg = m_baseValue;
        if (source) {
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
        if (source) {
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