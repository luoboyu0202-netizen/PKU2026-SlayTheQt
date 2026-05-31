#include "ClericView.h"
#include "../../entities/Player.h"
#include "../../logic/CardManager.h"
#include "../carditem.h"
#include "TextButton.h"
#include <QDebug>

ClericView::ClericView(Player* player, CardManager* cardManager, 
                       RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void ClericView::setupContent() {
    GenericChoiceEventView::setupContent();

    setTitle("牧师");
    setEventImage(":/resources/images/events/Random/Cleric/Cleric.jpg");
    setDescription("一个戴着金头盔（？）的奇怪蓝色人形生物脸上带着大大的微笑走到了你面前。\n“你好啊朋友！我是牧师！你想不想试试我的服务呐？！”那个生物大声喊叫起来。");

    // 选项1: 治疗
    bool canAffordHeal = m_player->getGold() >= HEAL_COST;
    QString healText = QString("[治疗] %1 金币：回复 25% 生命。").arg(HEAL_COST);
    if (!canAffordHeal) healText += " (金币不足)";
    addOption(healText, [this]() { onHealChosen(); }, canAffordHeal);

    // 选项2: 净化
    bool canAffordPurify = m_player->getGold() >= PURIFY_COST;
    QString purifyText = QString("[净化] %1 金币：从你的牌组中移除一张牌。").arg(PURIFY_COST);
    if (!canAffordPurify) purifyText += " (金币不足)";
    addOption(purifyText, [this]() { onPurifyChosen(); }, canAffordPurify);

    // 选项3: 离开
    addOption("[离开]", [this]() { onLeaveChosen(); });
}

void ClericView::onHealChosen() {
    m_player->modifyGold(-HEAL_COST);
    int healAmount = m_player->getMaxHp() * 0.25;
    m_player->heal(healAmount);
    showEnding("一道温暖的<b>金光</b>笼罩了你然后消散了。\n那个生物咧嘴一笑：“牧师最强奶妈。那么祝你愉快啦！”");
}

void ClericView::onPurifyChosen() {
    m_player->modifyGold(-PURIFY_COST);
    startCardRemoval();
}

void ClericView::startCardRemoval() {
    showDarkOverlay(""); // 🔴 移除提示文字，保持纯净
    setOptionsEnabled(false); // 🔴 禁用底层所有事件按钮
    
    QList<Card*> removable;
    removable.append(m_cardManager->getDrawPile());
    removable.append(m_cardManager->getHand());
    removable.append(m_cardManager->getDiscardPile());

    if (removable.isEmpty()) {
        hideDarkOverlay();
        showEnding("你身上似乎没有什么可以净化的东西……");
        return;
    }

    const int cols = 5;
    const qreal cardW = 150, cardH = 220;
    const qreal startX = 260, startY = 250; // 🔴 对齐商店坐标
    
    for (int i = 0; i < removable.size(); ++i) {
        auto* item = new CardItem(removable[i]);
        item->setSelectionEnabled(true);
        int col = i % cols;
        int row = i / cols;
        QPointF targetPos(startX + col * (cardW + 20), startY + row * (cardH + 20));
        item->setPos(targetPos);
        item->setHomeState(targetPos, 0.0);
        item->setZValue(200); 
        m_scene->addItem(item);
        m_removalCardItems.append(item);

        connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
            for (auto* other : m_removalCardItems)
                other->setHighlighted(false);
            item->setHighlighted(true);
            if (m_confirmRemoveBtn) m_confirmRemoveBtn->show();
        });
    }

    // 确认按钮
    m_confirmRemoveBtn = new TextButton("确认移除", 200, 55);
    m_confirmRemoveBtn->setPos(960 - 120, 900);
    m_confirmRemoveBtn->setZValue(200);
    m_confirmRemoveBtn->hide();
    m_scene->addItem(m_confirmRemoveBtn);
    connect(m_confirmRemoveBtn, &TextButton::clicked, this, [this]() {
        Card* selected = nullptr;
        for (auto* item : m_removalCardItems) {
            if (item->isHighlighted()) { selected = item->getLogicCard(); break; }
        }
        if (selected) confirmRemoval(selected);
    });

    // 取消按钮
    m_cancelRemoveBtn = new TextButton("取消", 200, 55);
    m_cancelRemoveBtn->setPos(960 + 120, 900);
    m_cancelRemoveBtn->setZValue(200);
    m_scene->addItem(m_cancelRemoveBtn);
    connect(m_cancelRemoveBtn, &TextButton::clicked, this, &ClericView::cancelRemoval);
}

void ClericView::confirmRemoval(Card* card) {
    m_cardManager->removeCardPermanently(card);
    
    // 清理
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    setOptionsEnabled(true); // 🔴 重新启用底层按钮
    showEnding("一道寒冷的<b>蓝色火焰</b>笼罩了你然后消散了。\n那个生物咧嘴一笑：“牧师就是能干。那么祝你愉快啦！”");
}

void ClericView::cancelRemoval() {
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmRemoveBtn) { m_scene->removeItem(m_confirmRemoveBtn); delete m_confirmRemoveBtn; m_confirmRemoveBtn = nullptr; }
    if (m_cancelRemoveBtn) { m_scene->removeItem(m_cancelRemoveBtn); delete m_cancelRemoveBtn; m_cancelRemoveBtn = nullptr; }

    hideDarkOverlay();
    // 恢复原本的选项
    setupContent(); 
}

void ClericView::onLeaveChosen() {
    showEnding("你完全不相信这个“牧师”，就这样走开了。");
}

void ClericView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
