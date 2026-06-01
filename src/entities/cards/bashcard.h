#pragma once
#include "Card.h"
#include "../Player.h"
#include "../relics/RelicManager.h"
#include "../StatusManager.h"
#include <QDebug> // 记得包含 QDebug 打印日志喵

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
        m_rarity = CardRarity::Starter;
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
        if (target) {
            // 🔴【核心纠错 1】：读取主数值 (m_baseValue) 作为基础伤害！
            int finalDamage = StatusManager::calculateDamage(source, target, m_baseValue);

            // 走遗物管道：计算特定遗物的修饰
            if (relics) {
                finalDamage = relics->modifyAttackDamage(finalDamage);
            }

            // 狠狠砸下去！
            target->takeDamage(finalDamage);

            // =======================================================
            // 🔴【核心纠错 2】：读取副数值 (m_secondaryValue) 作为易伤层数！
            // =======================================================
            if (target->getStatusManager()) {
                target->getStatusManager()->applyStatus(StatusType::Vulnerable, m_secondaryValue);
            }
        }
    }
};