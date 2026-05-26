#pragma once
#include <QObject>
#include <QList>
#include "CardItem.h"
#include "BattleScene.h"
#include "../entities/Player.h"
#include "../logic/CardManager.h"

class HandLayoutManager : public QObject {
    Q_OBJECT
public:
    explicit HandLayoutManager(BattleScene* scene, Player* player, CardManager* cardManager, QObject* parent = nullptr);

    // 暴露所有的 UI 卡牌，方便引擎去连接信号
    const QList<CardItem*>& getHandItems() const { return m_handItems; }

public slots:
    void onCardDrawn(Card* logicCard);
    void onCardVisualDestroyed(CardItem* item);
    void onEnergyChanged(int current, int max);
    // 在 public slots 区域增加：
    void onCardDiscarded(Card* logicCard);
    // ========================================================
    // 🔴【新增】：专门处理卡牌被消耗的视觉响应！
    // ========================================================
    void onCardExhausted(Card* logicCard);

    // 🔴 结界解除时的善后工作
    void onSelectionModeEnded();

signals:
    // 将卡牌图元的打出请求转发给上层引擎
    void cardPlayedRequest(Card* card, Enemy* target);

private:
    void recalculateLayout(); // 核心算法：重排扇形

    BattleScene* m_scene;
    Player* m_player;
    CardManager* m_cardManager;
    QList<CardItem*> m_handItems;
};
