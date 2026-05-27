#pragma once
#include <QObject>
#include <QList>
#include "Relic.h"

class Card;

class RelicManager : public QObject {
    Q_OBJECT
public:
    explicit RelicManager(QObject* parent = nullptr);

    void addRelic(Relic* relic) {
        relic->setParent(this);
        m_relics.append(relic);
        emit relicAdded(relic); // 通知 UI 新增遗物
    }

    // ==========================================================
    // 🔴【救命接口 1】：接收卡牌打出事件，并广播给所有遗物！
    // ==========================================================
    void onCardPlayed(Card* card) {
        for (Relic* r : m_relics) {
            if (r) r->onCardPlayed(card);
        }
    }

    // ==========================================================
    // 🔴【救命接口 2】：层层叠加计算攻击伤害增幅！
    // ==========================================================
    int modifyAttackDamage(int baseDamage) {
        int finalDamage = baseDamage;
        // 让每一个遗物都有机会对伤害动手动脚喵！
        for (Relic* r : m_relics) {
            if (r) finalDamage = r->modifyAttackDamage(finalDamage);
        }
        return finalDamage;
    }

    // ==========================================================
    // 🔴【新接口】：层层叠加计算护甲增幅！
    // ==========================================================
    int modifyBlock(int baseBlock) {
        int finalBlock = baseBlock;
        for (Relic* r : m_relics) {
            if (r) finalBlock = r->modifyBlock(finalBlock);
        }
        return finalBlock;
    }

    const QList<Relic*>& getRelics() const { return m_relics; }

signals:
    void relicAdded(Relic* relic);

private:
    QList<Relic*> m_relics;
};
