#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>

class OrnamentalFanRelic : public Relic {
    Q_OBJECT
public:
    explicit OrnamentalFanRelic(QObject* parent = nullptr)
        : Relic("relic_ornamental_fan",
                QStringLiteral("装饰用扇子"),
                QStringLiteral("在同一回合内，你每打出 3 张 攻击 牌，获得 4 点 格挡 。"),
                parent) {
        m_counter = 0; // 回合计数器初始化
    }

    // 🌙 新回合开始：强行清零
    void onTurnStart() override {
        setCounter(0);
    }

    // 🃏 监听打牌：满 3 张就加甲！
    void onCardPlayed(Card* card) override {
        if (!card) return;

        if (card->getType() == CardType::Attack) {
            int nextCount = getCounter() + 1;

            if (nextCount == 3) {
                qDebug() << "[Relic] 🪭 装饰用扇子 展开！行云流水，获得 4 点格挡！";

                emit relicActivated();

                BattleEngine* engine = BattleEngine::getInstance();
                if (engine && engine->getPlayer()) {
                    engine->getPlayer()->addBlock(4);
                }

                setCounter(0); // 触发后清零，允许一回合内多次扇风！
            } else {
                setCounter(nextCount);
            }
        }
    }
};
