#pragma once
#include "GenericChoiceEventView.h"
#include <functional>

class CardItem;
class TextButton;

class SelfNoteView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit SelfNoteView(Player* player, CardManager* cardManager, 
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent();

private:
    void onTakeAndGiveChosen();
    void onIgnoreChosen();

    void startCardSelection();
    void confirmStorage(Card* card);
    void cancelSelection();

    void showEnding(const QString& resultText);

    // 状态追踪
    QList<CardItem*> m_selectionCardItems;
    TextButton* m_confirmBtn = nullptr;
    TextButton* m_cancelBtn = nullptr;
    
    QString m_storedCardId;
    bool m_isStoredCardUpgraded;
};
