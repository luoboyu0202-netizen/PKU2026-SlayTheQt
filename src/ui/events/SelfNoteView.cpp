#include "SelfNoteView.h"
#include "../../entities/Player.h"
#include "../../logic/CardManager.h"
#include "../../logic/CardFactory.h"
#include "../../logic/GlobalSaveData.h"
#include "../carditem.h"
#include "TextButton.h"
#include <QDebug>

SelfNoteView::SelfNoteView(Player* player, CardManager* cardManager, 
                           RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    auto* save = GlobalSaveData::getInstance();
    m_storedCardId = save->storedCardId;
    m_isStoredCardUpgraded = save->isStoredCardUpgraded;
    
    setupContent();
}

void SelfNoteView::setupContent() {
    clearOptions();
    GenericChoiceEventView::setupContent();

    setTitle("留给自己的讯息");
    setEventImage(":/resources/images/events/Random/SelfNote/SelfNote.jpg");
    setDescription("一根柱子吸引了你的注意，你发现上面有一块松动的石块。\n"
                   "搬动砖块，你在里面找到了一张折起来的纸条和一张牌，纸条上写着：“高塔之心在等待。”\n"
                   "这……是你自己的笔迹。");

    // 获取存放卡牌的名称
    Card* temp = CardFactory::createCard(m_storedCardId, this);
    if (m_isStoredCardUpgraded) temp->upgrade();
    QString cardDisplayName = temp->getName();
    delete temp;

    addOption(QString("[取之与之] 获得 %1，然后存放一张牌。").arg(cardDisplayName), 
              [this]() { onTakeAndGiveChosen(); });
    addOption("[无视]", [this]() { onIgnoreChosen(); });
}

void SelfNoteView::onTakeAndGiveChosen() {
    // 1. 获得存放的牌
    Card* gained = CardFactory::createCard(m_storedCardId, this);
    if (m_isStoredCardUpgraded) gained->upgrade();
    m_cardManager->addCardToDiscardPile(gained);

    // 2. 进入存牌流程 (移除一张牌)
    startCardSelection();
}

void SelfNoteView::onIgnoreChosen() {
    showEnding("这究竟是怎么回事？");
}

void SelfNoteView::startCardSelection() {
    showDarkOverlay("");
    setOptionsEnabled(false);

    // ========================================================
    // 🔴 同样的异步渲染魔法
    // ========================================================
    QTimer::singleShot(50, this, [this]() {
        QList<Card*> removable;
        removable.append(m_cardManager->getDrawPile());
        removable.append(m_cardManager->getHand());
        removable.append(m_cardManager->getDiscardPile());

    const int cols = 5;
    const qreal cardW = 150, cardH = 220;
    const qreal startX = 260, startY = 250; 
    
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
        m_selectionCardItems.append(item);

        connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
            for (auto* other : m_selectionCardItems)
                other->setHighlighted(false);
            item->setHighlighted(true);
            if (m_confirmBtn) m_confirmBtn->show();
        });
    }

    m_confirmBtn = new TextButton("确认存放", 200, 55);
    m_confirmBtn->setPos(960 - 120, 900);
    m_confirmBtn->setZValue(200);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
    connect(m_confirmBtn, &TextButton::clicked, this, [this]() {
        Card* selected = nullptr;
        for (auto* item : m_selectionCardItems) {
            if (item->isHighlighted()) { selected = item->getLogicCard(); break; }
        }
        if (selected) confirmStorage(selected);
    });

    m_cancelBtn = new TextButton("返回", 200, 55);
    m_cancelBtn->setPos(960 + 120, 900);
    m_cancelBtn->setZValue(200);
    m_scene->addItem(m_cancelBtn);
    connect(m_cancelBtn, &TextButton::clicked, this, &SelfNoteView::cancelSelection);
    });
}

void SelfNoteView::confirmStorage(Card* card) {
    // 更新全局存档
    auto* save = GlobalSaveData::getInstance();
    save->storedCardId = card->getId();
    save->isStoredCardUpgraded = card->isUpgraded();

    // 移除玩家当前的牌
    m_cardManager->removeCardPermanently(card);

    // 清理界面
    for (auto* item : m_selectionCardItems) { m_scene->removeItem(item); delete item; }
    m_selectionCardItems.clear();
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    hideDarkOverlay();
    setOptionsEnabled(true);
    showEnding("这究竟是怎么回事？");
}

void SelfNoteView::cancelSelection() {
    for (auto* item : m_selectionCardItems) { m_scene->removeItem(item); delete item; }
    m_selectionCardItems.clear();
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    hideDarkOverlay();
    setOptionsEnabled(true);
}

void SelfNoteView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
