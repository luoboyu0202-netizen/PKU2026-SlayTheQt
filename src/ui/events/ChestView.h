#pragma once

#include "EventBaseView.h"
#include <QGraphicsPixmapItem>
#include <QList>

class RelicPopupWidget;
class Relic;

class ChestView : public EventBaseView {
    Q_OBJECT

public:
    explicit ChestView(Player* player, RelicManager* relicManager,
                       QWidget* parent = nullptr);

protected:
    void setupContent();
    void mousePressEvent(QMouseEvent* event) override;

private:
    void onChestClicked();
    void onTakeRelic(Relic* relic);
    void onSkipRelic();
    void showResult();

    QGraphicsPixmapItem* m_bgItem = nullptr;
    QGraphicsPixmapItem* m_chestItem = nullptr;
    QPixmap m_chestOpenPixmap;
    QList<QGraphicsEllipseItem*> m_sparkleParticles;
    RelicPopupWidget* m_relicPopup = nullptr;
    Relic* m_offeredRelic = nullptr;
    bool m_chestOpened = false;
    void playFlightEffect(Relic* relic, const QPointF& startPos);

signals:
    // 🔴 新增信號：通知 GameWindow 寶箱發放了遺物，趕緊讓頂欄顯示！
    void relicObtained(Relic* relic);
};
