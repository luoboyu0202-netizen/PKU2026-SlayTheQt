#include "CardBrowserOverlay.h"
#include <QFont>
#include <QGraphicsScene>
#include <algorithm> // std::sort 和 std::clamp 的家
#include <QPainter>
#include <QDebug>

// ========================================================
// 🏛️ 构造函数：注入自适应变量，打造弹性结界！
// ========================================================
CardBrowserOverlay::CardBrowserOverlay(const QList<Card*>& cards, const QString& title, qreal screenW, qreal screenH, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_title(title), m_screenW(screenW), m_screenH(screenH) {

    setAcceptHoverEvents(true); // 开启悬停
    setZValue(500);             // 霸道层级遮罩

    // 🔴 1. 关闭按钮靠右下角 (根据传入的屏幕尺寸动态吸附，完全告别黑洞！)
    m_closeBtnRect = QRectF(m_screenW - 250, m_screenH - 100, 180, 50);

    // 2. 锻造六边形点阵！(随着关闭按钮的位置自动偏移喵)
    qreal x = m_closeBtnRect.x();
    qreal y = m_closeBtnRect.y();
    qreal w = m_closeBtnRect.width();
    qreal h = m_closeBtnRect.height();
    qreal tip = 20; // 六边形两头尖尖的长度

    m_closeBtnPolygon.clear(); // 清空保安全
    m_closeBtnPolygon << QPointF(x + tip, y)          // 左上
                      << QPointF(x + w - tip, y)      // 右上
                      << QPointF(x + w, y + h / 2.0)  // 最右侧尖尖
                      << QPointF(x + w - tip, y + h)  // 右下
                      << QPointF(x + tip, y + h)      // 左下
                      << QPointF(x, y + h / 2.0);     // 最左侧尖尖

    // ========================================================
    // 🌌【高级布局】：高层级 UI 视觉层 (Z-Value = 100)
    // ========================================================
    // 1. 顶部渐隐遮罩条：拉满传入的屏幕宽度
    m_topBanner = new QGraphicsRectItem(0, 0, m_screenW, 120, this);
    m_topBanner->setBrush(QColor(0, 0, 0, 240)); // 极深的纯黑，用来遮挡卡牌
    m_topBanner->setPen(Qt::NoPen);
    m_topBanner->setZValue(100); // 绝对高空压制！永远在卡牌 (Z=0) 之上！

    // 2. 独立的大标题文字：在当前屏幕宽度下完美居中！
    m_titleText = new QGraphicsSimpleTextItem(m_title, m_topBanner); // 挂在遮罩条上
    m_titleText->setFont(QFont("Microsoft YaHei", 36, QFont::Bold));
    m_titleText->setBrush(Qt::white);
    qreal textWidth = m_titleText->boundingRect().width();
    m_titleText->setPos((m_screenW - textWidth) / 2.0, 75); // 👈 改为 75

    // 3. 独立的关闭按钮图形
    m_closeBtnVisual = new QGraphicsPolygonItem(m_closeBtnPolygon, this);
    m_closeBtnVisual->setZValue(100); // 和顶部遮罩同等高度，压住卡牌！
    m_closeBtnVisual->setBrush(QColor(80, 80, 80));
    m_closeBtnVisual->setPen(QPen(Qt::yellow, 3));

    // 🟢【Qt 绝密黑科技】：拒绝接受任何鼠标按键，让点击直接穿透到父类遮罩！
    m_closeBtnVisual->setAcceptedMouseButtons(Qt::NoButton);

    // 4. 关闭按钮上的文字
    m_closeBtnText = new QGraphicsSimpleTextItem(QStringLiteral("关 闭"), m_closeBtnVisual);
    m_closeBtnText->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    m_closeBtnText->setBrush(Qt::white);

    // 文字在按钮中央完美对齐
    QRectF textRect = m_closeBtnText->boundingRect();
    QPointF center = m_closeBtnRect.center();
    m_closeBtnText->setPos(center.x() - textRect.width() / 2.0, center.y() - textRect.height() / 2.0);

    // ========================================================
    // 🔴【核心防作弊系统】：克隆一份副本，并进行“费用-字母”双重排序！
    // ========================================================
    QList<Card*> sortedCards = cards; // 浅拷贝一份用来展示的局部列表

    std::sort(sortedCards.begin(), sortedCards.end(), [](Card* a, Card* b) {
        if (a->getCost() != b->getCost()) {
            return a->getCost() < b->getCost(); // 费用低的排在前面
        }
        return a->getName() < b->getName();     // 字母序从小到大
    });

    // ========================================================
    // 🎨 动态网格居中排版算法（全新 6 列黄金比例喵！）
    // ========================================================
    int columns = 6;      // 一行优雅地容纳 6 张牌
    qreal spacingX = 220; // 精致的横向间距
    qreal spacingY = 280; // 纵向间距
    qreal startY = 250;   // 起始 Y 坐标，让出顶部标题栏空间

    // 🔴【灵魂公式】：自动计算卡牌矩阵在当前分辨率下的两侧等距留白！
    qreal totalGridWidth = (columns - 1) * spacingX;
    qreal startX = (m_screenW - totalGridWidth) / 2.0;

    for (int i = 0; i < sortedCards.size(); ++i) {
        CardItem* cItem = new CardItem(sortedCards[i], this); // 挂载到当前遮罩上
        cItem->setDisplayOnly(true); // 开启橱窗展示模式，禁止施法

        int row = i / columns;
        int col = i % columns;

        // 运用全自动对齐位置摆放卡牌
        qreal targetX = startX + col * spacingX;
        qreal targetY = startY + row * spacingY;

        cItem->setPos(targetX, targetY);
        cItem->setHomeState(cItem->pos(), 0.0);
        m_uiCards.append(cItem);
    }

    // ========================================================
    // 🔴【弹性滚动限制】：自动匹配当前视口深度
    // ========================================================
    int totalRows = (sortedCards.size() + columns - 1) / columns; // 向上取整算出行数
    qreal totalContentHeight = startY + (totalRows * spacingY) + 100; // +100是底部留白

    // 如果内容总高度超过了当前传入的屏幕高度，就允许滚动！否则 MaxScroll 就是 0。
    m_maxScrollY = std::max(0.0, totalContentHeight - m_screenH);
    m_currentScrollY = 0.0;
}

CardBrowserOverlay::~CardBrowserOverlay() {
    // Qt 的父子对象系统会自动解构挂在它身上的所有 CardItem 图元，没有内存碎片喵！
}

QRectF CardBrowserOverlay::boundingRect() const {
    // 🔴【结界边界动态化】：彻底告别原本写死的 1600x900 裁切Bug！
    return QRectF(0, 0, m_screenW, m_screenH);
}

void CardBrowserOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    // 🔴 涂黑整个动态结界，遮蔽下方战场！
    painter->setBrush(QColor(0, 0, 0, 210));
    painter->setPen(Qt::NoPen);
    painter->drawRect(boundingRect());
}

// ========================================================
// 🖱️ 交互系统：检测点击和关闭
// ========================================================
void CardBrowserOverlay::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_closeBtnRect.contains(event->pos())) {
            emit closed(); // 点了关闭按钮，发射信号
        }
    }
    event->accept(); // 彻底吞掉点击，绝对不允许穿透影响下层界面喵！
}

// ========================================================
// 🖱️ 滚轮事件：捕获鼠标滚动，驱动镜头！
// ========================================================
void CardBrowserOverlay::wheelEvent(QGraphicsSceneWheelEvent* event) {
    if (m_maxScrollY <= 0) {
        return; // 牌很少，一屏装得下，直接无视滚轮喵！
    }

    qreal scrollStep = 50.0; // 滚动灵敏度

    if (event->delta() > 0) {
        m_currentScrollY -= scrollStep; // 向上滚
    } else {
        m_currentScrollY += scrollStep; // 向下滚
    }

    // 用 std::clamp 锁死界限，防飞出平流层
    m_currentScrollY = std::clamp(m_currentScrollY, 0.0, m_maxScrollY);

    // 刷新所有手牌位置！
    updateCardPositions();
    event->accept();
}

// ========================================================
// 🔄 动态刷新：保持完美居中对齐的重新移动
// ========================================================
void CardBrowserOverlay::updateCardPositions() {
    int columns = 6;
    qreal spacingX = 220;
    qreal spacingY = 280;
    qreal startY = 250;

    // 🔴 保持一模一样的自动居中偏置量计算！
    qreal totalGridWidth = (columns - 1) * spacingX;
    qreal startX = (m_screenW - totalGridWidth) / 2.0;

    for (int i = 0; i < m_uiCards.size(); ++i) {
        int row = i / columns;
        int col = i % columns;

        qreal targetX = startX + col * spacingX;
        // 灵魂减法：减去当前的滚动量，实现卡牌整体向上升腾！
        qreal targetY = startY + row * spacingY - m_currentScrollY;

        m_uiCards[i]->setPos(targetX, targetY);
        m_uiCards[i]->setHomeState(QPointF(targetX, targetY), 0.0);
    }
}

void CardBrowserOverlay::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    bool hovered = m_closeBtnRect.contains(event->pos());
    if (hovered != m_isCloseBtnHovered) {
        m_isCloseBtnHovered = hovered;

        // 图元高速变色，规避重绘性能消耗！
        QColor btnColor = hovered ? QColor(220, 50, 50) : QColor(80, 80, 80);
        m_closeBtnVisual->setBrush(btnColor);
    }
}