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
            "relic_pen_nib",       // 钢笔尖
            "relic_snecko_eye",    // 异蛇之眼
            "relic_vajra",         // 金刚杵 (+1 力量)
            "relic_anchor",        // 船锚 (第一回合加格挡)
            "relic_burning_blood", // 燃烧之血 (战斗结束回血)
            "relic_bag_of_preparation",
            "relic_orichalcum",
            // 以后你每写一个新遗物，就往这里加一行喵！
        };
    }

private:
    RelicFactory() = default;
    ~RelicFactory() = default;
};
