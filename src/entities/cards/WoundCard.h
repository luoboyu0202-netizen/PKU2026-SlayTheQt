#pragma once
#include "Card.h"

class WoundCard : public Card {
    Q_OBJECT // 别忘了加上 Q_OBJECT 宏哦喵！
public:
    // 🔴【修正】：在初始化列表里，把档案规规矩矩地交给父类！
    // 参数依次为：ID, 名字, 费用(-1隐藏费用球), 是否虚无, parent
    explicit WoundCard(QObject* parent = nullptr)
        : Card("card_wound", QStringLiteral("伤口"), -1, false, parent) {

        m_description = QStringLiteral("不能打出。");
        m_type = CardType::Status;   // 状态牌
        m_target = CardTarget::None; // 不需要目标

        m_isUnplayable = true; // 🔴 开启不可打出开关
        m_imagePath = ":/resources/images/cards/wound.png"; // 记得换成你的图片路径喵！
    }

    void play(Player*, Fighter*, RelicManager*) override {}
};