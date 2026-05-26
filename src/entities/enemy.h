#pragma once
#include <QObject>
#include <QString>
#include <QList>
#include "StatusManager.h" // 🔴 必须引入状态管家，为了使用 StatusType
#include "Fighter.h"


// 1. 顺手给 StatusType 补一个 None（如果你还没加的话），打开 StatusManager.h 加上：
// enum class StatusType { None, Strength, Dexterity, Weak, Vulnerable };

// 2. 扩容意图类型，加入 Buff！
enum class IntentType {
    Unknown,
    Attack,
    Defend,
    Debuff, // 给玩家上负面状态
    Buff,    // 给自己上正面状态
    // 🔴【全新加入】：复合意图！
    AttackAndDebuff, // 攻击并给你上异常
    AttackAndBuff,   // 攻击并给自己上增益
    DefendAndBuff,    // 叠甲并给自己上增益

    InsertStatus, // 给你牌库或手牌塞状态牌！
    Summon        // 召唤小怪加入战场！
};

struct Intent {
    int multiHitCount=1; // 🔴 新增：多段攻击次数
    IntentType type;
    int value;             // 通用数值（如伤害、护甲）
    StatusType statusType; // 状态类型
    int statusValue;       // 状态层数

    // 🔴【新增】：为塞状态牌和召唤小怪准备的专属字段
    QString cardIdToInsert; // 要塞进来的卡牌ID（比如 "card_slimed"）
    QString enemyIdToSummon; // 要召唤的怪物ID（比如 "Slime_Small"）

    // 构造函数末尾加上：int multiHit = 1
    Intent(IntentType t = IntentType::Unknown, int v = 0, StatusType s = StatusType::None, int sv = 0, const QString& cardId = "", const QString& summonId = "", int multiHit = 1)
        : type(t), value(v), statusType(s), statusValue(sv), cardIdToInsert(cardId), enemyIdToSummon(summonId), multiHitCount(multiHit) {} // 初始化它！
};

class Enemy : public Fighter {
    Q_OBJECT
public:

    // 🔴 1. 构造函数新增 imagePath 参数，给个默认值防止旧代码报错喵！
    explicit Enemy(const QString& name, int maxHp, const QString& imagePath = "", QObject* parent = nullptr);

    // 🔴 2. 增加获取照片地址的接口
    QString getImagePath() const { return m_imagePath; }

    Intent getCurrentIntent() const {
        // 🔴 现在用 int 类型的 m_currentIntentIndex 来做判断和取值
        if (m_currentIntentIndex >= 0 && m_currentIntentIndex < m_intentSequence.size()) {
            return m_intentSequence.at(m_currentIntentIndex);
        }
        return Intent(); // 如果越界了，返回一个空意图兜底
    }

    // ========================================================
    // 🔴【新增】：怪物的内部身份证 ID！
    // ========================================================
    void setId(const QString& id) { m_id = id; }
    QString getId() const { return m_id; }

    // 【主策钦点功能】: 自由注入意图序列
    void setIntentSequence(const QList<Intent>& sequence);

    // 切换到下一个意图
    void rollNextIntent();

    // 🔴 完美重写的 Getter：直接向新系统要数据！
    IntentType getIntentType() const { return getCurrentIntent().type; }
    int getIntentValue() const { return getCurrentIntent().value; }

    // 🔴【新增】：护甲操作声明
    void addBlock(int b);
    void loseBlock();

    // 🔴【全新加入】：获取和设定怪物的绝对座位号（0 代表最左边，1 代表第二个...）
    int getSlotIndex() const { return m_slotIndex; }
    void setSlotIndex(int index) { m_slotIndex = index; }

signals:
    void intentChanged(IntentType type, int value);

private:
    QList<Intent> m_intentSequence;
    int m_currentIntentIndex; // 🔴 必须是 int 类型！代表当前打到第几个意图了
    QString m_imagePath;
    QString m_id;
    int m_slotIndex = -1; // 🔴 默认是 -1（未排座）
};
