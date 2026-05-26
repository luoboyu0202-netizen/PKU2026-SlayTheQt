// SlimedCard.h
#pragma once
#include "Card.h"
#include <QDebug>

class SlimedCard : public Card {
public:
    // 🔴 传入 ID, 名字, 费用(1费!), 是否虚无(false), parent
    explicit SlimedCard(QObject* parent = nullptr)
        : Card("card_slimed", QStringLiteral("黏液"), 1, false, parent) {

        m_description = QStringLiteral("消耗。");
        m_type = CardType::Status;
        m_target = CardTarget::None; // 扔黏液不需要指定目标

        // 🔴 核心配置：
        m_isUnplayable = false; // 它是可以打出的！
        m_exhaustOnUse = true;  // 开启打出时自我消耗开关！

        // 加上它的专属立绘路径
        m_imagePath = ":/resources/images/cards/slimed.png";
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(source); Q_UNUSED(target); Q_UNUSED(relics);
        // 🔴 黏液牌本身没有任何正面效果，打出它纯粹是为了浪费 1 能量把它从手牌里清除喵！
        qDebug() << "[Logic] 玩家花费了 1 点能量，清理了黏液喵！";
    }
};
