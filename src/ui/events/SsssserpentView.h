#pragma once
#include "GenericChoiceEventView.h"

class SsssserpentView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit SsssserpentView(Player* player, CardManager* cardManager, 
                             RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    void onAgree();
    void onDisagree();
    void showEnding(const QString& resultText);
};
