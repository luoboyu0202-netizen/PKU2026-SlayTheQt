#pragma once
#include "Relic.h"
#include "../Player.h"

class OrichalcumRelic : public Relic {
    Q_OBJECT
public:
    explicit OrichalcumRelic(QObject* parent = nullptr);
    void onTurnEnd() override;

protected:
    Player* m_player = nullptr; // 用于测试，实际应从上下文获取
};
