#pragma once
#include "Relic.h"

class IceCreamRelic : public Relic {
    Q_OBJECT
public:
    explicit IceCreamRelic(QObject* parent = nullptr)
        : Relic("relic_ice_cream",
                QStringLiteral("冰淇淋"),
                QStringLiteral("能量不再于你的回合结束时清空。"),
                parent) {
        m_counter = -1;
    }

    // 同理，它属于机制改写型遗物，只要确保头文件和工厂里有它，底层调用它作为状态开关即可！
};
