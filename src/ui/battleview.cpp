#include "BattleView.h"
#include <QDebug>
#include "PlayerItem.h"
#include "ShuffleEffectItem.h"
#include <QPropertyAnimation>
#include <QTimer>
#include <QMouseEvent>
#include "CardBrowserOverlay.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>

BattleView::BattleView(QWidget* parent)
    : QGraphicsView(parent), m_layoutManager(nullptr){

    // 1. 初始化场景尺寸
    m_scene = new BattleScene(this);
    m_scene->setSceneRect(0, 0, 1920, 1080);
    setScene(m_scene);

    // ====================================================
    // 🔴【核心修正组合拳】：彻底解决窗口对齐与裁切 Bug！
    // ====================================================
    // 组合拳一：强行将对齐方式改为左上角对齐！
    // 这样场景的 (0,0) 就会死死黏在窗口的左上角，绝对坐标再也不会错位！
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 组合拳二：给外层主窗口一个合理的初始尺寸（比如 1600x900 或直接 1920x1080）
    // 防止窗口初始化时太小导致内容看不全喵
    this->setFixedSize(1600, 900); // 填入你觉得最完美的尺寸
    this->setStyleSheet("background-color: black;"); // 🟢 强行抹黑底色，防止背景穿透！

    // 如果想让窗口可以根据组员的电脑屏幕大小全自动缩放，还可以加上这句：
    //this->setViewState(Qt::WindowMaximized); // 默认最大化显示

    // ====================================================

    // 2. 禁用滚动条，保持游戏画面的纯净度
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    // 3. 骨架搭起：铺好一成不变的 UI
    initStageInfrastructure();

}

void BattleView::playShuffleAnimation() {
    // 1. 获取两座牌堆在场景中的绝对坐标
    QPointF startPos = m_discardPileUI->scenePos();
    QPointF endPos = m_drawPileUI->scenePos();

    // 2. 连续发射 15 个金色小方块喵！
    for (int i = 0; i < 15; ++i) {

        // 🔴 错开时间发射，每个方块晚出生 40 毫秒，形成连绵不绝的弹幕效果！
        QTimer::singleShot(i * 40, this, [this, startPos, endPos]() {

            // 创造金色方块并扔进舞台
            ShuffleEffectItem* goldSquare = new ShuffleEffectItem(startPos, endPos);
            m_scene->addItem(goldSquare);

            // 给它挂上动力引擎
            QPropertyAnimation* anim = new QPropertyAnimation(goldSquare, "progress");
            anim->setDuration(500); // 飞过去需要 0.5 秒
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::InOutQuad); // 起步和落地稍微平滑一点

            // 🔴 极其重要：飞完之后，连同方块和动画引擎一起原地销毁，绝不漏内存！
            connect(anim, &QPropertyAnimation::finished, goldSquare, &QObject::deleteLater);
            connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);

            anim->start();
        });
    }
}

void BattleView::initStageInfrastructure() {
    // m_topBar = new TopBar();
    // m_scene->addItem(m_topBar);

    // m_relicTray = new RelicTray();
    // m_relicTray->setPos(10, 70);
    // m_relicTray->setZValue(5000);
    // m_scene->addItem(m_relicTray);

    m_drawPileUI = new PileItem(QStringLiteral("抽牌堆"));
    m_drawPileUI->setPos(80, 1000);
    m_scene->addItem(m_drawPileUI);

    m_discardPileUI = new PileItem(QStringLiteral("弃牌堆"));
    m_discardPileUI->setPos(1850, 1000);
    m_scene->addItem(m_discardPileUI);

    // ========================================================
    // 🔴 组装消耗牌堆！
    // ========================================================
    m_exhaustPileUI = new PileItem("消耗堆"); // 假设你的 PileItem 构造函数支持传名字

    // 摆在右下角，弃牌堆的上方。你需要根据实际屏幕微调这个坐标喵！
    // 假设弃牌堆是 (1700, 950)，我们把它放在 (1750, 750)
    m_exhaustPileUI->setPos(1850, 800);
    m_exhaustPileUI->setZValue(50);

    // 🔴 核心特征：战斗刚开始时没有任何牌被烧，所以它是隐藏的！
    m_exhaustPileUI->hide();

    m_scene->addItem(m_exhaustPileUI);

    m_endTurnBtn = new EndTurnButton();
    m_endTurnBtn->setPos(1600, 850); // 尊享尖塔黄金布局点
    m_scene->addItem(m_endTurnBtn);

    m_energyBall = new EnergyWidget();
    m_energyBall->setPos(400, 840);
    m_scene->addItem(m_energyBall);

    // 1. 创建覆盖全屏的暗色结界 (假设屏幕是 1920x1080，原点在左上角)
    m_darkOverlay = new QGraphicsRectItem(0, 0, 1920, 1080);
    m_darkOverlay->setBrush(QColor(0, 0, 0, 180)); // 180 意味着 70% 的黑色遮罩
    // 🔴 核心机制：ZValue！
    // 背景图是 -1000，怪物是 0。蒙版设为 100，把它盖在所有东西上面！
    m_darkOverlay->setZValue(100);
    m_darkOverlay->hide(); // 平时藏起来
    m_scene->addItem(m_darkOverlay);

    // 2. 创建悬浮文字
    m_promptTextItem = new QGraphicsTextItem();
    m_promptTextItem->setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
    m_promptTextItem->setDefaultTextColor(Qt::white);
    // 把它放在蒙版上面 (Z=101)
    m_promptTextItem->setZValue(101);
    m_promptTextItem->hide();
    m_scene->addItem(m_promptTextItem);

    // 1. 在 initStageInfrastructure() 里组装新按钮，摆在你红圈的位置！
    m_confirmBtn = new ConfirmButton();
    // 坐标大概在 1500, 750 的位置（弃牌堆的上方），你可以自己微调喵！
    m_confirmBtn->setPos(1500, 750);
    m_confirmBtn->setZValue(102);
    m_confirmBtn->hide();
    m_scene->addItem(m_confirmBtn);

}

void BattleView::bindEngine(BattleEngine* engine) {
    if (!engine) return;

    // 正式接管新大脑！
    m_engine = engine;

    QString bgPath = m_engine->getBackgroundPath();

    // 🔴 测谎仪 1 号：把大脑传过来的路径直接打印出来！
    qDebug() << "[Diagnostics - UI] 大脑传来的背景路径是:" << bgPath;

    if (!bgPath.isEmpty()) {
        QPixmap bgPixmap(bgPath);
        if (!bgPixmap.isNull()) {
            // 1. 让图片平滑、无损地缩放到 1920x1080
            bgPixmap = bgPixmap.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(bgPixmap);

            // 2. 扔到最底层，确保不挡住大史莱姆
            bgItem->setZValue(-1000);

            // 3. 🔴 确认对齐位置：
            bgItem->setPos(0, 0);

            m_scene->addItem(bgItem);
            // 🔴 测谎仪 2 号：如果成功了，一定要欢呼一声！
            qDebug() << "[Diagnostics - UI] 成功绘制背景图片！";
        } else {
            qDebug() << "[UI Architecture] 警告：背景图加载失败，路径：" << bgPath;
        }
    } else {
        // 🔴 测谎仪 3 号：如果路径为空，当场抓获！
        qDebug() << "[UI Architecture] 致命警告：路径是空的！一定是 Launcher 没传进来或者 Engine 没存住！";
    }

    // ========================================================
    // 🔴【新核心】：为主角实例化 UI 肉体，并扔进场景里！
    // ========================================================
    Player* playerLogic = m_engine->getPlayer();

    //🔴 终极修复：使用 Qt 推荐的生命周期管理方式
    if (m_playerItem) {
        // 1. 先检查它是否还在场景里
        if (m_playerItem->scene() != nullptr) {
            m_scene->removeItem(m_playerItem);
        }

        // 2. 绝对禁止手动 delete！因为它是由 scene 管理的，手动 delete 会导致野指针！
        // 真正的删除方式是使用 deleteLater() (因为它继承了 QObject)
        m_playerItem->deleteLater();

        // 3. 🔴 这一步是灵魂：指针清空，防止野指针再次访问！
        m_playerItem = nullptr;
    }

    m_playerItem = new PlayerItem(playerLogic);
    m_playerItem->setPos(400, 620); // 主角站在屏幕左边
    m_scene->addItem(m_playerItem);

    // 获取卡牌管家
    CardManager* cardManager = engine->getCardManager();

    // ========================================================
    // 🔴【多怪物舞台大重构】：动态生成并排版怪物集团！
    // ========================================================
    // 1. 先清理上一场的尸体
    for (EnemyItem* oldItem : m_enemyItems) {
        if (oldItem->scene()) { m_scene->removeItem(oldItem); }
        oldItem->deleteLater();
    }
    m_enemyItems.clear();

    const QList<Enemy*>& enemies = m_engine->getEnemies();

    // 🟢【极其重要】：删除了局部变量 startX 和 spacing，直接用头文件的常量！

    // 4. 循环把它们画在舞台上！
    for (int i = 0; i < enemies.size(); ++i) {
        Enemy* enemyLogic = enemies[i];

        int spriteYOffset = 0;
        QString id = enemyLogic->getId();

        if (id == "Slime_Small") spriteYOffset = 60;
        else if (id == "Slime_01") spriteYOffset = 10;
        else if (id == "Flying_Bat") spriteYOffset = -40;

        EnemyItem* enemyItem = new EnemyItem(enemyLogic, m_engine, spriteYOffset);

        // ========================================================
        // 🔴【绝对槽位排版】：全场统一使用大写常量！
        // ========================================================
        int slot = enemyLogic->getSlotIndex();

        // 兜底防穿模：如果没排座位，默认坐 0 号位
        if (slot < 0 || slot > 3) slot = 0;

        // 🟢 核心修正：直接使用 MONSTER_START_X 和 MONSTER_SPACING！
        enemyItem->setPos(MONSTER_START_X + (slot * MONSTER_SPACING), UNIFIED_BASE_Y);

        m_scene->addItem(enemyItem);
        m_enemyItems.append(enemyItem);

        // 🟢 完整的死亡火化清理机制
        connect(enemyLogic, &Enemy::died, this, [this, enemyItem]() {
            m_enemyItems.removeOne(enemyItem);
            if (enemyItem->scene()) {
                m_scene->removeItem(enemyItem);
            }
            enemyItem->deleteLater();
        });
    }

    // ========================================================
    // 后续绑定逻辑保持原样
    // ========================================================
    // 3. 将玩家数据绑定给那些嗷嗷待哺的固定 UI 组件
    // m_topBar->bindPlayer(playerLogic);
    m_energyBall->bindPlayer(playerLogic);

    // 4. 实例化手牌阵列管家（将场景和逻辑卡牌缝合）
    if (m_layoutManager) { delete m_layoutManager; }
    m_layoutManager = new HandLayoutManager(m_scene, playerLogic, cardManager, this);

    // 加上 Qt::UniqueConnection 后，哪怕这段代码被执行 100 次，信号也只会绑定 1 次！
    connect(m_layoutManager, &HandLayoutManager::cardPlayedRequest,
            m_engine, &BattleEngine::playCard,
            Qt::UniqueConnection); // 🔴 终极防抖结界！

    // 5. 【大网信号闭环】：在这里完成一尘不染的跨模块咬合！
    // 🔴 修正：把参数名统一写为 drawCount, discardCount, exhaustCount，让它们更清晰喵！
    connect(cardManager, &CardManager::pileCountsChanged, this, [this](int drawCount, int discardCount, int exhaustCount) {

        // 1. 更新原本的牌堆 (🔴 修正：使用你定义好的 updateCount)
        m_drawPileUI->updateCount(drawCount);
        m_discardPileUI->updateCount(discardCount);

        // 2. 更新消耗堆的数字 (🔴 修正：同上)
        m_exhaustPileUI->updateCount(exhaustCount);

        // 3. 动态显示逻辑：只有当里面有牌时，它才配出现在场上！
        if (exhaustCount > 0) {
            if (!m_exhaustPileUI->isVisible()) {
                m_exhaustPileUI->show();
                // 如果你想加点果汁感，可以在这里给 m_exhaustPileUI 挂一个从小变大的弹出动画喵！
            }
        } else {
            m_exhaustPileUI->hide();
        }
    });

    connect(m_endTurnBtn, &EndTurnButton::clicked, this, [engine](){
        engine->endPlayerTurn();
    });

    // connect(m_layoutManager, &HandLayoutManager::cardPlayedRequest, this, [engine](Card* card, Enemy* target){
    //     engine->playCard(card, target);
    // });

    connect(cardManager, &CardManager::deckShuffled, this, &BattleView::playShuffleAnimation);

    qDebug() << "[UI Architecture] BattleEngine successfully wired up to BattleView with multiple enemies!";

    // ========================================================
    // 🟢 结界展开：显示黑幕、文字，并请出我们的智能确认按钮！
    // ========================================================
    connect(engine, &BattleEngine::selectionModeStarted, this, [this](const QString& text) {
        // 1. 铺黑幕，写提示字
        m_darkOverlay->show();
        m_promptTextItem->setPlainText(text);

        // 计算文字宽度，让它完美居中！(假设屏幕宽 1920)
        qreal textWidth = m_promptTextItem->boundingRect().width();
        m_promptTextItem->setPos((1920 - textWidth) / 2.0, 300);
        m_promptTextItem->show();

        // 2. 召唤新的智能按钮，并重置为不可点状态！
        // 🔴 以前那堆 setBrush 和 setText 全部删掉啦，ConfirmButton 内部自己会搞定！
        m_confirmBtn->setValid(false);
        m_confirmBtn->show();
    });

    // ========================================================
    // 🟢 智能按钮状态同步：大脑说能点，按钮就变绿！
    // ========================================================
    connect(engine, &BattleEngine::selectionValidityChanged, this, [this](bool canConfirm) {
        // 🔴 以前那堆复杂的 if-else 变色逻辑全删掉，只留这一句！
        m_confirmBtn->setValid(canConfirm);
    });

    // ========================================================
    // 🟢 引爆结界：玩家点下按钮，把信号传给大脑！
    // ========================================================
    // 🔴 全新连线：只要 ConfirmButton 内部判断可以点击并发出了 clicked 信号，大脑直接确认收网！
    connect(m_confirmBtn, &ConfirmButton::clicked, engine, &BattleEngine::confirmHandSelection);

    // ========================================================
    // 🟢 结界解除：藏起黑幕、文字和按钮！
    // ========================================================
    connect(engine, &BattleEngine::selectionModeEnded, this, [this]() {
        m_darkOverlay->hide();
        m_promptTextItem->hide();
        m_confirmBtn->hide();
    });

    connect(m_exhaustPileUI, &PileItem::clicked, this, [this, engine]() {
        qDebug() << "[UI] 玩家点开了消耗堆喵！";

        // 1. 获取墓地里的所有卡牌
        const QList<Card*>& exhaustCards = engine->getCardManager()->getExhaustPile();

        if (exhaustCards.isEmpty()) return; // 空的就别弹了

        // 2. 生成至尊卡牌浏览器，盖在舞台上！
        CardBrowserOverlay* browser = new CardBrowserOverlay(exhaustCards, "消耗牌堆");
        m_scene->addItem(browser);

        // 如果你想加点出场动画，可以给 browser 做个 Opacity 渐变喵~

        // 3. 监听它的关闭信号，点击关闭时自动销毁
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    // --------------------------------------------------------
    // 🔴 1. 抽牌堆的点击浏览功能
    // --------------------------------------------------------
    connect(m_drawPileUI, &PileItem::clicked, this, [this, engine]() {
        const QList<Card*>& drawCards = engine->getCardManager()->getDrawPile();
        if (drawCards.isEmpty()) return; // 没牌就不弹

        // 直接复用至尊浏览器，只改个标题喵！
        CardBrowserOverlay* browser = new CardBrowserOverlay(drawCards, "抽牌堆");
        m_scene->addItem(browser);
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    // --------------------------------------------------------
    // 🔴 2. 弃牌堆的点击浏览功能
    // --------------------------------------------------------
    connect(m_discardPileUI, &PileItem::clicked, this, [this, engine]() {
        const QList<Card*>& discardCards = engine->getCardManager()->getDiscardPile();
        if (discardCards.isEmpty()) return;

        CardBrowserOverlay* browser = new CardBrowserOverlay(discardCards, "弃牌堆");
        m_scene->addItem(browser);
        connect(browser, &CardBrowserOverlay::closed, browser, &QGraphicsObject::deleteLater);
    });

    // ========================================================
    // 🔴【终极动态生成】：监听怪物的召唤！
    // ========================================================
    connect(engine, &BattleEngine::enemySummoned, this, [this, engine](Enemy* newEnemy) {
        qDebug() << "[UI] 接收到召唤信号，准备生成肉体：" << newEnemy->getName();

        int spriteYOffset = (newEnemy->getId() == "Slime_Small") ? 60 : 0;
        EnemyItem* newItem = new EnemyItem(newEnemy, engine, spriteYOffset);

        // 🔴【重构】：新召唤的小怪同样严格对号入座，实现完美统一！
        int slot = newEnemy->getSlotIndex();
        newItem->setPos(MONSTER_START_X + (slot * MONSTER_SPACING), UNIFIED_BASE_Y);

        m_scene->addItem(newItem);
        m_enemyItems.append(newItem);

        // 🟢 极其重要：新出生的小怪死掉时，也要彻底火化！
        connect(newEnemy, &Enemy::died, this, [this, newItem]() {
            m_enemyItems.removeOne(newItem);

            if (newItem->scene()) {
                m_scene->removeItem(newItem);
            }
            newItem->deleteLater();
        });

        // Q弹变大动画保持原样喵~
        newItem->setScale(0.1);
        QPropertyAnimation* popAnim = new QPropertyAnimation(newItem, "scale");
        popAnim->setDuration(500);
        popAnim->setEndValue(1.0);
        popAnim->setEasingCurve(QEasingCurve::OutBack);
        popAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // ========================================================
    // 🎨 塞牌动画：怪物朝你扔出黏液的果汁感特效！
    // ========================================================
    connect(cardManager, &CardManager::cardInsertedToDiscard, this, [this](Card* card) {

        // 1. 制造一张视觉替身牌（不挂载任何逻辑状态）
        CardItem* visualCard = new CardItem(card);
        // 🔴 彻底开启断路器，让它成为一个纯视觉的死物
        visualCard->setGhostMode(true);
        m_scene->addItem(visualCard);

        // 2. 从屏幕中央偏上的地方（模拟从怪物那边飞过来）出现
        visualCard->setPos(1920 / 2 - 100, 1080 / 2 - 200);
        visualCard->setZValue(1000); // 浮在最上面

        // 3. 组装复合动画：一边飞向弃牌堆，一边放大再缩小！
        QParallelAnimationGroup* group = new QParallelAnimationGroup(visualCard);

        // 飞行轨道
        QPropertyAnimation* flyAnim = new QPropertyAnimation(visualCard, "pos");
        flyAnim->setDuration(300);
        // 飞向弃牌堆 UI 的坐标
        flyAnim->setEndValue(m_discardPileUI->pos());
        flyAnim->setEasingCurve(QEasingCurve::InQuad); // 越飞越快

        // 缩放轨道 (0.1 -> 1.2 爆出 -> 0.2 缩进牌堆)
        QPropertyAnimation* scaleAnim = new QPropertyAnimation(visualCard, "scale");
        scaleAnim->setDuration(300);
        scaleAnim->setStartValue(0.5);
        scaleAnim->setKeyValueAt(0.3, 1.2);
        scaleAnim->setEndValue(0.01);

        group->addAnimation(flyAnim);
        group->addAnimation(scaleAnim);

        // 🔴 动画播完，替身自动销毁！不留内存隐患！
        connect(group, &QParallelAnimationGroup::finished, visualCard, &QObject::deleteLater);

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // ========================================================
    // 🌌【能力牌：灵魂注入特效】
    // ========================================================
    connect(engine, &BattleEngine::powerActivated, this, [this](Card* card, Fighter* source) {
        // 1. 创建一个临时的视觉替身（因为它已经从手牌列表消失了，我们需要一个新肉体来演戏）
        CardItem* ghostCard = new CardItem(card);
        // 🔴 彻底开启断路器，让它成为一个纯视觉的死物
        ghostCard->setGhostMode(true);
        m_scene->addItem(ghostCard);

        // 2. 起点设在屏幕中央（模拟刚打出的位置）
        ghostCard->setPos(1920 / 2 - 100, 1080 / 2 - 150);
        ghostCard->setZValue(2000); // 绝对高层
        ghostCard->setScale(1.2);   // 出现时稍微变大，有冲击力

        // 3. 组装顺序动画：停留一下 -> 飞向主角并消失
        QSequentialAnimationGroup* group = new QSequentialAnimationGroup(ghostCard);

        // --- 动作 A：短暂的蓄力停留（加个小抖动） ---
        QPropertyAnimation* focus = new QPropertyAnimation(ghostCard, "scale");
        focus->setDuration(300);
        focus->setEndValue(1.3); // 微微胀大
        focus->setEasingCurve(QEasingCurve::OutBack);

        // --- 动作 B：快速冲向主角 ---
        QPropertyAnimation* fly = new QPropertyAnimation(ghostCard, "pos");
        fly->setDuration(500);
        // 目标：主角 PlayerItem 的坐标！
        fly->setEndValue(m_playerItem->pos() + QPointF(0, -100));
        fly->setEasingCurve(QEasingCurve::InBack); // 先往后撤一点再猛冲！

        // --- 动作 C：缩放动画（冲的过程中变小） ---
        QPropertyAnimation* shrink = new QPropertyAnimation(ghostCard, "scale");
        shrink->setDuration(500);
        shrink->setEndValue(0.1);

        group->addPause(200); // 停顿 0.2 秒让玩家看清牌名
        group->addAnimation(focus);

        // 让飞行和缩小同步发生！
        QParallelAnimationGroup* rushGroup = new QParallelAnimationGroup(group);
        rushGroup->addAnimation(fly);
        rushGroup->addAnimation(shrink);
        group->addAnimation(rushGroup);

        // 🔴 动画结束后的连锁反应
        connect(group, &QSequentialAnimationGroup::finished, [this, ghostCard]() {
            // 1. 销毁替身
            ghostCard->deleteLater();

            // 2. 🔴 主角身体闪烁一下（果汁感！）
            QPropertyAnimation* glow = new QPropertyAnimation(m_playerItem, "scale");
            glow->setDuration(200);
            glow->setKeyValueAt(0, 1.0);
            glow->setKeyValueAt(0.5, 1.15); // 身体胀大一下
            glow->setKeyValueAt(1, 1.0);
            glow->start(QAbstractAnimation::DeleteWhenStopped);

            // 这里可以顺便播放一个叮的音效喵！
        });

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // ========================================================
    // 🔴【核心修复 2&3】：全局状态广播局！
    // ========================================================
    connect(engine->getPlayer()->getStatusManager(), &StatusManager::statusChanged, this, [this](StatusType type, int amount) {
        Q_UNUSED(type); Q_UNUSED(amount);

        // 🔴 核心修复：使用真正存在的 m_layoutManager 变量！
        // 并且加上防空指针保护，极其安全喵！
        if (m_layoutManager) {
            // 只要主角状态一变（比如获得力量），强令手牌里的所有卡牌立刻重绘！
            for (CardItem* cItem : m_layoutManager->getHandItems()) {
                if (cItem) {
                    cItem->update();
                }
            }
        }

        qDebug() << "[UI] 主角状态巨变！全场手牌强制刷新！";
    });

    // ========================================================
    // 🌌【终极视觉】：卡牌从抽牌堆飞向屏幕中央的“时停演出”
    // ========================================================
    connect(engine, &BattleEngine::topCardRevealed, this, [this, engine](Card* card, bool exhaustIt) {

        // 1. 制造一张高精度的视觉替身牌
        CardItem* ghostCard = new CardItem(card);
        // 🔴 彻底开启断路器，让它成为一个纯视觉的死物
        ghostCard->setGhostMode(true);
        m_scene->addItem(ghostCard);

        // 2. 起点设在抽牌堆 (左下角)
        ghostCard->setPos(m_drawPileUI->pos());
        ghostCard->setZValue(3000); // 绝对霸道层级，挡住一切！

        // 3. 第一段动画：飞到正中央并放大！
        QParallelAnimationGroup* popGroup = new QParallelAnimationGroup(ghostCard);

        QPropertyAnimation* flyCenter = new QPropertyAnimation(ghostCard, "pos");
        flyCenter->setDuration(400);
        flyCenter->setEndValue(QPointF(1920 / 2.0 - 100, 1080 / 2.0 - 150));
        flyCenter->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation* scaleUp = new QPropertyAnimation(ghostCard, "scale");
        scaleUp->setDuration(400);
        scaleUp->setEndValue(1.5); // 放大 1.5 倍震撼全场！

        popGroup->addAnimation(flyCenter);
        popGroup->addAnimation(scaleUp);

        // 4. 当飞到中央后，开始进入“时停悬停”与“最终斩杀”环节！
        // 4. 当飞到中央后，开始进入“时停悬停”与“最终斩杀”环节！
        connect(popGroup, &QParallelAnimationGroup::finished, [this, engine, ghostCard, card, exhaustIt]() {

            // 延迟 0.8 秒（给玩家看清牌）
            QTimer::singleShot(800, [this, engine, ghostCard, card, exhaustIt]() {

                // 🔴 1. 时停结束，大脑恢复运转，执行真正的卡牌逻辑！
                // 如果是能力牌，这句话内部会触发 emit powerActivated() 召唤 2号替身！
                engine->executeRevealedCard(card, exhaustIt);

                // ========================================================
                // 🛡️ 2. 【核心修复】：视觉分流闸门！
                // ========================================================
                if (card->getType() == CardType::Power) {
                    // 🌟 能力牌的“灵魂注入”动画已经由 powerActivated 接管了！
                    // 这个负责时停的 1号替身 功成身退，原地解散喵！
                    ghostCard->deleteLater();
                }
                else {
                    // ⚔️ 攻击/技能牌：老老实实播放飞向墓地或弃牌堆的动画
                    QPropertyAnimation* flyAway = new QPropertyAnimation(ghostCard, "pos");
                    flyAway->setDuration(300);

                    // 💡 顺手修复一个小细节：卡牌如果自身带消耗属性，也得进墓地！
                    bool willExhaust = exhaustIt || card->isExhaustOnUse();
                    flyAway->setEndValue(willExhaust ? m_exhaustPileUI->pos() : m_discardPileUI->pos());
                    flyAway->setEasingCurve(QEasingCurve::InBack);

                    QPropertyAnimation* scaleDown = new QPropertyAnimation(ghostCard, "scale");
                    scaleDown->setDuration(300);
                    scaleDown->setEndValue(0.1);

                    QParallelAnimationGroup* endGroup = new QParallelAnimationGroup(ghostCard);
                    endGroup->addAnimation(flyAway);
                    endGroup->addAnimation(scaleDown);

                    // 彻底飞进牌堆后，销毁 1号替身！
                    connect(endGroup, &QParallelAnimationGroup::finished, ghostCard, &QObject::deleteLater);
                    endGroup->start(QAbstractAnimation::DeleteWhenStopped);
                }
            });
        });

        popGroup->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void BattleView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);

    // 🟢【黑魔法】：全自动等比例缩放视口，确保 1920x1080 的战局永远完美塞在窗口里！
    if (m_scene) {
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}

