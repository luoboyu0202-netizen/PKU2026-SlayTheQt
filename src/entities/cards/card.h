#pragma once
#include <QObject>
#include <QString>
#include "StatusManager.h"
#include "Player.h"

class Player;
class Fighter;
class RelicManager;

// ==========================================
// 1. 定义卡牌的类型字典
// ==========================================
enum class CardType {
    Attack,  // 攻击牌（红色边框）
    Skill,   // 技能牌（绿色边框）
    Power,   // 能力牌（蓝色边框）
    Status,  // 状态牌（如：伤口）
    Curse    // 诅咒牌
};

// ==========================================
// 🌟 1.5 定义卡牌的稀有度字典 (队友商店系统的灵魂)
// ==========================================
enum class CardRarity {
    Starter,    // 初始牌（如打击、防御）
    Common,     // 普通（白色）
    Uncommon,   // 罕见（蓝色）
    Rare,       // 稀有（金色）
    Special     // 特殊（如衍生牌、诅咒）
};

// ==========================================
// 2. 定义卡牌的目标类型字典
// ==========================================
enum class CardTarget {
    Enemy,       // 需要指向单个敌人
    AllEnemies,  // 针对全体敌人（AOE）
    Player,      // 针对玩家自己（比如叠甲）
    None         // 无目标（比如抽牌的技能牌）
};

// ==========================================
// 3. 卡牌基类
// ==========================================
class Card : public QObject {
    Q_OBJECT

public:
    explicit Card(const QString& id, const QString& name, int cost, bool isEthereal = false, QObject* parent = nullptr)
        : QObject(parent)
        , m_id(id)
        , m_name(name)
        , m_cost(cost)
        , m_isEthereal(isEthereal)
        , m_description("")
        , m_type(CardType::Attack)
        , m_target(CardTarget::None)
        , m_rarity(CardRarity::Common) // 默认给个普通稀有度
        , m_owner(nullptr) // 🔴 初始化時預設為空
    {
    }
    virtual ~Card() = default;

    // ========================================================
    // 🔗 靈魂綁定：設定與獲取卡牌的主人！
    // ========================================================
    void setOwner(Player* owner) { m_owner = owner; }
    Player* getOwner() const { return m_owner; }

    virtual bool requiresTarget() const { return m_target == CardTarget::Enemy; }
    bool isUpgraded() const { return m_isUpgraded; }

    virtual void upgrade() {
        if (!m_isUpgraded) {
            m_isUpgraded = true;
            m_name += "+";
            m_id += "+"; // 存档与工厂防伪标识
        }
    }

    // ========================================================
    // 👑 统一的 Getter 接口
    // ========================================================
    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }
    QString getImagePath() const { return m_imagePath; }
    int getCost() const { return m_cost; }
    CardType getType() const { return m_type; }
    CardTarget getTarget() const { return m_target; }
    CardRarity getRarity() const { return m_rarity; } // 🌟 商店定价的绝对依据！

    // ========================================================
    // ⚙️ 动态属性修改器 (捍卫异蛇之眼的尊严！)
    // ========================================================
    virtual void setCost(int newCost) {
        if (newCost < 0) newCost = 0;
        m_cost = newCost;
        m_isCostModified = true;
    }
    bool isCostModified() const { return m_isCostModified; }
    void setCostModified(bool modified) { m_isCostModified = modified; }

    // ========================================================
    // 🛡️ 战斗状态查询
    // ========================================================
    bool isUnplayable() const { return m_isUnplayable; }
    bool isEthereal() const { return m_isEthereal; }
    bool isExhaustOnUse() const { return m_exhaustOnUse; }
    bool isXCost() const { return m_isXCost; }

    // ========================================================
    // ⚔️ 核心战斗接口
    // ========================================================
    virtual void play(Player* source, Fighter* target, RelicManager* relics) = 0;
    virtual void triggerOnEndOfTurn() { }

    // ========================================================
    // 🔮 动态文案生成器 (声明在头文件，实现在 .cpp，绝对安全！)
    // ========================================================
    virtual QString getDynamicDescription(Player* source, Fighter* target = nullptr);

protected:
    // 静态与基础属性
    QString m_id;
    QString m_name;
    QString m_description;
    QString m_rawDescription;
    QString m_imagePath;

    int m_cost;
    int m_baseValue = 0;
    int m_secondaryValue = 0;

    CardType m_type;
    CardTarget m_target;
    CardRarity m_rarity; // 🌟 稀有度基因

    // 🔴 核心牽絆：卡牌的主人指標
    Player* m_owner;

    // 状态标志位
    bool m_isUnplayable = false;
    bool m_isEthereal = false;
    bool m_exhaustOnUse = false;
    bool m_isUpgraded = false;
    bool m_isXCost = false;
    bool m_isCostModified = false;
};