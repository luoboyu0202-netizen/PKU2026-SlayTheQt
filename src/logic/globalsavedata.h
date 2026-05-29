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

    // 主角全局基础属性
    int currentHp = 80;
    int maxHp = 80;
    int gold = 99;
    int maxEnergy = 3;

    // 核心安全设计：只存 ID 列表，绝不存实体指针！
    QList<QString> deckIds;
    QList<QString> relicIds;

    // 初始化新游戏的初始状态
    void initNewGame() {
        currentHp = 80;
        gold = 99;
        deckIds.clear();
        relicIds.clear();

        // 塞入初始卡牌 ID (参考你 CardFactory 的可用 ID)
        deckIds.append("card_test");

        // 初始遗物 ID (参考你 RelicFactory 的可用 ID)
        relicIds.append("relic_burning_blood");
    }

    // 🔴【新增】：记忆保险箱，记录 <遗物ID, 计数值>
    QMap<QString, int> relicCounters;

private:
    GlobalSaveData() {}
};

#endif // GLOBALSAVEDATA_H