#pragma once
#include "Card.h"
#include "../Player.h"
#include "../StatusManager.h"
#include <QDebug>

class BarricadeCard : public Card {
    Q_OBJECT
public:
    explicit BarricadeCard(QObject* parent = nullptr)
        : Card("card_barricade", QStringLiteral("壁垒"), 3, false, parent) {

        // 🔴 统一架构标准：即便是静态文本，也交给解析器基底！
        m_rawDescription = QStringLiteral("格挡不再在你的回合开始时消失。");
        m_description = m_rawDescription;

        m_type = CardType::Power;
        m_target = CardTarget::None;
        m_imagePath = ":/resources/images/cards/barricade.png";
    }

    void upgrade() override {
        if (!m_isUpgraded) {
            Card::upgrade();
            m_cost = 2; // 🔴 纯净的机制进化，毫无字符串拼接的负担！
            qDebug() << "[Card]" << m_name << "升级完毕！费用已降低为 2 费喵！";
        }
    }

    void play(Player* source, Fighter* target, RelicManager* relics) override {
        Q_UNUSED(target); Q_UNUSED(relics);
        source->getStatusManager()->applyStatus(StatusType::Barricade, 1);
        qDebug() << "[Logic] 不动如山！玩家开启了壁垒形态！";
    }
};
