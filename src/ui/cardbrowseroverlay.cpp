#include "CardBrowserOverlay.h"
#include <QFont>
#include <QGraphicsScene>
#include <algorithm> // 🔴【新增】：引入标准库排序算法 std::sort 喵！
#include <QPainter>
#include <QDebug>

CardBrowserOverlay::CardBrowserOverlay(const QList<Card*>& cards, const QString& title, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_title(title) {

    setAcceptHoverEvents(true); // 开启悬停
    setZValue(500);             // 霸道层级遮罩
    // 1. 定义一个用于碰撞检测和居中文字的大概矩形 (x=1600, y=950, w=180, h=50)
    // 需要根据你的 PileItem 坐标自行微调，确保不挡住它喵喵喵！
    m_closeBtnRect = QRectF(1600, 950, 180, 50);

    // 2. 锻造六边形点阵！(就像我们做确认按钮那样，尖尖朝向左右喵)
    qreal x = m_closeBtnRect.x();
    qreal y = m_closeBtnRect.y();
    qreal w = m_closeBtnRect.width();
    qreal h = m_closeBtnRect.height();
    qreal tip = 20; // 六边形两头尖尖的长度

    m_closeBtnPolygon << QPointF(x + tip, y)          // 左上
                      << QPointF(x + w - tip, y)      // 右上
                      << QPointF(x + w, y + h / 2.0)  // 最右侧尖尖
                      << QPointF(x + w - tip, y + h)  // 右下
                      << QPointF(x + tip, y + h)      // 左下
                      << QPointF(x, y + h / 2.0);     // 最左侧尖尖

    // ========================================================
    // 🌌【全新架构】：高层级 UI 视觉层 (Z-Value = 100)
    // ========================================================
    // 1. 顶部渐隐遮罩条（当卡牌滚上去时，优雅地将它们隐藏）
    m_topBanner = new QGraphicsRectItem(0, 0, 1920, 120, this);
    m_topBanner->setBrush(QColor(0, 0, 0, 240)); // 极深的纯黑，用来遮挡卡牌
    m_topBanner->setPen(Qt::NoPen);
    m_topBanner->setZValue(100); // 🔴 绝对高空压制！永远在卡牌 (Z=0) 之上！

    // 2. 独立的大标题文字
    m_titleText = new QGraphicsSimpleTextItem(m_title, m_topBanner); // 挂在遮罩条上
    m_titleText->setFont(QFont("Microsoft YaHei", 36, QFont::Bold));
    m_titleText->setBrush(Qt::white);
    qreal textWidth = m_titleText->boundingRect().width();
    m_titleText->setPos((1920 - textWidth) / 2.0, 30); // 完美居中

    // 3. 独立的关闭按钮图形
    m_closeBtnVisual = new QGraphicsPolygonItem(m_closeBtnPolygon, this);
    m_closeBtnVisual->setZValue(100); // 🔴 和顶部遮罩同等高度，压住卡牌！
    m_closeBtnVisual->setBrush(QColor(80, 80, 80));
    m_closeBtnVisual->setPen(QPen(Qt::yellow, 3));

    // 🟢【Qt 绝密黑科技】：关闭这些图元的事件拦截，让点击直接穿透到父类！
    // 这样你就一行都不用改底下的 mousePressEvent 啦！
    // 🟢 换成这句！让它拒绝接受任何鼠标按键，直接穿透给底层的遮罩！
    m_closeBtnVisual->setAcceptedMouseButtons(Qt::NoButton);

    // 4. 关闭按钮上的文字
    m_closeBtnText = new QGraphicsSimpleTextItem(QStringLiteral("关 闭"), m_closeBtnVisual);
    m_closeBtnText->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    m_closeBtnText->setBrush(Qt::white);

    // 文字居中算法
    QRectF textRect = m_closeBtnText->boundingRect();
    QPointF center = m_closeBtnRect.center();
    m_closeBtnText->setPos(center.x() - textRect.width() / 2.0, center.y() - textRect.height() / 2.0);

    // ========================================================
    // 🔴【核心防作弊系统】：克隆一份副本，并进行“费用-字母”双重排序！
    // ========================================================
    QList<Card*> sortedCards = cards; // 浅拷贝一份用来展示的局部列表

    std::sort(sortedCards.begin(), sortedCards.end(), [](Card* a, Card* b) {
        // 1. 第一优先度：比较卡牌的费用 (Cost)
        if (a->getCost() != b->getCost()) {
            return a->getCost() < b->getCost(); // 费用低的排在前面（从小到大）
        }
        // 2. 第二优先度：费用相同时，按卡牌名字的字母/字符串顺序排定座次
        return a->getName() < b->getName();     // 字母序从小到大
    });

    // ========================================================
    // 🎨 网格排版算法（现在使用的是已经排好序的 sortedCards 喵！）
    // ========================================================
    int columns = 7;      // 一行 5 张牌
    qreal startX = 300;   // 起始 X 坐标
    qreal startY = 400;   // 起始 Y 坐标
    qreal spacingX = 240; // 横向间距
    qreal spacingY = 280; // 纵向间距

    for (int i = 0; i < sortedCards.size(); ++i) {
        CardItem* cItem = new CardItem(sortedCards[i], this); // 挂载到当前遮罩上
        cItem->setDisplayOnly(true); // 开启橱窗模式，防诈尸打出

        int row = i / columns;
        int col = i % columns;

        // 丝滑摆放
        cItem->setPos(startX + col * spacingX, startY + row * spacingY);
        cItem->setHomeState(cItem->pos(), 0.0);
        m_uiCards.append(cItem);
    }

    // ========================================================
    // 🔴【新增】：计算最大的滚动深度！
    // ========================================================
    int totalRows = (sortedCards.size() + columns - 1) / columns; // 向上取整算出行数
    qreal totalContentHeight = startY + (totalRows * spacingY) + 100; // +100是底部留白

    // 如果内容总高度超过了屏幕高度(1080)，就允许滚动！否则 MaxScroll 就是 0。
    m_maxScrollY = std::max(0.0, totalContentHeight - 1080.0);
    m_currentScrollY = 0.0;

}

CardBrowserOverlay::~CardBrowserOverlay() {
    // Qt 的父子对象系统会自动 delete 掉挂在它身上的所有 CardItem 喵！
}

QRectF CardBrowserOverlay::boundingRect() const {
    return QRectF(0, 0, 1920, 1080); // 覆盖全屏
}

void CardBrowserOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    // 🔴 只需要画这一个全屏的墓地遮罩！标题和按钮由实体图元自己负责画啦！
    painter->setBrush(QColor(0, 0, 0, 210));
    painter->setPen(Qt::NoPen);
    painter->drawRect(boundingRect());
}

// ========================================================
// 🖱️ 交互系统：检测点击和悬停
// ========================================================
void CardBrowserOverlay::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_closeBtnRect.contains(event->pos())) {
            emit closed(); // 点了关闭按钮，发射信号
        }
    }
    event->accept(); // 吞掉所有点击，防止穿透到下层的战斗场景喵！
}

// ========================================================
// 🖱️ 滚轮事件：捕获鼠标滚动，改变偏移量！
// ========================================================
void CardBrowserOverlay::wheelEvent(QGraphicsSceneWheelEvent* event) {
    if (m_maxScrollY <= 0) {
        return; // 牌很少，一屏就能装下，直接无视滚轮喵！
    }

    qreal scrollStep = 50.0; // 🔴 滚动灵敏度，数字越大滚得越快

    // event->delta() 为正代表向上滚（内容应该往下走，减小 Y），为负代表向下滚
    if (event->delta() > 0) {
        m_currentScrollY -= scrollStep;
    } else {
        m_currentScrollY += scrollStep;
    }

    // 🔴 核心：用 std::clamp 锁死范围，绝不能滚出边界！(需要 #include <algorithm>)
    m_currentScrollY = std::clamp(m_currentScrollY, 0.0, m_maxScrollY);

    // 重新排列所有卡牌！
    updateCardPositions();

    event->accept(); // 吞掉事件，防止背后的战场跟着乱滚
}

// ========================================================
// 🔄 动态刷新：根据当前的滚动偏移量，重新移动所有卡牌！
// ========================================================
void CardBrowserOverlay::updateCardPositions() {
    int columns = 7;
    qreal startX = 200;
    qreal startY = 250;
    qreal spacingX = 220;
    qreal spacingY = 300;

    for (int i = 0; i < m_uiCards.size(); ++i) {
        int row = i / columns;
        int col = i % columns;

        qreal targetX = startX + col * spacingX;
        // 🔴 灵魂减法：用原本的 Y 坐标减去滚动量，实现卡牌整体上移！
        qreal targetY = startY + row * spacingY - m_currentScrollY;

        // 瞬间移动卡牌
        m_uiCards[i]->setPos(targetX, targetY);
        // 🟢 极其重要：必须同步更新卡牌的 HomeState，否则鼠标悬停放大时，卡牌会乱飞！
        m_uiCards[i]->setHomeState(QPointF(targetX, targetY), 0.0);
    }
}

void CardBrowserOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    bool hovered = m_closeBtnRect.contains(event->pos());
    if (hovered != m_isCloseBtnHovered) {
        m_isCloseBtnHovered = hovered;

        // 🔴 直接命令图元变色，不需要调用极其耗费性能的 update() 啦！
        QColor btnColor = hovered ? QColor(220, 50, 50) : QColor(80, 80, 80);
        m_closeBtnVisual->setBrush(btnColor);
    }
}