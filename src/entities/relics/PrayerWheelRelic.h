#pragma once
#include "Relic.h"

class PrayerWheelRelic : public Relic {
    Q_OBJECT
public:
    explicit PrayerWheelRelic(QObject* parent = nullptr)
        : Relic("relic_prayer_wheel",
                QStringLiteral("转经轮"),
                QStringLiteral("普通敌人掉落的 卡牌奖励 现在可以挑选 2 次。"),
                parent) {
        m_counter = -1; // 静态规则更改类遗物，无计数器
    }

    // 该遗物无需实现战斗钩子，它的 ID 将被结算引擎直接提取使用
};
