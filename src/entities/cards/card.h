#pragma once
#include <QObject>
#include <QString>
#include "StatusManager.h"
#include "Player.h" // 🔴 必须包含全量定义，让编译器知道 Player 也是 Fighter！

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
// 1.5 定义卡牌的稀有度字典
// ==========================================
enum class CardRarity {
    Starter,    // 初始牌（如打击、防御）
    Common,     // 普通（白色）
    Uncommon,   // 罕见（蓝色）
    Rare,       // 稀有（金色）
    Special     // 特殊（如衍生牌）
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
    // 构造函数：只强制要求最基础的参数，防止传参噩梦
    explicit Card(const QString& id, const QString& name, int cost, bool isEthereal = false, QObject* parent = nullptr)
        : QObject(parent)
        , m_id(id)
        , m_name(name)
        , m_cost(cost)
        , m_isEthereal(isEthereal)
        , m_description("")      // 顺手给描述赋个空初值，防止野指针喵
        , m_type(CardType::Attack) // 默认给个安全的初值
        , m_target(CardTarget::None)
        , m_rarity(CardRarity::Common) // 默认普通稀有度
    {
        // 这里留空就可以啦，因为所有工作都在上面的初始化列表（Colon Initializer List）里做完啦喵！
    }
    virtual ~Card() = default;

    // 🟢【自动判定】：如果目标类型是 Enemy，就必须要求目标！
    virtual bool requiresTarget() const { return m_target == CardTarget::Enemy; }


    // 🔴【全新加入】：查询这张牌是否已经锻造过
    bool isUpgraded() const { return m_isUpgraded; }

    // 🔴【全新加入】：虚函数！篝火营地只要调用这个，卡牌就会自我升华！
    virtual void upgrade() {
        if (!m_isUpgraded) {
            m_isUpgraded = true;
            m_name += "+"; // 🌟 杀戮尖塔灵魂设定：升级后的牌名自动加个加号！
        }
    }

    // 统一的 Getter 接口（外部 UI 只能读，不能改）
    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; } // 新增：供 UI 画文字用
    // 🔴【新增】：读取照片地址的接口
    QString getImagePath() const { return m_imagePath; }

    int getCost() const { return m_cost; }
    CardType getType() const { return m_type; }              // 新增：判断是否受某些遗物影响
    CardTarget getTarget() const { return m_target; }        // 新增：判断划线逻辑
    CardRarity getRarity() const { return m_rarity; }        // 新增：商店定价逻辑依据


    // 核心接口：打出这张牌的实际逻辑（由子类实现）
    virtual void play(Player* source, Fighter* target, RelicManager* relics) = 0;

    // 🔴【喵娘的架构魔法】：把 public 改成 protected！
    // 这样子类（如 BashCard）可以直接给它们赋值，但外部系统无法随意篡改卡牌的属性！

    // 🔴 1. 新增属性获取器
    bool isUnplayable() const { return m_isUnplayable; }
    bool isEthereal() const { return m_isEthereal; }
    bool isExhaustOnUse() const { return m_exhaustOnUse; }

    // 🔴 2. 新增回合结束的钩子函数（默认什么都不做）
    // 供【灼伤】这类会在回合结束时发作的卡牌重写喵！
    virtual void triggerOnEndOfTurn() { }
    // 🔴 动态文案生成器！(如果 target 为空，说明还没指到怪，只算主角的力量；如果不为空，就算上怪物的易伤！)

    // 🔴【新增】：让外部 UI 知道它是不是 X 费牌
    bool isXCost() const { return m_isXCost; }

protected:
    QString m_id;
    QString m_name;
    QString m_description;
    int m_cost;
    CardType m_type;
    CardTarget m_target;
    QString m_imagePath;

    // 🔴 3. 新增属性开关（默认为假，普通牌都能打出且没有虚无）
    bool m_isUnplayable = false;
    bool m_isEthereal = false;
    bool m_exhaustOnUse = false;

    // 🔴 锻造防伪标签：默认是没升级的
    bool m_isUpgraded = false;

    int m_baseValue;
    int m_secondaryValue=0;

    // 🔴 新增：用于存放带标签的原始模板，例如 "造成 !D! 点伤害。给予 !M! 层易伤。"
    QString m_rawDescription;

    // 🔴【新增】：X 费牌专属视觉标记！
    bool m_isXCost = false;
    CardRarity m_rarity;

public:
    // ========================================================
    // 🔮【核心解析器】：统一处理所有卡牌的动态文案和染色！
    // ========================================================
    virtual QString getDynamicDescription(Player* source, Fighter* target = nullptr) {
        QString finalDesc = m_rawDescription;

        // ⚔️ 1. 解析伤害标签 !D! (Damage)
        if (finalDesc.contains("!D!")) {
            int currentDmg = m_baseValue;

            if (source) {
                // 🔴 完美匹配你的签名：(Fighter* source, Fighter* target, int baseDamage)
                // C++ 会自动将 Player* 向上转型为 Fighter* 喵！
                currentDmg = StatusManager::calculateDamage(source, target, m_baseValue);
            }

            QString dmgStr = QString::number(currentDmg);

            // 🎨 动态染色！
            if (currentDmg > m_baseValue) {
                dmgStr = QString("<font color='#7fff00'>%1</font>").arg(currentDmg);
            } else if (currentDmg < m_baseValue) {
                dmgStr = QString("<font color='#ff6563'>%1</font>").arg(currentDmg);
            }
            finalDesc.replace("!D!", dmgStr);
        }

        // 🛡️ 2. 解析格挡标签 !B! (Block)
        if (finalDesc.contains("!B!")) {
            int currentBlock = m_baseValue;

            if (source) {
                // 🔴 完美匹配你的签名：(Fighter* source, int baseBlock)
                currentBlock = StatusManager::calculateBlock(source, m_baseValue);
            }

            QString blockStr = QString::number(currentBlock);
            if (currentBlock > m_baseValue) {
                blockStr = QString("<font color='#7fff00'>%1</font>").arg(currentBlock);
            } else if (currentBlock < m_baseValue) {
                blockStr = QString("<font color='#ff6563'>%1</font>").arg(currentBlock);
            }
            finalDesc.replace("!B!", blockStr);
        }

        // 🌟 3. 解析副数值标签 !M! (Magic Number)
        if (finalDesc.contains("!M!")) {
            finalDesc.replace("!M!", QString::number(m_secondaryValue));
        }

        return finalDesc;
    }
};
