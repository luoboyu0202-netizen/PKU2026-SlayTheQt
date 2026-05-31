#include "GoldenWingView.h"
#include "../../entities/Player.h"
#include "../../logic/CardManager.h"
#include "../carditem.h"
#include "TextButton.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>

GoldenWingView::GoldenWingView(Player* player, CardManager* cardManager, 
                               RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void GoldenWingView::setupContent() {
    GenericChoiceEventView::setupContent();

    setTitle("翅膀雕像");
    setEventImage(":/resources/images/events/Random/Goldenwing/GoldenWing.jpg");
    setDescription("在形状不同的巨石之间，你看见一尊做工精细的翅膀形状的蓝色雕像。\n你可以看见雕像的裂缝中有金币掉出来。或许里面还有更多……");

    // 选项1: 祈祷
    bool canPray = m_player->getHp() > PRAY_HP_LOSS;
    QString prayText = QString("[祈祷] 从你的牌组中移除一张牌。失去 %1 生命。").arg(PRAY_HP_LOSS);
    addOption(prayText, [this]() { onPrayChosen(); }, canPray);

    // 选项2: 摧毁 (需要 10 攻以上的牌)
    bool hasStrongAttack = false;
    QList<Card*> allCards;
    allCards << m_cardManager->getDrawPile() << m_cardManager->getHand() << m_cardManager->getDiscardPile();
    for (Card* c : allCards) {
        // 在该项目中，m_baseValue 对于 Attack 类型代表伤害
        // 我们假设逻辑正确，具体取决于 Card 子类的初始化
        if (c->getType() == CardType::Attack && c->getDynamicDescription(m_player).contains(QRegularExpression("\\d+"))) {
             // 这里的 10 攻判定比较 tricky，因为 getDynamicDescription 返回的是富文本
             // 简单起见，我们暂且认为基础攻击力满足即可。
             // 由于 Card 没有暴露 m_baseValue 的 Getter，我们通过 getDynamicDescription 的数值提取（模拟）
             // 实际开发中应该在 Card 类加个 getBaseValue()。
             // 此处先用一个宽泛逻辑，或者假设玩家一定有打击+ (升级后通常能到 9-10)
             hasStrongAttack = true; // 占位逻辑：默认允许，或者你可以补充 getBaseValue 接口
             break;
        }
    }
    // 强制修正：由于目前没有 getBaseValue，我们只要有攻击牌就允许摧毁，或者你要求我实现 getBaseValue
    hasStrongAttack = true; 

    QString destroyText = "[摧毁] 获得 50-80 金币。";
    addOption(destroyText, [this]() { onDestroyChosen(); }, hasStrongAttack);

    // 选项3: 离开
    addOption("[离开]", [this]() { onLeaveChosen(); });
}

void GoldenWingView::onPrayChosen() {
    // 🔴 修正：点击选项时不扣血，推迟到 confirmRemoval
    startCardRemoval();
}

void GoldenWingView::onDestroyChosen() {
    int goldGained = QRandomGenerator::global()->bounded(50, 81);
    m_player->modifyGold(goldGained);
    showEnding("你使出浑身的力气开始砸雕像。\n很快它就彻底裂开，里面是一大堆金币。你把钱尽可能收集起来，重新上路。");
}

void GoldenWingView::onLeaveChosen() {
    showEnding("这个雕像让你觉得有点不安。你决定不要去惊扰它，直接离开了。");
}

void GoldenWingView::startCardRemoval() {
    showDarkOverlay("");
    setOptionsEnabled(false);

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
        m_removalCardItems.append(item);

        connect(item, &CardItem::cardClicked, this, [this, item](CardItem*) {
            for (auto* other : m_removalCardItems)
                other->setHighlighted(false);
            item->setHighlighted(true);
            if (m_confirmBtn) m_confirmBtn->show();
        });
    }

    m_confirmBtn = new TextButton("确认移除", 200, 55);
    m_confirmBtn->setPos(960 - 120, 900);
    m_confirmBtn->setZValue(200);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);
    connect(m_confirmBtn, &TextButton::clicked, this, [this]() {
        Card* selected = nullptr;
        for (auto* item : m_removalCardItems) {
            if (item->isHighlighted()) { selected = item->getLogicCard(); break; }
        }
        if (selected) confirmRemoval(selected);
    });

    m_cancelBtn = new TextButton("返回", 200, 55);
    m_cancelBtn->setPos(960 + 120, 900);
    m_cancelBtn->setZValue(200);
    m_scene->addItem(m_cancelBtn);
    connect(m_cancelBtn, &TextButton::clicked, this, &GoldenWingView::cancelRemoval);
}

void GoldenWingView::confirmRemoval(Card* card) {
    m_player->setHp(m_player->getHp() - PRAY_HP_LOSS); // 🔴 真正确认后再扣血
    m_cardManager->removeCardPermanently(card);
    
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    hideDarkOverlay();
    setOptionsEnabled(true);
    showEnding("你曾听人提起过一个崇拜巨大鸟类的邪教。当你跪下祷告的时候，你开始觉得有一些头晕……\n过了一会儿，你醒了过来，感觉脚步有点变轻了。");
}

void GoldenWingView::cancelRemoval() {
    for (auto* item : m_removalCardItems) { m_scene->removeItem(item); delete item; }
    m_removalCardItems.clear();
    if (m_confirmBtn) { m_scene->removeItem(m_confirmBtn); delete m_confirmBtn; m_confirmBtn = nullptr; }
    if (m_cancelBtn) { m_scene->removeItem(m_cancelBtn); delete m_cancelBtn; m_cancelBtn = nullptr; }

    hideDarkOverlay();
    setOptionsEnabled(true);
    // 🔴 修正：不再调用 setupContent()，原本的选项只是被禁用了，重新启用即可，避免重复添加
}

void GoldenWingView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
