#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include "../../entities/Player.h"
#include "logic/CardManager.h"
#include "../../entities/relics/RelicManager.h"
#include "../../api/EventAPI.h"
#include "../../api/BattleAPI.h"
#include "LeaveButton.h"
#include "../TopBar.h"

class RelicTray;

class EventBaseView : public QGraphicsView {
    Q_OBJECT

public:
    explicit EventBaseView(Player* player, CardManager* cardManager,
                           RelicManager* relicManager, QWidget* parent = nullptr);
    virtual ~EventBaseView() = default;

signals:
    void eventFinished();
    void requestBattle(BattleContext context);
    void relicObtained(Relic* relic); // 🔴 新增：全域發放遺物訊號！

protected:
    virtual void setupContent(); // 子类在此填充中央内容

    void showDarkOverlay(const QString& text = "");
    void hideDarkOverlay();
    void setLeaveButtonVisible(bool visible);

    // ========================================================
    // 🗑️ 喵娘注：这里的幕布引擎已经被成功剥离！
    // 黑屏切场统一交由 GameWindow 统筹，保持了绝对的低耦合！
    // ========================================================

    QGraphicsScene* m_scene;
    Player* m_player;
    CardManager* m_cardManager;
    RelicManager* m_relicManager;

    QGraphicsPixmapItem* m_playerImage;
    QPixmap m_playerPixmap;

    // 遮罩层（子类可操作文字内容）
    QGraphicsRectItem* m_darkOverlay = nullptr;
    QGraphicsTextItem* m_overlayText = nullptr;

    LeaveButton* m_leaveBtn = nullptr;

private:
    void setupCommonUI();
    QPointF playerImagePos() const;
    QPointF leaveButtonPos() const;
};
