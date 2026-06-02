#pragma once
#include "Relic.h"
#include <QDebug>

class ToriiRelic : public Relic {
    Q_OBJECT
public:
    explicit ToriiRelic(QObject* parent = nullptr)
        : Relic("relic_torii",
                QStringLiteral("鸟居"),
                QStringLiteral("当你即将受到 5 点或更少的 未加格挡 的伤害时，将该伤害改为 1 点。"),
                parent) {
        m_counter = -1;
    }

    // ========================================================
    // 🛡️ 钩子：强行拦截并修改玩家即将受到的真实伤害值
    // ========================================================
    int modifyIncomingDamage(int damage)  {
        // 只有大于 1 点且小于等于 5 点的毛毛雨伤害，才会被鸟居化解
        if (damage > 1 && damage <= 5) {
            qDebug() << "[Relic] ⛩️ 鸟居结界张开！将" << damage << "点小额伤害化解为 1 点！";

            emit relicActivated(); // 特效闪烁！

            return 1;
        }
        return damage; // 其余大额伤害或 0/1 点伤害不触发
    }
};
