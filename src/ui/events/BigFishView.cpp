#include "BigFishView.h"
#include "../../entities/Player.h"
#include "../../entities/relics/RelicManager.h"
#include "../../logic/RelicFactory.h"
#include "../../logic/CardFactory.h"
#include <QDebug>
#include "../../logic/GlobalSaveData.h"

BigFishView::BigFishView(Player* player, CardManager* cardManager, 
                         RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void BigFishView::setupContent() {
    GenericChoiceEventView::setupContent();

    setTitle("大鱼");
    setEventImage(":/resources/images/events/Random/Big_fish/Fishing.jpg");
    setDescription("当你走过一条长廊时，你看见空中漂浮着一根香蕉，一个甜甜圈，和一个盒子。 不……仔细一看，它们都是被用绳子系着，从天花板上的几个洞里悬挂下来的。你在接近这几样东西时，上方似乎传来一阵咯咯的笑声。\n你会怎么做？");

    addOption("[香蕉] 回复最大生命值的1/3。", [this]() { onBananaChosen(); });
    addOption("[甜甜圈] 最大生命值 +5。", [this]() { onDonutChosen(); });
    addOption("[盒子] 获得一件遗物。被诅咒——悔恨。", [this]() { onBoxChosen(); });
}

void BigFishView::onBananaChosen() {
    int healAmount = m_player->getMaxHp() / 3;
    m_player->setHp(qMin(m_player->getMaxHp(), m_player->getHp() + healAmount));
    showEnding("你吃下了<b>香蕉</b>，它很有营养，似乎还有些魔法，回复了你的生命。");
}

void BigFishView::onDonutChosen() {
    m_player->setMaxHp(m_player->getMaxHp() + 5);
    m_player->setHp(m_player->getHp() + 5);
    showEnding("你吃下了<b>甜甜圈</b>，真是太好吃了！你的最大生命值增加了。");
}

void BigFishView::onBoxChosen() {
    // ========================================================
    // 🔴 3A 级查重摇奖：先查户口，再造肉身！
    // ========================================================
    GlobalSaveData* save = GlobalSaveData::getInstance();

    // 1. 喂给它玩家已有的遗物名单 (save->relicIds)，摇出一个新 ID
    QString relicId = RelicFactory::generateRandomRelic(save->relicIds);

    if (!relicId.isEmpty()) {
        // 2. 拿着新 ID，去呼叫真正的实体制造机！
        Relic* r = RelicFactory::createRelic(relicId, this);
        if (r) {
            save->relicIds.append(relicId);
            m_relicManager->addRelic(r); // 放进沙盒，等 GameWindow 监听
        }
    }

    // 临时逻辑：默认先给一张“打击”作为诅咒占位
    Card* placeholderCurse = CardFactory::createCard("Strike_R", this);
    if (placeholderCurse) {
        m_cardManager->addCardToDiscardPile(placeholderCurse);
    }

    showEnding("你抓住了盒子，在里面找到了一个<b>遗物</b>！\n可是，你真的很想吃那个甜甜圈……\n你的心中充满了<b>悲伤</b>，尤其是一份<b>悔恨</b>。");
}

void BigFishView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
