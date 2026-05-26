#pragma once
#include "Card.h"
#include "../Player.h"
#include "../relics/RelicManager.h"
#include "../StatusManager.h"
#include <QDebug>

class DefendCard : public Card {
    Q_OBJECT
public:
    explicit DefendCard(QObject* parent = nullptr)
        : Card("card_defend", QStringLiteral("防御"), 1, false, parent) {

        m_baseValue = 5;

        // 🟢 智能模板注入：极其纯正的 !B! 标签！吃敏捷，吃虚弱（如果虚弱减防的话）！
        m_rawDescription = QStringLiteral("获得 !B! 点 格挡 。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::Player; // 或者 CardTarget::None 都可以喵
        m_imagePath = ":/resources/images/cards/defend.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_baseValue += 3; // 🔴 升级直接 +3，文案全自动更新！
            qDebug() << "[Card]" << m_name << "升级完毕！当前格挡：" << m_baseValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target);

        if (source) {
            // 完美走状态管道和遗物管道，一刀不剪
            int finalBlock = StatusManager::calculateBlock(source, m_baseValue);
            if (relics) {
                finalBlock = relics->modifyBlock(finalBlock);
            }
            source->addBlock(finalBlock);
        }
    }
};
