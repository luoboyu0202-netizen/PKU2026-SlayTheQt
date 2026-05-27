#include "ui/RelicItem.h"
#include <QPainter>
#include <QPropertyAnimation> // 记得包含动画头文件喵

RelicItem::RelicItem(Relic* logicRelic, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_logicRelic(logicRelic) {

    m_displayCounter = logicRelic->getCounter();

    connect(logicRelic, &Relic::counterChanged, this, [this](int count){
        m_displayCounter = count;
        update();
    });

    // 🔴 监听遗物触发，播放 Q 弹放大特效！
    connect(logicRelic, &Relic::relicActivated, this, [this]() {
        // 让锚点居中，这样放大时就不会往右下角偏了
        setTransformOriginPoint(boundingRect().center());

        QPropertyAnimation* pop = new QPropertyAnimation(this, "scale");
        pop->setDuration(300);
        pop->setKeyValueAt(0, 1.0);
        pop->setKeyValueAt(0.5, 1.5); // 瞬间胀大 1.5 倍！
        pop->setKeyValueAt(1, 1.0);
        pop->setEasingCurve(QEasingCurve::OutBack); // 带回弹的曲线

        pop->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QRectF RelicItem::boundingRect() const {
    return QRectF(0, 0, 48, 48); // 遗物通常是 48x48 的方块
}

void RelicItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 绘制遗物底色/边框 (暂用深灰色圆形代替图标)
    painter->setPen(QPen(QColor(200, 200, 200), 2));
    painter->setBrush(QColor(40, 40, 45));
    painter->drawEllipse(2, 2, 44, 44);

    // 2. 绘制首字母缩写模拟图标
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);
    painter->drawText(boundingRect(), Qt::AlignCenter, m_logicRelic->getName().left(1));

    // 3. 绘制计数值 (如果有)
    if (m_displayCounter >= 0) {
        font.setPixelSize(14);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QColor(241, 196, 15)); // 标志性的黄色计数值
        painter->drawText(QRectF(0, 28, 45, 20), Qt::AlignRight, QString::number(m_displayCounter));
    }
}
