#pragma once
#include "Card.h"
#include "../Player.h"
#include "../relics/RelicManager.h"
#include "../StatusManager.h"
#include <QDebug> // 记得包含 QDebug 打印日志喵
#include "logic/battleengine.h"

class BashCard : public Card {
    Q_OBJECT
public:
    explicit BashCard(QObject* parent = nullptr)
        : Card("card_bash", QStringLiteral("痛击"), 2, false, parent) {

        m_baseValue = 8;
        m_secondaryValue = 2;

        // 🔴【优雅升级】：只需要写一次标签模板！剩下的全交给基类处理！
        m_rawDescription = QStringLiteral("造成 !D! 点伤害。给予 !M! 层 易伤 。");
        m_description = m_rawDescription; // 初始化给个保底

        m_type = CardType::Attack;
        m_target = CardTarget::Enemy;
        m_imagePath = ":/resources/images/cards/bash.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue += 2;
            m_secondaryValue += 1;

            // 🟢 注意看：这里连 m_description 都不用重写了！
            // 因为 m_rawDescription 没变，基础数值变了，基类的解析器会自动算出新文本！
            qDebug() << "[Card]" << m_name << "升级完毕！";
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        // 记得获取我们的大管家！
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !source || !target) return;

        // =======================================================
        // 🔴【新架构发威】：呼叫大管家，一次性算出带力量、易伤、钢笔尖的真实伤害！
        // （注意：不再需要手动写 relics->modify... 了，管家全包了！）
        // =======================================================
        int finalDamage = engine->calculateSnapshotDamage(source, target, m_baseValue);

        // 狠狠砸下去！(这里的 takeDamage 已经是个只负责扣血/扣护甲的纯净版了)
        target->takeDamage(finalDamage);

        // 施加易伤！
        if (target->getStatusManager()) {
            target->getStatusManager()->applyStatus(StatusType::Vulnerable, m_secondaryValue);
        }
    }
};