#pragma once
#include <QString>
#include <QList>
#include "cards/Card.h"
#include "entities/relics/Relic.h"

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
    int currentLayer = 0;       // 当前所在层数（决定怪物血量、攻击力的动态成长系数）

    // 🔴 核心解耦：只传类型标签，不传死具体的怪物名字！
    // 比如："Monster" (普通怪), "Elite" (精英), "Boss", "Event" (事件强敌)
    QString nodeType = "Monster";

    // 备用通道：只有在特定事件（如：剧情强制打大史莱姆）时，才用这个字段强行锁死怪物ID
    QString forcedEnemyId = "";
};

// ==========================================
// 🏆 战后捷报：生还状态与战利品包裹
// ==========================================
struct BattleResult {
    // --- 1. 战局生死裁决 ---
    bool isVictory = false;

    // --- 2. 经过战火洗礼后的肉体状态 ---
    int currentHp = 0;
    int maxHp = 0;
    int maxEnergy = 3;
    int currentGold = 0; // ⚠️ 注意：这是进战斗前身上的钱，不是爆出来的钱！

    // --- 3. 经过战斗可能发生永久改变的资产 ---
    // (例如：战斗中用了【噬咬】等永久改变牌组的卡，或者触发了破坏遗物的机制)
    QList<QString> finalDeckIds;
    QList<QString> finalRelicIds;

    // --- 4. 🎁 战利品包裹 (Loot Box) - 全新加入！ ---
    // 司令部拿到这些数据后，就会弹出华丽的【战利品拾取界面】！
    int rewardGold = 0;                // 怪物爆出的金币（比如随机 10~20 金币）
    QList<QString> rewardRelicIds;     // 爆出的遗物（比如打赢精英怪必掉 1 个随机遗物）
    bool hasCardReward = false;        // 是否掉落了卡牌抓取机会？(绝大多数战斗为 true)
    int cardRewardCount = 3;           // 掉落几张卡牌供玩家三选一？（通常是3张，有特定遗物变4张）
};