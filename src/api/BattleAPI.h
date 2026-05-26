#pragma once
#include <QList>
#include <QString>
#include "cards/Card.h"
#include "RelicItem.h"

// 📥【输入合同】：外部系统组传给我们的玩家状态与遭遇信息
struct BattleContext {
    int currentHp;
    int maxHp;
    int gold;
    int maxEnergy;
    QList<Card*> currentDeck; // 玩家当前的卡组
    QString enemySeedOrId;    // 遇到什么怪物？(比如 "Slime_01")
    QList<Relic*> relics;
    // 还可以加 QList<QString> relics; 等等
};

// 📤【输出合同】：我们打完之后，还给系统组的结算报告
struct BattleResult {
    bool isVictory;    // 赢了还是死了？
    int remainingHp;   // 打完还剩多少血？（以便大地图更新状态）
    int currentGold;   // 打完还剩多少钱？（怪掉落的或者被小偷偷走的）
    QList<Relic*> battledRelics;
};