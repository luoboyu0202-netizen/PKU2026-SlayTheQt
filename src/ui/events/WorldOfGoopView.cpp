#include "WorldOfGoopView.h"
#include "../../entities/Player.h"
#include <QRandomGenerator>

WorldOfGoopView::WorldOfGoopView(Player* player, CardManager* cardManager, 
                                 RelicManager* relicManager, QWidget* parent)
    : GenericChoiceEventView(player, cardManager, relicManager, parent)
{
    setupContent();
}

void WorldOfGoopView::setupContent() {
    GenericChoiceEventView::setupContent();

    setTitle("黏液世界");
    setEventImage(":/resources/images/events/Random/Goop/GoopPuddle.jpg");
    setDescription("你掉进了一个水坑里。\n可是坑里全是史莱姆黏液！\n你感觉到这黏液似乎会灼伤你，便拼命想要从坑中脱身。\n你的耳朵、鼻子和全身都被黏液给浸透了。\n爬出来后，你发现自己的金币似乎变少了。你回头一看，发现水坑里不但有你掉落的钱，还有不少其他不幸的冒险者们落下的金币。");

    addOption("[收集金币] 获得 75 金币。失去 11 生命。", [this]() { onGatherGold(); });
    
    // 动态计算失去的金币
    int loseGold = QRandomGenerator::global()->bounded(20, 51); // 20~50
    QString leaveText = QString("[放手吧] 失去 %1 金币。").arg(loseGold);
    addOption(leaveText, [this, loseGold]() { onLeaveItBe(loseGold); });
}

void WorldOfGoopView::onGatherGold() {
    m_player->setGold(m_player->getGold() + 75);
    m_player->takeDamage(11);
    showEnding("在长时间与黏液接触而导致你的皮肤被烧走之前，你成功地捞出了不少金币。");
}

void WorldOfGoopView::onLeaveItBe(int loseGold) {
    m_player->setGold(qMax(0, m_player->getGold() - loseGold));
    showEnding("你决定这样做不值得。");
}

void WorldOfGoopView::showEnding(const QString& resultText) {
    clearOptions();
    setDescription(resultText);
    addOption("[离开]", [this]() {
        emit eventFinished();
    });
}
