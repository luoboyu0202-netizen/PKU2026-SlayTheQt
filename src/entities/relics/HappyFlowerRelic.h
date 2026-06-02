#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../Player.h"
#include <QDebug>

class HappyFlowerRelic : public Relic {
    Q_OBJECT
public:
    explicit HappyFlowerRelic(QObject* parent = nullptr)
        : Relic("relic_happy_flower", // 对应贴图：relic_happy_flower.png
                QStringLiteral("开心小花"),
                QStringLiteral("每经过 3 个回合，获得 1 点 能量 。"),
                parent) {

        // 🔴 覆盖基类的 -1，初始化为 0 层
        m_counter = 0;
    }

    // ========================================================
    // ☀️ 钩子：监听回合开始
    // ========================================================
    void onTurnStart(){
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getPlayer()) return;

        // 如果当前已经是 2 层，说明这个回合就是第 3 个回合！
        if (getCounter() == 2) {
            qDebug() << "[Relic] 🌻 开心小花 绽放啦！产生 1 点能量，层数归零喵！";

            emit relicActivated();

            // 给你 1 点能量
            engine->getPlayer()->addEnergy(1);

            // 层数归零 (内部会自动 emit counterChanged 刷新 UI)
            setCounter(0);
        } else {
            // 还没到 3 回合，默默把层数 +1
            qDebug() << "[Relic] 🌻 开心小花 吸收了阳光，当前层数：" << getCounter() + 1;
            setCounter(getCounter() + 1);
        }
    }
};
