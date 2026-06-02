#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class ShurikenRelic : public Relic {
    Q_OBJECT
public:
    explicit ShurikenRelic(QObject* parent = nullptr)
        : Relic("relic_shuriken",
                QStringLiteral("手里剑"),
                QStringLiteral("在同一回合内，你每打出 3 张 攻击 牌，获得 1 点 力量 。"),
                parent) {
        m_counter = 0; // 初始化回合内攻击计数为 0
    }

    // ========================================================
    // 🌙 钩子 1：新回合开始，必须重置回合内的连击计数！
    // ========================================================
    void onTurnStart(){
        setCounter(0);
    }

    // ========================================================
    // 🃏 钩子 2：监听打牌，每满 3 张攻击牌就发功
    // ========================================================
    void onCardPlayed(Card* card) override {
        if (!card) return;

        if (card->getType() == CardType::Attack) {
            int nextCount = getCounter() + 1;

            if (nextCount == 3) {
                qDebug() << "[Relic] 🥷 手里剑旋转！触发 3 连击，注入 1 点力量！";

                emit relicActivated(); // 触发 UI 闪烁

                BattleEngine* engine = BattleEngine::getInstance();
                if (engine && engine->getPlayer() && engine->getPlayer()->getStatusManager()) {
                    engine->getPlayer()->getStatusManager()->applyStatus(StatusType::Strength, 1);
                }

                setCounter(0); // 触发后计数归零，允许在一回合内多次触发（比如单回合打出6张攻击牌给2点力量）
            } else {
                setCounter(nextCount);
            }
        }
    }
};
