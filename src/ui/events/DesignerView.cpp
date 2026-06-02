#include "DesignerView.h"
#include "../../entities/Player.h"
#include "../../logic/CardManager.h"
#include "../carditem.h"
#include "TextButton.h"
#include <QDebug>

DesignerView::DesignerView(Player* player, CardManager* cardManager, 
                           RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void DesignerView::setupContent() {
    clearOptions();
    GenericChoiceEventView::setupContent();

    setTitle("尖端设计师");
    setEventImage(":/resources/images/events/Random/Designer/Designer.jpg");
    setDescription("你发现一家五彩斑斓的店，横幅上挂着大大的“尖端”两个字，就走进去想看看里面有什么。\n"
                   "“等等，别，不行，你不能进来！”\n"
                   "一个穿着打扮无比荒唐的男人出现在门口把你拦了下来\n"
                   "“不行不行，这怎么能行呢，哎呀，你这算是什么风格？令人作呕！什么，你还流流，流着血？哎呀好恶心啊。啊？你问我是做什么生意的？？你是顾客吗？好吧，哎呀真是没办法啦。”\n"
                   "他夸张地叹了一口气，伸手指向一张服务目录。\n"
                   "服务内容看起来还挺正常的，但你现在一心指向对着这自我感觉良好的家伙得意洋洋的脸上一拳揍过去。");

    int gold = m_player->getGold();
    int hp = m_player->getHp();

    // 选项1: 小修一下
    bool canAdjust = gold >= ADJUST_COST && !m_cardManager->getUpgradableCards().isEmpty();
    QString adjustText = QString("[小修一下] 失去 %1 金币：随机升级 2 张牌。").arg(ADJUST_COST);
    if (gold < ADJUST_COST) adjustText += " (金币不足)";
    else if (!canAdjust) adjustText += " (无牌可升级)";
    addOption(adjustText, [this]() { onAdjustChosen(); }, canAdjust);

    // 选项2: 清洁一下
    bool canClean = gold >= CLEAN_COST;
    QString cleanText = QString("[清洁一下] 失去 %1 金币：移除 1 张牌。").arg(CLEAN_COST);
    if (gold < CLEAN_COST) cleanText += " (金币不足)";
    addOption(cleanText, [this]() { onCleanChosen(); }, canClean);

    // 选项3: 全套服务
    bool canFull = gold >= FULL_COST;
    QString fullText = QString("[全套服务] 失去 %1 金币：移除 1 张牌，随机升级 1 张牌。").arg(FULL_COST);
    if (gold < FULL_COST) fullText += " (金币不足)";
    addOption(fullText, [this]() { onFullServiceChosen(); }, canFull);

    // 选项4: 一拳过去
    bool canPunch = hp > PUNCH_HP_LOSS;
    QString punchText = QString("[一拳过去] 失去 %1 点生命值。").arg(PUNCH_HP_LOSS);
    addOption(punchText, [this]() { onPunchChosen(); }, canPunch);
}

void DesignerView::onAdjustChosen() {
    m_player->modifyGold(-ADJUST_COST);
    m_cardManager->upgradeRandomCards(2);
    showEnding("“好啦，那下次再来哦。”\n...刚刚真该一拳揍上去的。");
}

void DesignerView::onCleanChosen() {
    startCardRemoval([this](Card* card) {
        // 🟢 替換為讓沙盒玩家付錢！
        m_player->modifyGold(-CLEAN_COST);
        m_cardManager->removeCardPermanently(card);
        showEnding("“好啦，那下次再来哦。”\n...刚刚真该一拳揍上去的。");
    });
}

void DesignerView::onFullServiceChosen() {
    startCardRemoval([this](Card* card) {
        // 🟢 替換為讓沙盒玩家付錢！
        m_player->modifyGold(-FULL_COST);
        m_cardManager->removeCardPermanently(card);
        m_cardManager->upgradeRandomCards(1);
        showEnding("“好啦，那下次再来哦。”\n...刚刚真该一拳揍上去的。");
    });
}

void DesignerView::onPunchChosen() {
    m_player->setHp(m_player->getHp() - PUNCH_HP_LOSS);
    setEventImage(":/resources/images/events/Random/Designer/Punched.png"); // 🔴 换图
    showEnding("你一拳揍了过去，打到你的手都疼了。\n“我的脸啊!!这下我要一一”他<b>晕了过去</b>。\n呵呵，现在谁才又恶心又流着血啦?");
}

void DesignerView::startCardRemoval(std::function<void(Card*)> onConfirmed) {
    showDarkOverlay("");
    setOptionsEnabled(false);

    // ========================================================
    // 🔴 注意：除了 this，还要捕获 onConfirmed 回调函数！
    // ========================================================
    QTimer::singleShot(50, this, [this, onConfirmed]() {
        QList<Card*> removable;
        removable.append(m_cardManager->getDrawPile());
        removable.append(m_cardManager->getHand());
        removable.append(m_cardManager->getDiscardPile());

        if (removable.isEmpty()) {
            hideDarkOverlay();
            setOptionsEnabled(true);
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
            if (m_confirmRemoveBtn) m_confirmRemoveBtn->show();
        });
    }

    m_confirmRemoveBtn = new TextButton("确认服务", 200, 55);
    m_confirmRemoveBtn->setPos(960 - 120, 900);
    m_confirmRemoveBtn->setZValue(200);
    m_confirmRemoveBtn->hide();
    m_scene->addItem(m_confirmRemoveBtn);
    connect(m_confirmRemoveBtn, &TextButton::clicked, this, [this, onConfirmed]() {
        Card* selected = nullptr;
        for (auto* item : m_removalCardItems) {
            if (item->isHighlighted()) { selected = item->getLogicCard(); break; }
        }
        if (selected) {
            // 🔴 1. 救命步驟：優雅地超度 UI，切斷繪圖引擎聯繫
            for (auto* item : m_removalCardItems) {
                m_scene->removeItem(item);
                item->deleteLater(); // 絕對不能用 delete!
            }
            m_removalCardItems.clear();

            if (m_confirmRemoveBtn) {
                m_scene->removeItem(m_confirmRemoveBtn);
                m_confirmRemoveBtn->deleteLater();
                m_confirmRemoveBtn = nullptr;
            }
            if (m_cancelRemoveBtn) {
                m_scene->removeItem(m_cancelRemoveBtn);
                m_cancelRemoveBtn->deleteLater();
                m_cancelRemoveBtn = nullptr;
            }

            hideDarkOverlay();
            setOptionsEnabled(true);

            // 🔴 2. UI 徹底剝離後，才執行底層的殺牌和扣錢動作
            onConfirmed(selected);
        }
    });

    m_cancelRemoveBtn = new TextButton("返回", 200, 55);
    m_cancelRemoveBtn->setPos(960 + 120, 900);
    m_cancelRemoveBtn->setZValue(200);
    m_scene->addItem(m_cancelRemoveBtn);
    connect(m_cancelRemoveBtn, &TextButton::clicked, this, &DesignerView::cancelRemoval);
    });
}

void DesignerView::cancelRemoval() {
    // 🔴 再次優雅地超度 UI
    for (auto* item : m_removalCardItems) {
        m_scene->removeItem(item);
        item->deleteLater();
    }
    m_removalCardItems.clear();

    if (m_confirmRemoveBtn) {
        m_scene->removeItem(m_confirmRemoveBtn);
        m_confirmRemoveBtn->deleteLater();
        m_confirmRemoveBtn = nullptr;
    }
    if (m_cancelRemoveBtn) {
        m_scene->removeItem(m_cancelRemoveBtn);
        m_cancelRemoveBtn->deleteLater();
        m_cancelRemoveBtn = nullptr;
    }

    hideDarkOverlay();
    setOptionsEnabled(true);
}

void DesignerView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
