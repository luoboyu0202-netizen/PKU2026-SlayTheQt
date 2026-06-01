#pragma once
#include "Relic.h"
#include "../../logic/BattleEngine.h"
#include "../../logic/CardManager.h"
#include <QDebug>

class BagOfPreparationRelic : public Relic {
    Q_OBJECT
public:
    explicit BagOfPreparationRelic(QObject* parent = nullptr)
        : Relic("relic_bag_of_preparation", // 对应贴图：relic_bag_of_preparation.png
                QStringLiteral("准备背包"),
                QStringLiteral("在每场战斗开始时，额外抽 2 张牌。"),
                parent) {

        m_counter = -1;
        m_imagePath = ":/resources/images/relics/relic_bag_of_preparation.png";
    }

    // ========================================================
    // 🚩 钩子：监听战斗开始
    // ========================================================
    void onBattleStart() override {
        BattleEngine* engine = BattleEngine::getInstance();
        if (!engine || !engine->getCardManager()) return;

        qDebug() << "[Relic] 🎒 准备背包拉开拉链！开局额外抽取 2 张牌！";

        emit relicActivated();

        // 直接呼叫卡牌大管家，强行抽 2 张牌！
        engine->getCardManager()->drawCards(2);
    }
};