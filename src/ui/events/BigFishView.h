#pragma once
#include "GenericChoiceEventView.h"

class BigFishView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit BigFishView(Player* player, CardManager* cardManager, 
                         RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    void onBananaChosen();
    void onDonutChosen();
    void onBoxChosen();
    void showEnding(const QString& resultText);
};
