#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>

class OrichalcumRelic : public Relic {
    Q_OBJECT
public:
    explicit OrichalcumRelic(QObject* parent = nullptr)
        : Relic("relic_orichalcum", // 对应贴图：relic_orichalcum.png
                QStringLiteral("奥利哈钢"),
                QStringLiteral("如果你的回合结束时没有 格挡 ，获得 6 点 格挡 。"),
                parent) {

        m_counter = -1;
    }

    // ========================================================
    // 🌙 钩子：监听回合结束
    // ========================================================
    void onTurnEnd() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getPlayer()) return;

        Player* player = engine->getPlayer();

        // 🔴 核心机制：窥探当前状态！只有在格挡为 0 的时候才发功！
        if (player->getBlock() == 0) {
            qDebug() << "[Relic] 🛡️ 奥利哈钢共鸣！检测到无格挡，强行生成 6 点护盾！";

            emit relicActivated();
            player->addBlock(6);

        } else {
            // 如果已经有格挡了，就保持沉默，连特效都不触发
            qDebug() << "[Relic] 🛡️ 奥利哈钢检测到已有护甲，保持沉默喵。";
        }
    }
};