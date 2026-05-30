#pragma once
#include "EventBaseView.h"
#include "IconButton.h"
#include "TextButton.h"
#include "../carditem.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QPropertyAnimation>

class CampfireView : public EventBaseView {
    Q_OBJECT

public:
    explicit CampfireView(Player* player, CardManager* cardManager,
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;

private:
    void onRest();
    void onUpgrade();
    void showCardSelector();
    void confirmUpgrade();
    void cancelUpgrade();
    void runUpgradeAnimation(Card* card);
    void createCampfireVisual();
    void createRestSmoke();
    QList<Card*> allUpgradableCards() const;

    // 火堆视觉
    QList<QGraphicsItem*> m_fireItems;
    QList<QVariantAnimation*> m_fireAnimations;

    // 选项按钮（图标）
    IconButton* m_restBtn = nullptr;
    IconButton* m_upgradeBtn = nullptr;
    QGraphicsTextItem* m_promptText = nullptr;
    QGraphicsEllipseItem* m_choiceCloud = nullptr;
    QGraphicsTextItem* m_restLabel = nullptr;
    QGraphicsTextItem* m_upgradeLabel = nullptr;

    // 选牌相关
    Card* m_selectedCard = nullptr;
    QList<QGraphicsObject*> m_cardDisplayItems;
    QGraphicsTextItem* m_cardSelectPrompt = nullptr;
    TextButton* m_confirmBtn = nullptr;
    TextButton* m_cancelBtn = nullptr;

    // 休息烟雾
    QList<QGraphicsEllipseItem*> m_smokeParticles;
};
