#include "OrichalcumRelic.h"
#include "PenNibRelic.h"

// OrichalcumRelic Implementation
OrichalcumRelic::OrichalcumRelic(QObject* parent)
    : Relic("relic_orichalcum", QStringLiteral("奥利哈钢"),
            QStringLiteral("如果你的回合结束时没有 格挡 ，获得 6 点 格挡 。"),
            parent) {
    m_counter = -1;
}

void OrichalcumRelic::onTurnEnd() {
    // 简化实现以便测试，不依赖复杂的 BattleEngine 实例
    if (m_player) {
        if (m_player->getBlock() == 0) {
            emit relicActivated();
            m_player->addBlock(6);
        }
    }
}

// PenNibRelic Implementation
PenNibRelic::PenNibRelic(QObject* parent)
    : Relic("relic_pen_nib", QStringLiteral("钢笔尖"),
            QStringLiteral("你每打出 10 张 攻击 牌，下一次攻击造成 双倍 伤害。"), 
            parent) {
    m_counter = 0;
}

int PenNibRelic::modifyAttackDamage(int currentDamage) {
    if (m_counter == 9) return currentDamage * 2;
    return currentDamage;
}

void PenNibRelic::onCardPlayed(Card* card) {
    if (card && card->getType() == CardType::Attack) {
        if (m_counter == 9) {
            setCounter(0);
            emit relicActivated();
        } else {
            setCounter(m_counter + 1);
        }
    }
}
