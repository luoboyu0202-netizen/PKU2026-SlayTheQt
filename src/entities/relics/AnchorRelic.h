#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>

class AnchorRelic : public Relic {
    Q_OBJECT
public:
    explicit AnchorRelic(QObject* parent = nullptr)
        : Relic("relic_anchor", // 对应贴图：relic_anchor.png
                QStringLiteral("锚"),
                QStringLiteral("每场战斗开始时，获得 10 点 格挡 。"),
                parent) {

        m_counter = -1; // 不需要显示数字
        m_imagePath = ":/resources/images/relics/relic_anchor.png";
    }

    // ========================================================
    // 🚩 钩子：监听战斗开始
    // ========================================================
    void onBattleStart() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getPlayer()) return;

        qDebug() << "[Relic] ⚓ 锚重重落下！为主角提供 10 点开局格挡！";

        // 触发 UI 的 Q 弹高光特效！
        emit relicActivated();

        // 注入 10 点护甲 (假设你的 Player 有 addBlock 接口)
        engine->getPlayer()->addBlock(10);
    }
};
