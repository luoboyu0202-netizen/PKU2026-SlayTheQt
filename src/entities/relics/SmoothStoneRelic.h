#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class SmoothStoneRelic : public Relic {
    Q_OBJECT
public:
    explicit SmoothStoneRelic(QObject* parent = nullptr)
        : Relic("relic_smooth_stone",
                QStringLiteral("光滑石头"),
                QStringLiteral("在每场战斗开始时，获得 1 点 敏捷 。"),
                parent) {
        m_counter = -1; // 静态被动，不需要显示数字
    }

    // ========================================================
    // ⚔️ 钩子：战斗开始时挂上敏捷 Buff
    // ========================================================
    void onBattleStart() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (engine && engine->getPlayer() && engine->getPlayer()->getStatusManager()) {
            qDebug() << "[Relic] 💎 光滑石头 闪烁！赋予 1 点敏捷！";

            emit relicActivated();

            // 挂上敏捷状态，后续卡牌计算 modifyBlock 时会自动变大
            engine->getPlayer()->getStatusManager()->applyStatus(StatusType::Dexterity, 1);
        }
    }
};
