#pragma once
#include "EventBaseView.h"
#include "../Carditem.h"
#include <QGraphicsPixmapItem>
#include <QList>
#include <QVariantAnimation>
#include <QWheelEvent> // 🔴 頂部引入正確的滾輪事件類別 // 頂部記得引入

class RelicItem; // 🔴 引入遗物 UI 包装类！
class Card;
class Relic;

class MerchantView : public EventBaseView {
    Q_OBJECT

public:
    explicit MerchantView(Player* player, CardManager* cardManager,
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent();
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void shopDataChanged();
    void relicBought(Relic* newRelic); // 🔴 新增：通知大管家有新遗物肉身加入了！

private:
    void setupPhaseOne();
    void onMerchantClicked();

    void setupPhaseTwo();
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
    // 🔴 换成这俩：
    QGraphicsObject* m_merchantImage = nullptr;
    QGraphicsObject* m_exitBanner = nullptr;

    QGraphicsPixmapItem* m_carpet = nullptr;
    QGraphicsPixmapItem* m_handCursor = nullptr;
    QGraphicsPixmapItem* m_removeButton = nullptr;
    QGraphicsPixmapItem* m_saleTag = nullptr;
    QGraphicsPixmapItem* m_soldoutItem = nullptr;

    QVariantAnimation* m_handAnimX = nullptr;
    QVariantAnimation* m_handAnimY = nullptr;

    CardItem* m_cardSlots[7] = {};
    Card* m_shopCards[7] = {};
    int m_cardPrices[7] = {};
    int m_saleIndex = -1;

    RelicItem* m_relicIcons[3] = {};
    QGraphicsTextItem* m_relicPriceTexts[3] = {};
    QGraphicsTextItem* m_relicNameTexts[3] = {};
    Relic* m_shopRelics[3] = {};
    int m_relicPrices[3] = {};

    QGraphicsTextItem* m_removePriceText = nullptr;

    QGraphicsRectItem* m_relicTooltipBg = nullptr;
    QGraphicsTextItem* m_relicTooltipText = nullptr;

    bool m_cardRemoved = false;
    QGraphicsItem* m_currentHoveredItem = nullptr;

    // Card removal selection state
    bool m_inRemovalMode = false;
    QList<QGraphicsObject*> m_removalCardItems;
    QGraphicsObject* m_confirmRemoveBtn = nullptr;
    QGraphicsObject* m_cancelRemoveBtn = nullptr;

protected:
    // 🔴 新增：攔截滑鼠滾輪事件
    void wheelEvent(QWheelEvent* event) override;

private:
    // 🔴 新增：滾動引擎核心變數
    qreal m_removalScrollY = 0.0;
    qreal m_maxRemovalScrollY = 0.0;

    // 🔴 新增：刷新卡牌位置的專屬函數
    void updateRemovalCardPositions();
};
