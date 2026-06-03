#include <QApplication>
#include "ui/GameWindow.h"
#include "logic/GlobalSaveData.h"
#include "api/EventLauncher.h"
#include "ui/events/EventBaseView.h"
#include "ui/battleview.h"
#include "ui/TopBar.h"
#include "ui/RelicTray.h"
#include "logic/CardFactory.h"
#include "logic/RelicFactory.h"
#include <QDebug>

// ============================================================
// 测试开关：设为 1 启用对应事件独立测试，设为 0 则走大地图
// ============================================================
#define TEST_EVENT 1          // 0=地图模式, 1=独立测试事件
#define TEST_WHICH 7          // 1=Campfire, 2=Chest, 3=Merchant, 4=QuestionMark(GoldenWing), 5=QuestionMark(BigFish), 6=WorldOfGoop, 7=Ssssserpent

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    GlobalSaveData* save = GlobalSaveData::getInstance();
    save->initNewGame();
    save->gold = 999;

#if TEST_EVENT
    // ========================================================
    // 🧪 队友的沙盒独立测试模式！极简启动！
    // ========================================================
    EventContext ctx;
    ctx.currentHp = save->currentHp;
    ctx.maxHp = save->maxHp;
    ctx.gold = save->gold;
    ctx.maxEnergy = save->maxEnergy;

#if TEST_WHICH == 1
    ctx.eventType = EventType::Campfire;
#elif TEST_WHICH == 2
    ctx.eventType = EventType::Chest;
#elif TEST_WHICH == 3
    ctx.eventType = EventType::Merchant;
#elif TEST_WHICH == 4 || TEST_WHICH == 5 || TEST_WHICH == 6 || TEST_WHICH == 7
    ctx.eventType = EventType::QuestionMark;
#if TEST_WHICH == 4
    ctx.eventSubtype = "GoldenWing";
#elif TEST_WHICH == 5
    ctx.eventSubtype = "BigFish";
#elif TEST_WHICH == 6
    ctx.eventSubtype = "WorldOfGoop";
#elif TEST_WHICH == 7
    ctx.eventSubtype = "Ssssserpent";
#endif
#endif

    for (const QString& id : save->deckIds) {
        Card* c = CardFactory::createCard(id);
        if (c) ctx.currentDeck.append(c);
    }
    for (const QString& id : save->relicIds) {
        Relic* r = RelicFactory::createRelic(id);
        if (r) ctx.relics.append(r);
    }

    EventLauncher* launcher = new EventLauncher();
    
    auto playTestMeteor = [](QWidget* parent, const QString& imgPath) {
        QLabel* meteor = new QLabel(parent);
        meteor->setPixmap(QPixmap(imgPath).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        meteor->setGeometry(800, 450, 64, 64);
        meteor->show();
        meteor->raise();
        
        QPropertyAnimation* anim = new QPropertyAnimation(meteor, "pos");
        anim->setDuration(500);
        anim->setStartValue(QPoint(800, 450));
        anim->setEndValue(QPoint(1400, 50));
        anim->setEasingCurve(QEasingCurve::InQuad);
        QObject::connect(anim, &QPropertyAnimation::finished, meteor, &QLabel::deleteLater);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    QObject::connect(launcher, &EventLauncher::showEventViewRequest, [launcher, playTestMeteor](EventBaseView* view) {
        // 在独立测试模式下，手动把 TopBar 和 遗物栏 塞进场景
        // 🚨 修正：因为 EventView 的场景是 1920x1080 并缩放到 1600x900
        // 而 TopBar 是按 1600 宽度设计的。为了让 1600 的 TopBar 在 1920 的场景中看起来正常，
        // 我们需要抵消掉 view 的 fitInView 缩放，或者将 TopBar 放大 1.2 倍 (1920/1600)
        TopBar* topBar = new TopBar();
        topBar->bindPlayer(launcher->getPlayer());
        topBar->setScale(1920.0 / 1600.0); // 🔴 关键：拉伸比例以适配 1920 场景
        topBar->setZValue(999);
        view->scene()->addItem(topBar);

        RelicTray* tray = new RelicTray();
        tray->setPos(10 * (1920.0 / 1600.0), 55 * (1920.0 / 1600.0));
        tray->setScale(1920.0 / 1600.0);
        tray->setZValue(999);
        view->scene()->addItem(tray);
        
        // 监听卡牌加入信号（测试模式特供版动画）
        if (launcher->getCardManager()) {
            QObject::connect(launcher->getCardManager(), &CardManager::cardInsertedToDiscard, [view, topBar, playTestMeteor](Card* c) {
                playTestMeteor(view, c->getImagePath());
                topBar->refreshDeckCount();
            });
        }

        view->show();
    });
    QObject::connect(launcher, &EventLauncher::showBattleViewRequest, [launcher](BattleView* view) {
        // 战斗模式同理
        TopBar* topBar = new TopBar();
        if (view->getEngine() && view->getEngine()->getPlayer()) {
            topBar->bindPlayer(view->getEngine()->getPlayer());
        }
        topBar->setScale(1920.0 / 1600.0);
        topBar->setZValue(999);
        view->scene()->addItem(topBar);

        RelicTray* tray = new RelicTray();
        tray->setPos(10 * (1920.0 / 1600.0), 55 * (1920.0 / 1600.0));
        tray->setScale(1920.0 / 1600.0);
        tray->setZValue(999);
        view->scene()->addItem(tray);

        view->show();
    });
    QObject::connect(launcher, &EventLauncher::eventConcluded, [&a](EventResult result) {
        qDebug() << "Event done. HP:" << result.remainingHp
                 << "Gold:" << result.currentGold
                 << "Cards:" << result.resultDeck.size();
        a.quit();
    });
    launcher->launch(ctx);

    return a.exec();
#else
    // ========================================================
    // 🎮 我们的 3A 级大地图正式游戏模式！
    // ========================================================
    GameWindow window;
    window.show();

    return a.exec();
#endif
}