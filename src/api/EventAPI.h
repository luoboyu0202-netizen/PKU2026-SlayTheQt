#pragma once
#include <QList>
#include <QString>
#include "cards/Card.h"
#include "RelicItem.h"

// 事件类型枚举
enum class EventType {
    Campfire,     // 火堆
    Merchant,     // 商人
    Chest,        // 宝箱
    QuestionMark  // 问号
};

// 输入合同：地图组传给事件模块的玩家状态与事件信息
struct EventContext {
    int currentHp;
    int maxHp;
    int gold;
    int maxEnergy;
    QList<Card*> currentDeck;
    QList<Relic*> relics;
    EventType eventType;
    QString eventSubtype; // 问号事件的子类型ID（可选）
};

// 输出合同：事件结束后还给地图组的结算报告
struct EventResult {
    int remainingHp;
    int currentGold;
    QList<Card*> resultDeck;
    QList<Relic*> resultRelics;

    bool hpChanged = false;
    bool deckChanged = false;
    bool relicsChanged = false;
    bool goldChanged = false;
    bool playerDead = false; // 玩家在事件中死亡（如在问号战斗中阵亡）
};
