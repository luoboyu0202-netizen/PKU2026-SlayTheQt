#pragma once
#include <QObject>
#include <QString>
#include "cards/Card.h"

class Relic : public QObject {
    Q_OBJECT
public:
    explicit Relic(const QString& id, const QString& name, const QString& description, QObject* parent = nullptr)
        : QObject(parent), m_id(id), m_name(name), m_description(description), m_counter(-1), m_imagePath("") {}

    QString getName() const { return m_name; }
    QString getId() const { return m_id; }
    QString getDescription() const { return m_description; }
    int getCounter() const { return m_counter; }
    QString getImagePath() const { return m_imagePath; }

    void setCounter(int count) {
        m_counter = count;
        emit counterChanged(m_counter); // 计数变动信号
    }

    // ========================================================
    // 🔴【核心机制】：战斗事件钩子 (Hooks)！
    // 默认什么都不做，子类遗物根据自己的需求重写它们！
    // ========================================================

    // 1. 当玩家打出一张牌时触发（例如：钢笔尖、死灵书）
    virtual void onCardPlayed(Card* card) { Q_UNUSED(card); }

    // 2. 当计算攻击伤害时触发（例如：钢笔尖、金刚杵）
    virtual int modifyAttackDamage(int currentDamage) { return currentDamage; }

    // 3. 当计算获得格挡时触发（例如：奥利哈钢、外卡钳）
    virtual int modifyBlock(int currentBlock) { return currentBlock; }

    // 4. 当战斗开始时触发（例如：灯笼、赤牛）
    virtual void onBattleStart() {}

    // 5. 当回合结束时触发（例如：水银沙漏）
    virtual void onTurnEnd() {}

signals:
    void counterChanged(int newCount);
    void relicActivated(); // 触发时的闪烁信号

protected:
    QString m_id;
    QString m_name;
    QString m_description;
    int m_counter; // -1 表示不显示数字
    QString m_imagePath;
};
