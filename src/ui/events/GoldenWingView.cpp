#include "GoldenWingView.h"
#include "../../entities/Player.h"
#include "../../logic/CardManager.h"
#include "../carditem.h"
#include "TextButton.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTimer> // 🔴 必须引入 QTimer 以支持异步渲染！

GoldenWingView::GoldenWingView(Player* player, CardManager* cardManager,
                               RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void GoldenWingView::setupContent() {
    // ========================================================
    // 🔴 修复 1：在最开头执行大扫除，杜绝重复调用时选项翻倍！
    // ========================================================
    clearOptions();

    GenericChoiceEventView::setupContent();

    setTitle("翅膀雕像");
    setEventImage(":/resources/images/events/Random/Goldenwing/GoldenWing.jpg");
    setDescription("在形状不同的巨石之间，你看见一尊做工精细的翅膀形状的蓝色雕像。\n你可以看见雕像的裂缝中有金币掉出来。或许里面还有更多……");

    // 选项1: 祈祷
    bool canPray = m_player->getHp() > PRAY_HP_LOSS;
    QString prayText = QString("[祈祷] 从你的牌组中移除一张牌。失去 %1 生命。").arg(PRAY_HP_LOSS);
    addOption(prayText, [this]() { onPrayChosen(); }, canPray);

    // 选项2: 摧毁 (由于项目中暂时没有 getBaseValue()，默认允许摧毁)
    bool hasStrongAttack = true;

    QString destroyText = "[摧毁] 获得 50-80 金币。";
    addOption(destroyText, [this]() { onDestroyChosen(); }, hasStrongAttack);

    // 选项3: 离开
    addOption("[离开]", [this]() { onLeaveChosen(); });
}

void GoldenWingView::onPrayChosen() {
    // 🔴 点击选项时不扣血，推迟到 confirmRemoval
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

    // ========================================================
    // 🔴 修复 2：异步渲染结界！让出主线程 50 毫秒，拒绝卡牌抢跑！
    // ========================================================
    QTimer::singleShot(50, this, [this]() {
        QList<Card*> removable;
        removable.append(m_cardManager->getDrawPile());
        removable.append(m_cardManager->getHand());
        removable.append(m_cardManager->getDiscardPile());

        if (removable.isEmpty()) {
            hideDarkOverlay();
            setOptionsEnabled(true); // 如果没牌可删，直接恢复选项
            return;
        }

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
    });
}

void GoldenWingView::confirmRemoval(Card* card) {
    // ========================================================
    // 🔴 修复 3：救命步骤，先清理 UI 切断联系，再处理底层逻辑！
    // ========================================================
    for (auto* item : m_removalCardItems) {
        m_scene->removeItem(item);
        item->deleteLater(); // 绝对不要用 delete
    }
    m_removalCardItems.clear();

    if (m_confirmBtn) {
        m_scene->removeItem(m_confirmBtn);
        m_confirmBtn->deleteLater();
        m_confirmBtn = nullptr;
    }
    if (m_cancelBtn) {
        m_scene->removeItem(m_cancelBtn);
        m_cancelBtn->deleteLater();
        m_cancelBtn = nullptr;
    }

    hideDarkOverlay();
    setOptionsEnabled(true);

    // UI 彻底剥离后，执行底层的杀牌和扣血动作
    m_player->setHp(m_player->getHp() - PRAY_HP_LOSS); // 真正确认后再扣血
    m_cardManager->removeCardPermanently(card);

    showEnding("你曾听人提起过一个崇拜巨大鸟类的邪教。当你跪下祷告的时候，你开始觉得有一些头晕……\n过了一会儿，你醒了过来，感觉脚步有点变轻了。");
}

void GoldenWingView::cancelRemoval() {
    // 🔴 同上，优雅地超度 UI
    for (auto* item : m_removalCardItems) {
        m_scene->removeItem(item);
        item->deleteLater();
    }
    m_removalCardItems.clear();

    if (m_confirmBtn) {
        m_scene->removeItem(m_confirmBtn);
        m_confirmBtn->deleteLater();
        m_confirmBtn = nullptr;
    }
    if (m_cancelBtn) {
        m_scene->removeItem(m_cancelBtn);
        m_cancelBtn->deleteLater();
        m_cancelBtn = nullptr;
    }

    hideDarkOverlay();
    setOptionsEnabled(true);
    // 🔴 不再调用 setupContent()，避免选项翻倍和重叠！
}

void GoldenWingView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}