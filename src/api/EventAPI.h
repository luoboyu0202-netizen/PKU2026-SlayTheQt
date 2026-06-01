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
    QString eventSubtype;

    int currentLayer = 0; // 🔴 補上這行！GameWindow 生成戰鬥時需要它！
};

// 输出合同：事件结束后还给地图组的结算报告
struct EventResult {
    int remainingHp;
    int finalMaxHp;     // 🔴 救命拼圖：必須加上這個，否則甜甜圈加的血上限帶不出來！
    int currentGold;
    QList<Card*> resultDeck;
    QList<Relic*> resultRelics;

    bool hpChanged = false;
    bool deckChanged = false;
    bool relicsChanged = false;
    bool goldChanged = false;
    bool playerDead = false;
};
