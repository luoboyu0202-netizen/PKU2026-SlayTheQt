#pragma once
#include <QString>
#include <QList>
#include "cards/Card.h"
#include "entities/relics/Relic.h"
#include "GameEnums.h"

// ==========================================
// 📦 战前出征密令：携带状态与关卡情报
// ==========================================
struct BattleContext {
    // --- 1. 玩家当前肉体状态 (原封不动) ---
    int currentHp = 0;
    int maxHp = 0;
    int gold = 0;
    int maxEnergy = 3;
    QList<Card*> currentDeck;
    QList<Relic*> relics;

    // --- 2. 🌟 战局情报 (全新加入！) ---
    int currentLayer = 0;

    // 🔴 核心解耦：直接使用极度安全的强类型枚举！
    NodeType nodeType = NodeType::Monster;

    // 备用通道：只有在特定事件时，才用这个字段强行锁死怪物ID
    QString forcedEnemyId = "";
};

// ==========================================
// 🏆 战后捷报：生还状态与战利品包裹 (保持原样即可)
// ==========================================
struct BattleResult {
    bool isVictory = false;
    int currentHp = 0;
    int maxHp = 0;
    int maxEnergy = 3;
    int currentGold = 0;
    QList<QString> finalDeckIds;
    QList<QString> finalRelicIds;
    int rewardGold = 0;
    QList<QString> rewardRelicIds;
    bool hasCardReward = false;
    int cardRewardCount = 3;
};