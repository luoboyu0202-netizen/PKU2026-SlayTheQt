// DazedCard.h
#pragma once
#include "Card.h"

class DazedCard : public Card {
public:
    // 🔴 按照父类要求：传入 ID, 名字, 费用(-1), 虚无(true!!!), parent
    explicit DazedCard(QObject* parent = nullptr)
        : Card("card_dazed", QStringLiteral("眩晕"), -1, true, parent) {

        m_description = QStringLiteral("不能打出。虚无。");
        m_type = CardType::Status;
        m_target = CardTarget::None;

        m_isUnplayable = true;

        // 🔴 加上图片路径喵！
        m_imagePath = ":/resources/images/cards/dazed.png";
    }

    void play(Player*, Fighter*, RelicManager*) override {}
};
