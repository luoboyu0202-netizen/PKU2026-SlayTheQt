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
};

#endif // CARD_H
