#include "CardDraftOverlay.h"
#include "logic/CardFactory.h"
#include "ui/CardItem.h" // 引入你的卡牌 UI 包装类喵！
#include <QVBoxLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QDebug>

// ========================================================
// 🎭 场景事件过滤器：巧妙捕捉卡牌点击！
// ========================================================
// 这是一个极其轻量级的黑魔法，用来在不修改 CardItem 源码的情况下捕捉点击
class DraftSceneFilter : public QObject {
public:
    DraftSceneFilter(CardDraftOverlay* overlay, QObject* parent = nullptr)
        : QObject(parent), m_overlay(overlay) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::GraphicsSceneMousePress) {
            QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QGraphicsScene* scene = static_cast<QGraphicsScene*>(watched);

            // 获取鼠标点到了什么东西？
            QGraphicsItem* clickedItem = scene->itemAt(mouseEvent->scenePos(), QTransform());

            // 一层层往上找，看看是不是点到了卡牌！(防误触子元素)
            while (clickedItem) {
                CardItem* cardItem = dynamic_cast<CardItem*>(clickedItem);
                if (cardItem) {
                    qDebug() << "[CardDraft] 玩家选中了卡牌！ID:" << cardItem->getLogicCard()->getId();
                    // 🔴 换成 getLogicCard() ！
                    QMetaObject::invokeMethod(m_overlay, "onCardClicked", Q_ARG(QString, cardItem->getLogicCard()->getId()));
                    return true; // 吞掉事件
                }
                clickedItem = clickedItem->parentItem();
            }
        }
        return QObject::eventFilter(watched, event);
    }
private:
    CardDraftOverlay* m_overlay;
};

// ========================================================
// 🔮 玻璃罩本体实现
// ========================================================
CardDraftOverlay::CardDraftOverlay(QWidget* parent) : QWidget(parent) {
    // 1. 铺满全屏，并设置半透明黑色背景，让后面的战利品界面变暗
    this->setFixedSize(1600, 900);
    // 🔴 激活咒语：强制 QWidget 渲染背景色！(这就是你的黑纱失踪的原因)
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("CardDraftOverlay { background-color: rgba(0, 0, 0, 220); }");

    // 2. 顶层排版布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 50, 0, 50);

    // 3. 华丽的标题
    m_titleLabel = new QLabel("选择一张卡牌加入你的牌组", this);
    m_titleLabel->setStyleSheet("color: #ecf0f1; font-size: 42px; font-weight: bold; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    // 4. 🖼️ 核心玻璃罩：透明的 QGraphicsView！
    m_view = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);

    // 去除所有边框、滚动条和背景，让它完全融于夜色！
    m_view->setStyleSheet("background: transparent; border: none;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_scene->setSceneRect(0, 0, 1600, 500); // 舞台区域大小

    // 🔴 挂载场景过滤器，全盘监控鼠标点击！
    m_scene->installEventFilter(new DraftSceneFilter(this, m_scene));

    mainLayout->addWidget(m_view);

    // 5. 🎨 终极抛光：羊皮纸复古风格的“返回”按钮
    m_skipButton = new QPushButton("返 回", this); // 👈 顺便把变量名逻辑在脑内等价于返回键
    m_skipButton->setFixedSize(180, 45);
    m_skipButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(35, 35, 40, 220);"  // 深暗底色
        "   color: #cbb796;"                            // 经典的暗金色字体
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   border: 1px solid #6b5c43;"                // 铜锈色细边框
        "   border-radius: 2px;"                       // 极其硬朗的微圆角
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(55, 55, 60, 255);"
        "   border-color: #cbb796;"                     // 悬停时边框高亮成暗金
        "   color: #ffffff;"
        "}"
        );
    // 注意连接的槽函数名字也可以保持或者改写
    connect(m_skipButton, &QPushButton::clicked, this, &CardDraftOverlay::onSkipClicked);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_skipButton);
    mainLayout->addLayout(btnLayout);
}

// ========================================================
// 🚀 核心逻辑：展示三选一卡牌，并播放果汁感入场动画
// ========================================================
void CardDraftOverlay::showDraft(const QList<QString>& cardIds) {
    m_scene->clear();
    m_draftCards.clear();

    // 🔴 更优雅的居中算法 (假设中心在 800)
    // 🟢 黄金紧凑排版：缩小间距，黄金分割高度
    qreal spacing = 250; // 👈 从 350 缩减到 250，让卡牌完美聚拢！
    qreal yPos = 180;    // 👈 稍微下移一点点，视觉重心更稳

    // 为了实现交错入场动画，我们把所有动画装进组里
    QParallelAnimationGroup* allAnims = new QParallelAnimationGroup(this);

    for (int i = 0; i < cardIds.size(); ++i) {
        // 1. 造卡并套上 UI 外壳
        Card* logicCard = CardFactory::createCard(cardIds[i]);
        m_draftCards.append(logicCard);

        CardItem* cItem = new CardItem(logicCard);
        cItem->setDisplayOnly(true); // 开启橱窗模式，防止它被拖拽出牌！
        m_scene->addItem(cItem);

        // 🟢 以 800 为绝对中心：i=0 在 550, i=1 在 800, i=2 在 1050，极其对称！
        qreal targetX = 800 + (i - 1) * spacing;

        cItem->setPos(targetX, yPos + 600); // 从下方飞入

        // 3. 💥 注入灵魂：延迟交错飞入动画 (Delay Staggering)
        // 第一张直接飞，第二张晚 100ms，第三张晚 200ms
        QSequentialAnimationGroup* delayGroup = new QSequentialAnimationGroup();
        delayGroup->addPause(i * 100);

        QPropertyAnimation* flyAnim = new QPropertyAnimation(cItem, "pos");
        flyAnim->setEndValue(QPointF(targetX, yPos));
        flyAnim->setDuration(600);
        flyAnim->setEasingCurve(QEasingCurve::OutBack); // 强烈的重力回弹感！

        delayGroup->addAnimation(flyAnim);
        allAnims->addAnimation(delayGroup);

        // 必须立刻更新它的 homeState，否则悬停放大时会乱飞！
        // 用一个定时器在动画结束后更新 homeState
        QTimer::singleShot(i * 100 + 650, this, [cItem, targetX, yPos]() {
            if(cItem) cItem->setHomeState(QPointF(targetX, yPos), 0.0);
        });
    }

    this->raise();
    this->show();
    allAnims->start(QAbstractAnimation::DeleteWhenStopped); // 播放全部动画！
}

// 隐藏于 QMetaObject::invokeMethod 之后的真实槽函数
void CardDraftOverlay::onCardClicked(QString cardId) {
    qDebug() << "[CardDraft] 发射选卡信号！退下吧！";
    emit cardSelected(cardId);
}

void CardDraftOverlay::onSkipClicked() {
    qDebug() << "[CardDraft] 玩家退出了三选一，返回主奖励黑板";
    emit returnRequested(); // 🟢 仅仅通知返回，不销毁任何物资
}
