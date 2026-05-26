#pragma once
#include "Card.h"
#include "../Player.h"
#include "../../logic/BattleEngine.h"
#include <QDebug>
#include <QRandomGenerator>

class BurningPactCard : public Card {
    Q_OBJECT
public:
    explicit BurningPactCard(QObject* parent = nullptr)
        : Card("card_burning_pact", QStringLiteral("燃烧契约"), 1, false, parent) {

        // 🔴 消耗数量固定为 1（写死），抽牌数量交给魔法数字！
        m_secondaryValue = 3;  // 抽 3 张

        // 🟢 智能模板注入
        m_rawDescription = QStringLiteral("消耗 1 张手牌。\n抽 !M! 张牌。");
        m_description = m_rawDescription;

        m_type = CardType::Skill;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/burning_pact.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_secondaryValue += 1; // 🔴 抽牌数 +1
            qDebug() << "[Card]" << m_name << "升级完毕！当前抽牌：" << m_secondaryValue;
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(source); Q_UNUSED(target); Q_UNUSED(relics);

        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getCardManager()) return;

        QList<Card*> handCards = engine->getCardManager()->getHand();

        // 1. 过滤掉自己（虽然自动打出时它根本不在手里，但在普通打出时有用）
        QList<Card*> validTargets;
        for (Card* c : handCards) {
            if (c != this) validTargets.append(c);
        }

        // 2. 如果手里真的一张其他牌都没了，按你之前的设定，直接白嫖抽牌
        if (validTargets.isEmpty()) {
            qDebug() << "[Card Engine] 燃烧契约无牌可献祭，直接触发白嫖抽牌喵！";
            engine->getCardManager()->drawCards(m_secondaryValue);
            return;
        }

        // ========================================================
        // 🤖 3. 【自动驾驶分支】：随机献祭机制！
        // ========================================================
        if (engine->isAutoPlayingCard()) {
            qDebug() << "[Card Engine] 燃烧契约被自动打出！触发盲选献祭喵！";

            // 随机挑一张牌
            int randomIndex = QRandomGenerator::global()->bounded(validTargets.size());
            Card* sacrificeCard = validTargets[randomIndex];

            // 烧掉它！抽牌！
            engine->getCardManager()->exhaustCard(sacrificeCard);
            engine->getCardManager()->drawCards(m_secondaryValue);

            return; // 🔴 极其重要：直接 return！绝对不要弹 UI！
        }

        // ========================================================
        // 🖐️ 4. 【玩家手动分支】：正常的 UI 交互机制
        // ========================================================
        QString prompt = QStringLiteral("请选择 1 张手牌消耗");
        int drawCount = m_secondaryValue;

        engine->requestHandSelection(1, prompt, [engine, drawCount](QList<Card*> selectedCards) {
            if (!selectedCards.isEmpty()) {
                for (Card* sacrifice : selectedCards) {
                    engine->getCardManager()->exhaustCard(sacrifice);
                }
                engine->getCardManager()->drawCards(drawCount);
            }
        });
    }
};
