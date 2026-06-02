#pragma once
#include "entities/relics/Relic.h"
#include <QRandomGenerator>
#include <QList>
#include <QDebug>

class Relic;
class QObject;

class RelicFactory {
public:
    static Relic* createRelic(const QString& relicId, QObject* parent = nullptr);
    // 🔴 升級版搖獎機：傳入玩家已有的遺物 ID 列表，返回一個不重複的全新 ID！
    static QString generateRandomRelic(const QStringList& ownedRelicIds);

    // ========================================================
    // 🏺 1. 遗物全图鉴大字典
    // ========================================================
    static QList<QString> getAllAvailableRelicIds() {
        return {
            "relic_pen_nib", "relic_orichalcum", "relic_bag_of_preparation",
            "relic_anchor", "relic_burning_blood", "relic_vajra", "relic_snecko_eye",
            "relic_lantern", "relic_happy_flower", "relic_shuriken",
            "relic_smooth_stone", "relic_mercury_hourglass",
            "relic_torii", "relic_prayer_wheel", "relic_ice_cream"
            // 以后你每写一个新遗物，就往这里加一行喵！
        };
    }

private:
    RelicFactory() = default;
    ~RelicFactory() = default;
};
