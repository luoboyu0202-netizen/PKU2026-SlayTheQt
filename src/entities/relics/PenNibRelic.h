#pragma once
#include "Relic.h"
#include "../cards/Card.h"
#include <QDebug>

class PenNibRelic : public Relic {
    Q_OBJECT
public:
    explicit PenNibRelic(QObject* parent = nullptr)
        : Relic("relic_pen_nib", // 🔴 严格对应图片文件名：:/resources/images/relics/relic_pen_nib.png
                QStringLiteral("钢笔尖"),
                QStringLiteral("你每打出 10 张 攻击 牌，下一次攻击造成 双倍 伤害。"),
                parent) {

        // 覆盖基类的 -1，初始化为 0 层
        // 注意：在构造函数中直接修改成员变量，UI 托盘绑定时会自动读取这个初始值
        m_counter = 0;
    }

    // ========================================================
    // ⚔️ 钩子 1：修改攻击伤害 (在引擎扣除怪物血量前被循环调用)
    // ========================================================
    int modifyAttackDamage(int currentDamage) override {
        // 如果现在是 9 层，说明当前正在结算的这把武器就是第 10 张！
        // 💡 机制优势：多段伤害卡牌会多次调用此函数，全部享受翻倍，极其还原！
        if (getCounter() == 9) {
            qDebug() << "[Relic] 🖋️ 钢笔尖锋芒毕露！伤害翻倍！" << currentDamage << "-> " << (currentDamage * 2);
            return currentDamage * 2;
        }
        return currentDamage;
    }

    // ========================================================
    // 🃏 钩子 2：监听打牌动作 (在卡牌执行完 play 逻辑并进入弃牌堆后被调用)
    // ========================================================
    void onCardPlayed(Card* card) override {
        // 🛡️ 绝对安全防线：防一手空指针
        if (!card) return;

        // 钢笔尖只在乎攻击牌
        if (card->getType() == CardType::Attack) {
            if (getCounter() == 9) {
                qDebug() << "[Relic] 🖋️ 钢笔尖能量释放完毕，重置为 0 层喵！";

                // 1. 触发我们在 RelicItem 里写好的果汁感 Q弹放大特效！
                emit relicActivated();

                // 2. 层数归零（内部会 emit counterChanged 自动刷新 UI 数字）
                setCounter(0);
            } else {
                qDebug() << "[Relic] 🖋️ 钢笔尖默默记录了一次攻击，当前层数：" << getCounter() + 1;

                // 默默加 1 层，UI 数字会自动随之跳动
                setCounter(getCounter() + 1);
            }
        }
    }
};