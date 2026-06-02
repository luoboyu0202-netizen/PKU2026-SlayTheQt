#pragma once
#include "Relic.h"
#include "../Player.h"
#include <QDebug>

class MeatOnTheBoneRelic : public Relic {
    Q_OBJECT
public:
    explicit MeatOnTheBoneRelic(QObject* parent = nullptr)
        : Relic("relic_meat_on_the_bone",
                QStringLiteral("带肉骨头"),
                QStringLiteral("如果在战斗结束时你的生命值在 50% 或以下，回复 12 点 生命 。"),
                parent) {
        m_counter = -1;
    }

    // 🏆 战斗胜利结算拦截
    void onBattleEnd(Player* player) override {
        if (!player) return;

        // 核心判定：当前血量 <= 最大血量的一半
        if (player->getHp() <= (player->getMaxHp() / 2)) {
            qDebug() << "[Relic] 🍖 带肉骨头 散发出香气！在绝境中为你回复了 12 点生命！";

            emit relicActivated();

            // 呼叫咱们早就写好的完美 heal 接口（内部自带上限防溢出）
            player->heal(12);
        } else {
            qDebug() << "[Relic] 🍖 带肉骨头 发现你还很健康，决定先不给你吃喵。";
        }
    }
};
