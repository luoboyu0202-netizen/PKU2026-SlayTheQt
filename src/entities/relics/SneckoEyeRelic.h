#pragma once
#include "Relic.h"
#include "../entities/Player.h"
#include "../StatusManager.h"
#include <QDebug>

class SneckoEyeRelic : public Relic {
    Q_OBJECT
public:
    explicit SneckoEyeRelic(QObject* parent = nullptr)
        : Relic("relic_snecko_eye", "异蛇之眼", "每回合额外抽 2 张牌。在战斗开始时，获得 混乱。", parent) {}

    // 🔴 拦截战斗开始钩子！
    void onBattleStart(Player* player) override {
        if (!player) return;

        // 1. 永久修改本场战斗的抽牌数！
        player->setBaseDrawCount(player->getBaseDrawCount() + 2);

        // 2. 挂上究极破坏状态：混乱！
        // ⚠️ 请确保你的 StatusType 枚举里（StatusManager.h）已经加上了 Confusion
        player->getStatusManager()->applyStatus(StatusType::Confusion, 1);

        qDebug() << "[Relic] 👁️ 异蛇之眼凝视着你！抽牌数变更为" << player->getBaseDrawCount() << "，并陷入了深深的混乱喵！";

        emit relicActivated();
    }

    // 🧹 极其严谨的善后清理：战斗结束后把抽牌数还原！
    void onBattleEnd(Player* player) override {
        if (!player) return;
        player->setBaseDrawCount(5); // 还原默认值，防止下一场战斗叠加
    }
};
