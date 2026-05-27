#include "IconButton.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QImage>

IconButton::IconButton(const QString& imagePath, QGraphicsItem* parent)
    : QGraphicsObject(parent) {
    m_pixmap.load(imagePath);
    if (m_pixmap.isNull()) {
        // ... (保持搜索逻辑)
        QString fileName = QDir::cleanPath(imagePath).remove(0, 2); 
        QString dir = QCoreApplication::applicationDirPath();
        for (int i = 0; i < 6; ++i) {
            QString testPath = dir + "/" + fileName;
            if (QFile::exists(testPath)) {
                m_pixmap.load(testPath);
                break;
            }
            dir = QDir::cleanPath(dir + "/..");
        }
    }
    
    if (!m_pixmap.isNull()) {
        QImage image = m_pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        QRect contentRect;

        for (int y = 0; y < image.height(); ++y) {
            const QRgb* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                if (qAlpha(line[x]) > 8) {
                    contentRect = contentRect.isNull()
                        ? QRect(x, y, 1, 1)
                        : contentRect.united(QRect(x, y, 1, 1));
                }
            }
        }

        if (!contentRect.isNull()) {
            image = image.copy(contentRect);
        }

        // Normalize the visible artwork, not the PNG canvas. Some assets include
        // transparent padding, which otherwise makes equal button frames look uneven.
        m_pixmap = QPixmap::fromImage(image).scaled(280, 180, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
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
        // ... (保持占位逻辑)
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
