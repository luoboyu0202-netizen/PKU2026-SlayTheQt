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
    static Relic* generateRandomRelic(QObject* parent = nullptr);

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

    // ========================================================
    // 🎰 2. 智能去重摇号机：掉落一个玩家绝对没有的遗物！
    // ========================================================
    static QString generateRandomRelic(const QList<QString>& ownedRelics) {
        QList<QString> pool = getAllAvailableRelicIds();

        // 🔴 极其无情的剔除算法：你有过的，池子里通通删掉！
        for (const QString& ownedId : ownedRelics) {
            pool.removeAll(ownedId);
        }

        // 🟢 触发 STS 原版的终极彩蛋防崩机制
        if (pool.isEmpty()) {
            qWarning() << "[RelicFactory] ⚠️ 卧槽！遗物池被抽干了！玩家拿到了全收集！";
            // 在原版杀戮尖塔中，如果你把遗物拿光了，系统就会无限给你掉落叫“头环 (Circlet)”的无用遗物。
            // 这里我们暂时返回空字符串，代表“没东西可掉了”。
            return "";
        }

        // 🎲 摇号并返回！
        int randomIndex = QRandomGenerator::global()->bounded(pool.size());
        return pool[randomIndex];
    }

private:
    RelicFactory() = default;
    ~RelicFactory() = default;
};
