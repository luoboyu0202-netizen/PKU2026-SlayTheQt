#pragma once
#include "GenericChoiceEventView.h"

class CardItem;
class TextButton;

class DesignerView : public GenericChoiceEventView {
    Q_OBJECT
public:
    explicit DesignerView(Player* player, CardManager* cardManager, 
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    // 选项分支
    void onAdjustChosen();  // 小修
    void onCleanChosen();   // 清洁
    void onFullServiceChosen(); // 全套
    void onPunchChosen();   // 揍他

    // 交互逻辑
    void startCardRemoval(std::function<void(Card*)> onConfirmed);
    void cancelRemoval();

    void showEnding(const QString& resultText);

    // 状态追踪
    QList<CardItem*> m_removalCardItems;
    TextButton* m_confirmRemoveBtn = nullptr;
    TextButton* m_cancelRemoveBtn = nullptr;

    // 常量
    const int ADJUST_COST = 50;
    const int CLEAN_COST = 75;
    const int FULL_COST = 110;
    const int PUNCH_HP_LOSS = 3;
};
