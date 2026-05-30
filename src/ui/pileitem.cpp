#include "PileItem.h"
#include <QPainter>
#include <QPainterPath>   // 🔴 描边字体必备
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>

PileItem::PileItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_name(name), m_count(0) {
    setAcceptHoverEvents(true);

    // ========================================================
    // 🖼️ 1. 智能贴图加载中心 (请把路径换成你自己的资源路径喵！)
    // ========================================================
    if (name == "抽牌堆") {
        m_iconPixmap.load(":/resources/images/ui/draw_pile.png");
    } else if (name == "弃牌堆") {
        m_iconPixmap.load(":/resources/images/ui/discard_pile.png");
    } else if (name == "总牌组" || name == "总牌堆") {
        m_iconPixmap.load(":/resources/images/ui/deck_pile.png");
    } else if (name == "消耗堆") {
        m_iconPixmap.load(":/resources/images/ui/exhaust_pile.png");
    }

    // 统一下采样：把原图平滑缩放成大约 64x64 的精致尺寸
    if (!m_iconPixmap.isNull()) {
        // 🔴 分级缩放魔法：总牌堆保持 60 左右的精致大小，其他的放大到 110！
        int targetSize = (name == "总牌组" || name == "总牌堆") ? 60 : 110;
        m_iconPixmap = m_iconPixmap.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void PileItem::updateCount(int count) {
    if (m_count != count) {
        m_count = count;
        // 🔴 明确命令 Qt：把整个领地彻彻底底给我重新画一遍！
        update(boundingRect());
    }
}

QRectF PileItem::boundingRect() const {
    // 🔴 领地扩张至 80x80！
    // 确保能完美包裹住 64x64 的贴图，以及右下角可能飞出来的数字气泡！
    return QRectF(-70, -70, 140, 140);
}

void PileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform); // 开启贴图平滑

    // ========================================================
    // 🎨 1. 画出极其精美的底部图标
    // ========================================================
    if (!m_iconPixmap.isNull()) {
        // 完美居中画出贴图
        int px = -m_iconPixmap.width() / 2;
        int py = -m_iconPixmap.height() / 2;
        painter->drawPixmap(px, py, m_iconPixmap);
    } else {
        // 防闪退兜底：如果路径写错了，画个灰底
        painter->setBrush(QColor(50, 50, 50));
        painter->drawRoundedRect(-25, -30, 50, 60, 8, 8);
    }

    // ========================================================
    // 🏅 2. 像素级复刻原版数字特效！
    // ========================================================
    if (m_count >= 0) {
        QString countStr = QString::number(m_count);

        // 🔴 样式 A：总牌库的“粗黑底描边字”（对应你发的图 3）
        if (m_name == "总牌组" || m_name == "总牌堆") {
            QFont font("Arial", 20, QFont::Black); // 使用极其厚重的 Black 字重
            QPainterPath path;

            // 计算文字宽度以实现右下角对齐
            QFontMetrics fm(font);
            int tw = fm.horizontalAdvance(countStr);
            path.addText(15 - tw / 2, 25, font, countStr); // X,Y 为文字左下角的基线

            // 第一层：极粗的纯黑描边（模拟杀戮尖塔的粗犷感）
            painter->setPen(QPen(QColor(20, 20, 20), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);

            // 第二层：纯白色的文字本体
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            painter->drawPath(path);
        }
        // 🔴 样式 B：抽牌堆/弃牌堆的“红圈白字徽章”
        else {
            // 将红圈定在变大后的图标的右下角 (比如 X=20, Y=20 的位置)，并把气泡稍微放大到 34x34
            QRectF badgeRect(20, 20, 34, 34);

            painter->setBrush(QColor(190, 60, 45));
            painter->setPen(QPen(QColor(30, 20, 20), 2));
            painter->drawEllipse(badgeRect);

            // 字体也稍微调大一点点，匹配变大的气泡
            painter->setFont(QFont("Arial", 16, QFont::Bold));
            painter->setPen(Qt::white);
            painter->drawText(badgeRect, Qt::AlignCenter, countStr);
        }
    }
}

void PileItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(); // 发射被点击信号！
        event->accept();
    }
}