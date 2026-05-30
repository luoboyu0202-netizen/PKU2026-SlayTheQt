#pragma once
#include "EventBaseView.h"
#include "../carditem.h"
#include <QGraphicsPixmapItem>
#include <QList>
#include <QVariantAnimation>

class Card;
class Relic;

class MerchantView : public EventBaseView {
    Q_OBJECT

public:
    explicit MerchantView(Player* player, CardManager* cardManager,
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void setupPhaseOne();
    void onMerchantClicked();

    void setupPhaseTwo();
    void setupPhaseTwoReenter();
    void generateShopItems();
    void layoutShopItems();
    void refreshAffordability();

    void updateHandCursor(const QPointF& mouseScenePos);
    void showRelicTooltip(int index);
    void hideRelicTooltip();
    void moveHandToItem(QGraphicsItem* item);
    void moveHandOffScreen();
    QGraphicsItem* findHoveredItem(const QPointF& scenePos);

    void onCardClicked(Card* card, int price);
    void onRelicClicked(int index);
    void onRemoveClicked();
    void onExitClicked();
    void startCardRemoval();
    void confirmRemoval(Card* card);
    void cancelRemoval();
    void playPurchaseEffect(QGraphicsItem* item, const QPointF& center);

    enum class Phase { Encounter, Shopping };
    Phase m_phase = Phase::Encounter;

    QGraphicsPixmapItem* m_bgItem = nullptr;
    QGraphicsPixmapItem* m_merchantImage = nullptr;

    QGraphicsPixmapItem* m_carpet = nullptr;
    QGraphicsPixmapItem* m_handCursor = nullptr;
    QGraphicsPixmapItem* m_exitBanner = nullptr;
    QGraphicsPixmapItem* m_removeButton = nullptr;
    QGraphicsPixmapItem* m_saleTag = nullptr;
    QGraphicsPixmapItem* m_soldoutItem = nullptr;

    QVariantAnimation* m_handAnimX = nullptr;
    QVariantAnimation* m_handAnimY = nullptr;

    CardItem* m_cardSlots[7] = {};
    Card* m_shopCards[7] = {};
    int m_cardPrices[7] = {};
    int m_saleIndex = -1;

    QGraphicsPixmapItem* m_relicIcons[3] = {};
    QGraphicsTextItem* m_relicPriceTexts[3] = {};
    QGraphicsTextItem* m_relicNameTexts[3] = {};
    Relic* m_shopRelics[3] = {};
    int m_relicPrices[3] = {};

    QGraphicsTextItem* m_removePriceText = nullptr;

    QGraphicsRectItem* m_relicTooltipBg = nullptr;
    QGraphicsTextItem* m_relicTooltipText = nullptr;

    bool m_cardRemoved = false;
    bool m_shopGenerated = false;
    QGraphicsItem* m_currentHoveredItem = nullptr;

    // Card removal selection state
    bool m_inRemovalMode = false;
    QList<QGraphicsObject*> m_removalCardItems;
    QGraphicsObject* m_confirmRemoveBtn = nullptr;
    QGraphicsObject* m_cancelRemoveBtn = nullptr;
};
