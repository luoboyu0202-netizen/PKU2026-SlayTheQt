#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include "StatusManager.h" // 🔴 必须引入状态管家，为了使用 StatusType
#include "Fighter.h"

// 1. 状态类型枚举 (确保 StatusManager.h 中有这个)
// enum class StatusType { None, Strength, Dexterity, Weak, Vulnerable };

// 2. 扩容意图类型，加入 Buff、复合意图等！
enum class IntentType {
    Unknown,
    Attack,
    Defend,
    Debuff,          // 给玩家上负面状态
    Buff,            // 给自己上正面状态
    AttackAndDebuff, // 攻击并给你上异常
    DefendAndBuff,   // 叠甲并给自己上增益
    AttackAndDefend, // 攻击并获得护甲
    InsertStatus,    // 给你牌库或手牌塞状态牌！
    Summon,           // 召唤小怪加入战场！
    Curse,
    AttackAndInsertStatus,
    GroupBuff,       // 全体强化（给所有活着的怪物加力量等）
    GroupDefend      // 全体叠甲（盾地精等辅助怪专属）
};

struct Intent {
    int multiHitCount=1;     // 🔴 新增：多段攻击次数
    IntentType type;
    int value;               // 通用数值（如伤害、护甲）
    StatusType statusType;   // 状态类型
    int statusValue;         // 状态层数

    // 🔴 为塞状态牌和召唤小怪准备的专属字段
    QString cardIdToInsert;  // 要塞进来的卡牌ID（比如 "card_slimed"）
    QString enemyIdToSummon; // 要召唤的怪物ID（比如 "Slime_Small"）

    // 构造函数
    Intent(IntentType t = IntentType::Unknown, int v = 0, StatusType s = StatusType::None, int sv = 0, const QString& cardId = "", const QString& summonId = "", int multiHit = 1)
        : type(t), value(v), statusType(s), statusValue(sv), cardIdToInsert(cardId), enemyIdToSummon(summonId), multiHitCount(multiHit) {}
};

class Enemy : public Fighter {
    Q_OBJECT
public:
    // 🔴 1. 构造函数新增 imagePath 参数，给个默认值防止旧代码报错喵！
    explicit Enemy(const QString& name, int maxHp, const QString& imagePath = "", QObject* parent = nullptr);

    // 🔴 2. 照片与身份 ID 接口
    QString getImagePath() const { return m_imagePath; }
    void setId(const QString& id) { m_id = id; }
    QString getId() const { return m_id; }

    // ========================================================
    // 🔴 3. 完美重写的 Getter：直接向新大脑要数据！
    // ========================================================
    Intent getCurrentIntent() const { return m_currentIntent; }
    IntentType getIntentType() const { return m_currentIntent.type; }
    int getIntentValue() const { return m_currentIntent.value; }

    // ========================================================
    // 🧠 4. 虚拟大脑：让子类别自己决定下一招！
    // ========================================================
    virtual void rollNextIntent();
    virtual void onBattleStart() {}

    // 🔴 5. 护甲与座位号操作
    void addBlock(int b);
    void loseBlock();
    int getSlotIndex() const { return m_slotIndex; }
    void setSlotIndex(int index) { m_slotIndex = index; }

    // 🔴 获取和设置体型倍率
    qreal getScaleFactor() const { return m_scaleFactor; }
    void setScaleFactor(qreal scale) { m_scaleFactor = scale; }
    // ========================================================
    // 🧠 抹除意图：打完收工后，让大脑陷入沉睡
    // ========================================================
    void clearIntent() {
        m_currentIntent = Intent(IntentType::Unknown);
    }

signals:
    void intentChanged(IntentType type, int value);

protected:
    // ========================================================
    // 📜 6. 记忆中枢：用来防止连续出同一招！(开放给子类使用)
    // ========================================================
    Intent m_currentIntent;          // 怪物脑子里正在想的事
    int m_moveHistory[2] = {-1, -1}; // 记录上回合和上上回合出过的招式 (用整数ID代表)

    // 帮助函数：把刚刚决定的招式写入历史记录
    void recordMove(int moveId) {
        m_moveHistory[1] = m_moveHistory[0]; // 上回合变成上上回合
        m_moveHistory[0] = moveId;           // 这回合变成上回合
    }

    // ... 其他受保护的变量 ...
    qreal m_scaleFactor = 1.0; // 🔴 默认大家都是 1.0 倍大小

    // 帮助函数：检测是否刚刚用过这招
    bool lastMoveWas(int moveId) const { return m_moveHistory[0] == moveId; }

    // 帮助函数：检测是否连续两次都用了这招
    bool lastTwoMovesWere(int moveId) const { return m_moveHistory[0] == moveId && m_moveHistory[1] == moveId; }

private:
    // ❌ 已经彻底删除旧的 m_intentSequence 和 m_currentIntentIndex！再见，死板的录影带！
    QString m_imagePath;
    QString m_id;
    int m_slotIndex = -1; // 🔴 默认是 -1（未排座）


};
