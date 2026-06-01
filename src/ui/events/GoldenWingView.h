#pragma once
#include "GenericChoiceEventView.h"

class CardItem;
class TextButton;

class GoldenWingView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit GoldenWingView(Player* player, CardManager* cardManager, 
                            RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent();

private:
    void onPrayChosen();
    void onDestroyChosen();
    void onLeaveChosen();

    void startCardRemoval();
    void confirmRemoval(Card* card);
    void cancelRemoval();

    void showEnding(const QString& resultText);

    // 状态追踪
    QList<CardItem*> m_removalCardItems;
    TextButton* m_confirmBtn = nullptr;
    TextButton* m_cancelBtn = nullptr;

    const int PRAY_HP_LOSS = 7;
};
