#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QString>
#include "StatusManager.h"
#include "Player.h"

// 前置声明：只告诉编译器有这些类，不需要读它们的头文件，彻底切断循环依赖！
class BattleEngine;
class Fighter;
class RelicManager;

enum class CardType { Attack, Skill, Power, Status, Curse };
enum class CardTarget { Enemy, AllEnemies, Player, None };

class Card : public QObject {
    Q_OBJECT
public:
    explicit Card(const QString& id, const QString& name, int cost, bool isEthereal = false, QObject* parent = nullptr);
    virtual ~Card() = default;

    virtual bool requiresTarget() const { return m_target == CardTarget::Enemy; }
    bool isUpgraded() const { return m_isUpgraded; }
    virtual void upgrade();

    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }
    QString getImagePath() const { return m_imagePath; }
    int getCost() const { return m_cost; }

    // ========================================================
    // 🔴【新增】：动态费用篡改接口（异蛇之眼、疯狂、破灭等卡牌必备！）
    // ========================================================
    virtual void setCost(int newCost) {
        // 防止出现负数费用
        if (newCost < 0) newCost = 0;

        m_cost = newCost;
        m_isCostModified = true; // 🌟 极其重要：打上“被篡改”的思想钢印！
    }

    // 查询这张牌的费用是不是被外力扭曲过？
    bool isCostModified() const { return m_isCostModified; }

    // 允许外部强行重置修改状态（比如战斗结束后）
    void setCostModified(bool modified) { m_isCostModified = modified; }

    CardType getType() const { return m_type; }
    CardTarget getTarget() const { return m_target; }

    virtual void play(Player* source, Fighter* target, RelicManager* relics) = 0;

    bool isUnplayable() const { return m_isUnplayable; }
    bool isEthereal() const { return m_isEthereal; }
    bool isExhaustOnUse() const { return m_exhaustOnUse; }
    virtual void triggerOnEndOfTurn() { }
    bool isXCost() const { return m_isXCost; }

    // 🔴 声明在这里，实现移到 .cpp 中去！
    virtual QString getDynamicDescription(Player* source, Fighter* target = nullptr);

protected:
    QString m_id;
    QString m_name;
    QString m_description;
    int m_cost;
    CardType m_type;
    CardTarget m_target;
    QString m_imagePath;

    bool m_isUnplayable = false;
    bool m_isEthereal = false;
    bool m_exhaustOnUse = false;
    bool m_isUpgraded = false;

    int m_baseValue;
    int m_secondaryValue = 0;
    QString m_rawDescription;
    bool m_isXCost = false;

    // 🔴【新增】：费用修改标识，默认是干干净净的 false
    bool m_isCostModified = false;
};

#endif // CARD_H
