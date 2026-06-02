#pragma once
#include <QObject>
#include <QList>
#include "Relic.h"

class Card;

class RelicManager : public QObject {
    Q_OBJECT
public:
    explicit RelicManager(QObject* parent = nullptr) : QObject(parent) {}

    void addRelic(Relic* relic) {
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

    int modifyIncomingDamage(int damage) {
        // 假设你用来存放遗物的列表叫 m_relics (如果是其他名字，比如 relics，请替换一下喵)
        for (Relic* relic : m_relics) {
            if (relic) {
                // 让每个遗物都有机会去拦截和修改这个伤害
                damage = relic->modifyIncomingDamage(damage);
            }
        }
        return damage; // 返回被遗物层层修改后的最终伤害！
    }

    bool hasRelic(const QString& relicId) const {
        for (Relic* relic : m_relics) {
            if (relic && relic->getId() == relicId) {
                return true;
            }
        }
        return false;
    }

    const QList<Relic*>& getRelics() const { return m_relics; }

signals:
    void relicAdded(Relic* relic);

private:
    QList<Relic*> m_relics;
};
