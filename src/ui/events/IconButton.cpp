#include "IconButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>

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
    if (!m_pixmap.isNull())
        m_pixmap = m_pixmap.scaled(280, 180, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    setAcceptHoverEvents(true);
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
        painter->drawPixmap(boundingRect().toRect(), m_pixmap);
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

void IconButton::hoverEnterEvent(QGraphicsSceneHoverEvent*) { m_hovered = true; update(); }
void IconButton::hoverLeaveEvent(QGraphicsSceneHoverEvent*) { m_hovered = false; update(); }
void IconButton::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) { m_pressed = true; update(); e->accept(); }
}
void IconButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false; update(); emit clicked(); e->accept();
    }
}
