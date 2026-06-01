#pragma once
#include "GenericChoiceEventView.h"

class CardItem;
class TextButton;

class ClericView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit ClericView(Player* player, CardManager* cardManager, 
                        RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent();

private:
    void onHealChosen();
    void onPurifyChosen();
    void onLeaveChosen();
    
    // 移除卡牌相关的逻辑 (同步自 MerchantView)
    void startCardRemoval();
    void confirmRemoval(Card* card);
    void cancelRemoval();

    void showEnding(const QString& resultText);

    // 状态追踪
    QList<CardItem*> m_removalCardItems;
    TextButton* m_confirmRemoveBtn = nullptr;
    TextButton* m_cancelRemoveBtn = nullptr;

    const int HEAL_COST = 35;
    const int PURIFY_COST = 50;
};
