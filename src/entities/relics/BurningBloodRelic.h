#pragma once
#include "Relic.h"
#include "../entities/Player.h"
#include <QDebug>

class BurningBloodRelic : public Relic {
    Q_OBJECT
public:
    explicit BurningBloodRelic(QObject* parent = nullptr)
        // 参数依次为：ID, 中文名, 描述, 父对象 (请根据你的 Relic 构造函数参数微调喵)
        : Relic("relic_burning_blood", "燃烧之血", "在战斗结束时，回复 6 点生命。", parent) {}

    // 🔴 拦截战斗结束的钩子！
    void onBattleEnd(Player* player)  {
        if (!player) return;

        int healAmount = 6;

        // 假设你的 Player 有 heal 方法，如果没有，请用 setHp(player->getHp() + healAmount) 且注意不要超过上限！
        int newHp = qMin(player->getHp() + healAmount, player->getMaxHp());
        player->setHp(newHp);

        qDebug() << "[Relic] 🩸 燃烧之血沸腾了！回复了 6 点生命，当前生命：" << newHp;

        // 💥 触发遗物闪烁和放大的果汁感动画！
        emit relicActivated();
    }
};
