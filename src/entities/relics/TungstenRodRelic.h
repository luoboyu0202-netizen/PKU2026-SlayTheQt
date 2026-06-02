#pragma once
#include "Relic.h"
#include <QDebug>
#include <algorithm> // 引入 std::max 保证伤害不出现负数

class TungstenRodRelic : public Relic {
    Q_OBJECT
public:
    explicit TungstenRodRelic(QObject* parent = nullptr)
        : Relic("relic_tungsten_rod",
                QStringLiteral("钨合金棒"),
                QStringLiteral("你失去的 生命 减少 1 点。"),
                parent) {
        m_counter = -1;
    }

    // ⛩️ 强行拦截并削减真实伤害
    int modifyIncomingDamage(int damage) override {
        if (damage > 0) {
            qDebug() << "[Relic] 🦯 钨合金棒 变硬了！强行吸收了 1 点真实伤害！";
            emit relicActivated();

            // 减 1 点，并用 std::max 保证伤害最低扣为 0
            return std::max(0, damage - 1);
        }
        return damage;
    }
};