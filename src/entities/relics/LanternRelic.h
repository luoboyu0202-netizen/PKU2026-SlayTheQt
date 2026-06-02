#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>

class LanternRelic : public Relic {
    Q_OBJECT
public:
    explicit LanternRelic(QObject* parent = nullptr)
        : Relic("relic_lantern", // 对应贴图：relic_lantern.png
                QStringLiteral("灯笼"),
                QStringLiteral("在每场战斗的第一回合，获得 1 点额外 能量 。"),
                parent) {
        m_counter = -1;
    }

    // ========================================================
    // ⚔️ 钩子：监听战斗开始
    // ========================================================
    void onBattleStart() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getPlayer()) return;

        qDebug() << "[Relic] 🏮 灯笼 亮起！第一回合注入 1 点额外能量！";

        emit relicActivated();

        // 假设你的 Player 有 modifyEnergy 或 gainEnergy 方法
        // 注意：原版能量上限和当前能量是分开的，这里是给当前能量 +1
        engine->getPlayer()->addEnergy(1);
    }
};
