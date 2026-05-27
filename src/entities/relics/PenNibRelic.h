#pragma once
#include "Relic.h"
#include "../cards/card.h"

class PenNibRelic : public Relic {
    Q_OBJECT
public:
    explicit PenNibRelic(QObject* parent = nullptr);
    int modifyAttackDamage(int currentDamage) override;
    void onCardPlayed(Card* card) override;
};
