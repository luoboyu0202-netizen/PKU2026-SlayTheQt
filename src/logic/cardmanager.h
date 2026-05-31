#pragma once
#include <QObject>
#include <QList>
#include "cards/Card.h"
#include <QDebug>
#include <QTimer>


class CardManager : public QObject {
    Q_OBJECT

public:
    explicit CardManager(QObject* parent = nullptr);
    virtual ~CardManager() = default;

    // 战斗初始化：将卡组放入抽牌堆并洗牌
    void initializeDeck(const QList<Card*>& masterDeck);



    // 核心流转动作
    void drawCards(int amount);
    void endTurnProcess(); // 触发回合结束结算（虚无消耗，其余弃牌）

    // 🔴【新增】：消耗一张指定的手牌！
    void exhaustCard(Card* card);

    // 单卡特定动作（打出卡牌时调用）
    void moveToDiscard(Card* card);
    void moveToExhaust(Card* card);

    // 状态获取
    int getDrawPileCount() const { return m_drawPile.size(); }
    int getHandCount() const { return m_hand.size(); }
    int getDiscardPileCount() const { return m_discardPile.size(); }
    int getExhaustPileCount() const { return m_exhaustPile.size(); }
    const QList<Card*>& getHand() const { return m_hand; }
    // 🔴【新增】：获取消耗堆（墓地）的卡牌列表
    const QList<Card*>& getExhaustPile() const { return m_exhaustPile; }
    // ========================================================
    // 🟢【新增】：获取抽牌堆和弃牌堆的钥匙！
    // ========================================================
    const QList<Card*>& getDrawPile() const { return m_drawPile; }
    const QList<Card*>& getDiscardPile() const { return m_discardPile; }

    // 🔴【新增】：能力牌打出后的永久存放区（仅限本场战斗记录，不参与洗牌）
    QList<Card*> m_playedPowers;

    void addCardToDiscardPile(Card* newCard);
    void shuffleDiscardToDraw();
    void emitPileCounts();
    // 通知 UI 有卡牌被强行塞入了弃牌堆！

    void moveToPowerZone(Card* card) {
        if (!card) return;

        // 1. 🔴 极其重要：从逻辑手牌列表中彻底注销它！
        m_hand.removeOne(card);

        // 2. 放入能力虚空区
        m_playedPowers.append(card);

        qDebug() << "[CardManager] 能力牌已融入体内，手牌户口已注销：" << card->getName();

        // 3. 🔴 呼叫 UI：原版肉体可以火化了！
        emit cardMovedToPower(card);
    }

    // 🔴【新增】：强行从抽牌堆顶部拿出一张牌（不进手牌，直接返回给你！）
    Card* popTopDrawPile();

    // 允许不检查是否在手牌中，直接强行移动
    void forceMoveToDiscard(Card* card);
    void forceMoveToExhaust(Card* card);

    // 🔴【新增】：通知所有手牌重新计算伤害并刷新 UI！
    void refreshHandDynamicText();

    // 🔴【商店用】：永久删除一张卡牌（从所有堆中移除）
    void removeCardPermanently(Card* card);


signals:
    // UI 监听这些信号来播放飞行动画和更新数字贴图
    void cardDrawn(Card* card);
    void cardDiscarded(Card* card);
    void pileCountsChanged(int drawCount, int discardCount, int exhaustCount);
    void deckShuffled(); // 洗牌广播

    // 🔴【新增】：通知 UI 这张牌被烧了，播放燃烧灰烬特效！
    void cardExhausted(Card* card);
    void cardInsertedToDiscard(Card* card);
    // 🔴 宣告：一张能力牌已进入虚空，UI 请销毁原肉体！
    void cardMovedToPower(Card* card);

    void handTextNeedsUpdate();

private:


    QList<Card*> m_masterDeck;  // 总牌库（只读备份，战斗中不碰它）
    QList<Card*> m_drawPile;    // 抽牌堆（会被洗乱，随时减少）
    QList<Card*> m_hand;        // 玩家手牌
    QList<Card*> m_discardPile; // 弃牌堆
    // 🔴【新增】：消耗堆（卡牌墓地）
    QList<Card*> m_exhaustPile;
};
