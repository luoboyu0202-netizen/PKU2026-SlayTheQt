#include "IconButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPropertyAnimation>

IconButton::IconButton(const QString& imagePath, QGraphicsItem* parent)
    : QGraphicsObject(parent) {
    // 先尝试Qt资源路径，再尝试文件系统路径
    m_pixmap.load(imagePath);
    if (m_pixmap.isNull()) {
        // 从exe目录向上搜索到项目根(找到resources目录为止)
        QString fileName = QDir::cleanPath(imagePath).remove(0, 2); // 去掉":/"
        QString dir = QCoreApplication::applicationDirPath();
        for (int i = 0; i < 6; ++i) {
            QString testPath = dir + "/" + fileName;
            if (QFile::exists(testPath)) {
                m_pixmap.load(testPath);
                qDebug() << "[IconButton] Found:" << testPath;
                break;
            }
            dir = QDir::cleanPath(dir + "/..");
        }
        if (m_pixmap.isNull())
            qDebug() << "[IconButton] NOT FOUND:" << fileName << "from" << QCoreApplication::applicationDirPath();
    }
    if (!m_pixmap.isNull()) {
        // 🔴 极其关键：改成 Qt::KeepAspectRatio！
        // 这样图片会等比例缩小/放大，直到刚好塞进 280x180 的盒子里，绝不变形！
        m_pixmap = m_pixmap.scaledToHeight(140, Qt::SmoothTransformation);
    }
    setAcceptHoverEvents(true);

    // ========================================================
    // 🔴 设定物理锚点：280x180 的中心点就是 (140, 90)
    // ========================================================
    setTransformOriginPoint(140, 90);
}

QRectF IconButton::boundingRect() const {
    return QRectF(0, 0, 280, 180);
}

void IconButton::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    qreal op = m_pressed ? 0.7 : m_hovered ? 1.1 : 1.0;
    painter->setOpacity(op);
    if (m_pressed) painter->translate(0, 3);

    if (!m_pixmap.isNull()) {
        // ========================================================
        // 🔴 极其优雅的“画框居中算法”！
        // ========================================================
        // 计算图片真实尺寸与 280x180 碰撞箱的差值，除以 2 得到完美的居中偏移量！
        qreal xOffset = (280.0 - m_pixmap.width()) / 2.0;
        qreal yOffset = (180.0 - m_pixmap.height()) / 2.0;

        // 用精确的浮点数坐标画出高清原图！
        painter->drawPixmap(QPointF(xOffset, yOffset), m_pixmap);
    } else {
        // 加载失败时渲染可见的占位按钮
        QRectF r = boundingRect();
        painter->setBrush(QColor(60, 50, 35));
        painter->setPen(QPen(QColor(190, 150, 80), 2));
        painter->drawRoundedRect(r.adjusted(2, 2, -2, -2), 8, 8);
        painter->setPen(QColor(230, 215, 180));
        QFont f("Microsoft YaHei", 14, QFont::Bold);
        painter->setFont(f);
        painter->drawText(r, Qt::AlignCenter, "?");
    }
}

void IconButton::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = true;
    update();

    // 🌟 鼠标进入：极其丝滑地放大到 1.15 倍！
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(150); // 150 毫秒的极速弹起
    anim->setEndValue(1.15);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void IconButton::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    m_hovered = false;
    update();

    // 🌟 鼠标离开：极其丝滑地回弹到 1.0 倍！
    QPropertyAnimation* anim = new QPropertyAnimation(this, "scale");
    anim->setDuration(150);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void IconButton::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) { m_pressed = true; update(); e->accept(); }
}
void IconButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false; update(); emit clicked(); e->accept();
    }
}
