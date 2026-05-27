#pragma once
#include "Relic.h"
#include "../logic/BattleEngine.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class VajraRelic : public Relic {
    Q_OBJECT
public:
    explicit VajraRelic(QObject* parent = nullptr)
        : Relic("relic_vajra",
                QStringLiteral("金刚杵"),
                QStringLiteral("在每场战斗开始时，获得 1 点 力量 。"),
                parent) {
    }

    void onBattleStart() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->getPlayer()) {
            qDebug() << "[Relic] 💪 金刚杵 发光！开局注入 1 点力量！";

            emit relicActivated();

            // 给玩家挂上力量状态！
            if (engine->getPlayer()->getStatusManager()) {
                engine->getPlayer()->getStatusManager()->applyStatus(StatusType::Strength, 1);
            }
        }
    }
};