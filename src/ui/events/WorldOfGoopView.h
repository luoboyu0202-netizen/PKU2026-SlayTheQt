#pragma once
#include "GenericChoiceEventView.h"

class WorldOfGoopView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit WorldOfGoopView(Player* player, CardManager* cardManager, 
                             RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    void onGatherGold();
    void onLeaveItBe(int loseGold);
    void showEnding(const QString& resultText);
};
