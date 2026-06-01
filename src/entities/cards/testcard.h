#pragma once
#include "Card.h"
#include "../Player.h"
#include "../Enemy.h"
#include "../../logic/BattleEngine.h"
#include "../StatusManager.h"
#include <QDebug>

class testcard : public Card {
    Q_OBJECT
public:
    explicit testcard(QObject* parent = nullptr)
        : Card("card_test", QStringLiteral("測試卡"), 1, false, parent) {

        m_baseValue = 1000;       // 伤害
        m_secondaryValue = 1;  // 易伤层数

        // 🟢 智能模板注入：!D! 和 !M! 的完美双打！
        m_rawDescription = QStringLiteral("对全体敌人造成 !D! 点伤害。\n给予全体敌人 !M! 层易伤。");
        m_description = m_rawDescription;

        m_type = CardType::Attack;
        m_target = CardTarget::AllEnemies;
        m_imagePath = ":/resources/images/cards/thunderclap.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();

            // 🔴 极致精简：只升级伤害！
            m_baseValue += 3;

            qDebug() << "[Card]" << m_name << "升级完毕！当前群体伤害：" << m_baseValue << "，群体易伤：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target);

        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine) return;

        const QList<Enemy*>& enemies = engine->getEnemies();
        for (Enemy* enemy : enemies) {
            if (!enemy || enemy->isDead()) continue;

            // 算伤，打人，挂易伤
            int finalDamage = StatusManager::calculateDamage(source, enemy, m_baseValue);
            if (relics) {
                finalDamage = relics->modifyAttackDamage(finalDamage);
            }

            enemy->takeDamage(finalDamage);
            enemy->getStatusManager()->applyStatus(StatusType::Vulnerable, m_secondaryValue);
        }

        qDebug() << "[Card Engine] ⚡ 闪电霹雳！全场敌人遭遇雷霆轰顶！";
    }
};
