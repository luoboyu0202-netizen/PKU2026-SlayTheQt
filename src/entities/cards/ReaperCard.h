#pragma once
#include "Card.h"
#include "../Player.h"
#include "BattleEngine.h"
#include "../StatusManager.h"
#include <algorithm>
#include <QDebug>

class ReaperCard : public Card {
    Q_OBJECT
public:
    explicit ReaperCard(QObject* parent = nullptr)
        : Card("card_reaper", QStringLiteral("死亡收割"), 2, false, parent) {

        m_baseValue = 4; // 基础AOE伤害

        // 🔴 智能模板注入：完美吃满主角力量加成的 !D! 标签
        m_rawDescription = QStringLiteral("消耗。对所有敌人造成 !D! 点伤害。回复等同于未被格挡伤害的生命值。");
        m_description = m_rawDescription;

        m_type = CardType::Attack;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/reaper.png";
        m_exhaustOnUse = true;
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue = 5; // 🔴 升级只改基础数值！
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target);
        auto engine = BattleEngine::getInstance();
        if (!engine) return;

        const QList<Enemy*>& enemies = engine->getEnemies();
        int totalHeal = 0;

        for (Enemy* enemy : enemies) {
            if (enemy && !enemy->isDead()) {
                // 完美读取 m_baseValue 计算最终受加成伤害
                int finalDmg = StatusManager::calculateDamage(source, enemy, m_baseValue);
                if (relics) finalDmg = relics->modifyAttackDamage(finalDmg);

                int oldHp = enemy->getHp();
                enemy->takeDamage(finalDmg);
                int unblockedDamage = oldHp - enemy->getHp();

                if (unblockedDamage > 0) {
                    totalHeal += unblockedDamage;
                }
            }
        }

        if (totalHeal > 0) {
            source->heal(totalHeal);
            qDebug() << "[Logic] 死亡收割饮血狂欢！总共吸血：" << totalHeal << "点！当前血量：" << source->getHp();
        }
    }
};
