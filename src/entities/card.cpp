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
        m_id += "+";   // 🔴 极其关键：身份证号加 +，给存档和工厂看的！
    }
}

QString Card::getDynamicDescription(Player* source, Fighter* target) {
    QString finalDesc = m_rawDescription;

    // ========================================================
    // 🛡️【核心断网判定】：只有传入了活生生的 source（玩家），才算处于真实战斗！
    // 火堆里 source 为 nullptr，直接无痛进入安全模式！
    // ========================================================
    bool isInCombat = (source != nullptr);

    // ⚔️ 1. 解析伤害标签 !D!
    if (finalDesc.contains("!D!")) {
        int currentDmg = m_baseValue; // 默认基础伤害

        // 🔴 只有战斗中才去查引擎，绝不跨界！
        if (isInCombat) {
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
        int currentBlock = m_baseValue; // 默认基础格挡

        // 🔴 同理，战斗中才算 Buff
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

    // 🟢 极其老实地直接 return，绝不动你的 \n 和原本的架构！
    return finalDesc;
}