#pragma once
#include "EventBaseView.h"
#include "IconButton.h"
#include "TextButton.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QPropertyAnimation>
#include "../Carditem.h" // (注意路径可能需要根据你的实际目录调整)

class CampfireView : public EventBaseView {
    Q_OBJECT

public:
    explicit CampfireView(Player* player, CardManager* cardManager,
                          RelicManager* relicManager, QWidget* parent = nullptr);

protected:
    void setupContent();
    // 拦截卡牌点击事件的过滤器
    bool eventFilter(QObject* obj, QEvent* event) override;

    // ========================================================
    // 🌟 新增：拦截鼠标滚轮事件，实现只滚卡牌不滚背景的 3A 级交互！
    // ========================================================
    void wheelEvent(QWheelEvent *event) override;

signals:
    void playerStatusChanged(); // 睡觉回血后通知顶栏刷新！
    void deckUpdated();         // 锻造卡牌后通知顶栏刷新总牌库！

private:
    void onRest();
    void onUpgrade();
    void showCardSelector(const QList<Card*>& candidates);
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
