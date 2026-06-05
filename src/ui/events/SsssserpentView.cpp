#include "SsssserpentView.h"
#include "../../entities/Player.h"
#include "../../logic/CardFactory.h"

SsssserpentView::SsssserpentView(Player* player, CardManager* cardManager, 
                                 RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void SsssserpentView::setupContent() {
    GenericChoiceEventView::setupContent();

    setTitle("蛇~");
    setEventImage(":/resources/images/events/Random/Liars/LiarsGame.jpg");
    setDescription("你走进一间房间，看见地上有一个大洞。当你靠近洞时，一条巨大的蛇形生物从里面钻了出来。\n“嚯嚯嚯！你好，你好啊！这是谁呀？哎呀呀，你好冒险者，我就问一个简单的问题。\n最幸福的人生当然就是什么东西都能买得起的土豪生活了！\n你同意吗？”");

    addOption("[同意] 得到 175 金币。被诅咒——疑虑。", [this]() { onAgree(); });
    addOption("[反对] 没什么发生。", [this]() { onDisagree(); });
}

void SsssserpentView::onAgree() {
    m_player->setGold(m_player->getGold() + 175);
    
    // 暂时用打击牌替代疑虑诅咒
    Card* doubtCurse = CardFactory::createCard("Strike_R", this);
    if (doubtCurse) {
        m_cardManager->addCardToDiscardPile(doubtCurse);
    }

    showEnding("“对～！\n这会很值～得的。\n嘶……嘶～嘶……”\n蛇抬起头，往上喷出了一堆金币！\n这令人震惊又有点可怕。\n你把金币收好，谢过蛇后，重新上路。");
}

void SsssserpentView::onDisagree() {
    showEnding("蛇非常失望地看着你。");
}

void SsssserpentView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
