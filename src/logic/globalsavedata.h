#ifndef GLOBALSAVEDATA_H
#define GLOBALSAVEDATA_H

#include <QString>
#include <QList>
#include <QMap>

class GlobalSaveData {
public:
    // 单例模式获取全局唯一实例
    static GlobalSaveData* getInstance() {
        static GlobalSaveData instance;
        return &instance;
    }

    // ========================================================
    // 🎲 PRD 偽隨機系統的靈魂變數
    // ========================================================
    QStringList availableEvents; // 事件抽獎袋
    int questionMarkMonsterChance = 10; // 問號節點遇到怪物的初始機率 (10%)

    // 主角全局基础属性
    int currentHp = 80;
    int maxHp = 80;
    int gold = 999;
    int maxEnergy = 3;
    int cardRemovalCost = 75;

    // --- 问号事件：留给自己的讯息 (Note For Yourself) ---
    QString storedCardId = "card_strike"; // 首次默认为打击 (原作为铁斩波，此处用已有卡牌)
    bool isStoredCardUpgraded = false;

    // 核心安全设计：只存 ID 列表，绝不存实体指针！
    QList<QString> deckIds;
    QList<QString> relicIds;

    // 初始化新游戏的初始状态
    void initNewGame() {
        currentHp = 80;
        gold = 999;
        cardRemovalCost = 75;
        deckIds.clear();
        relicIds.clear();

        // 塞入初始卡牌 ID (参考你 CardFactory 的可用 ID)
        deckIds.append("card_bash");
        deckIds.append("card_strike");
        deckIds.append("card_strike");
        deckIds.append("card_strike");
        deckIds.append("card_strike");
        deckIds.append("card_strike");
        deckIds.append("card_defend");
        deckIds.append("card_defend");
        deckIds.append("card_defend");
        deckIds.append("card_defend");


        // 初始遗物 ID (参考你 RelicFactory 的可用 ID)
        relicIds.append("relic_burning_blood");

    }

    // 🔴【新增】：记忆保险箱，记录 <遗物ID, 计数值>
    QMap<QString, int> relicCounters;

private:
    GlobalSaveData() {}
};

#endif // GLOBALSAVEDATA_H