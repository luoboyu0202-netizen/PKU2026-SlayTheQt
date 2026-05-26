#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"
#include "relics/RelicManager.h"
#include <QTimer>
#include "logic/battleengine.h"

class RelicManager; // 🟢 只需要这句！告诉编译器“有这么个东西存在”就行了！

class PummelCard : public Card {
    Q_OBJECT
public:
    explicit PummelCard(QObject* parent = nullptr)
        : Card("card_pummel", QStringLiteral("连续拳"), 1, false, parent) {

        m_baseValue = 2;
        m_secondaryValue = 4;

        // 🔴 智能模板注入：!D! 管伤害，!M! 管次数
        m_rawDescription = QStringLiteral("消耗 。造成 !D! 点伤害 !M! 次。");
        m_description = m_rawDescription;

        m_type = CardType::Attack;
        m_target = CardTarget::Enemy;
        m_exhaustOnUse = true;
        m_imagePath = ":/resources/images/cards/pummel.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 仅提升打击次数
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !source || !target) return;

        // 📸 拍照！让引擎用大一统公式算出最准确的一拳伤害
        int snapshotDamage = engine->calculateSnapshotDamage(source, target, 2);

        int hitCount = m_isUpgraded ? 5 : 4;
        int delay = 200;

        for (int i = 0; i < hitCount; ++i) {
            // 🔫 开枪！把拍好的伤害塞进子弹里
            QTimer::singleShot(i * delay, target, [target, snapshotDamage]() {
                if (target && !target->isDead()) {
                    // 🔴 注意看第三步：这里的 takeDamage 应该只负责扣护甲和血！
                    target->takeDamage(snapshotDamage);
                }
            });
        }
    }
};