#include "RelicItem.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QDebug>

RelicItem::RelicItem(Relic* logicRelic, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_logicRelic(logicRelic) {

    m_displayCounter = logicRelic->getCounter();

    // ========================================================
    // 🖼️ 1. 自动加载专属贴图！
    // 假设你的图片放在资源文件里的 /resources/images/relics/ 目录下
    // ========================================================
    QString imgPath = QString(":/resources/images/relics/%1.png").arg(logicRelic->getId());
    if (!m_pixmap.load(imgPath)) {
        qDebug() << "[UI] 警告：找不到遗物贴图，只能用灰色圆圈代替喵！试图寻找路径：" << imgPath;
    }

    // ========================================================
    // 💡 2. 悬停提示魔法：利用 Qt 富文本 (HTML) 实现果汁感文字！
    // ========================================================
    setAcceptHoverEvents(true); // 🔴 极其重要：允许接收鼠标悬停事件

    // 用 HTML 拼装出标题带颜色、正文白色的酷炫提示框
    QString tooltipHtml = QString(
                              "<div style='background-color:#2b2d31; padding:5px; border-radius:4px;'>"
                              "<b style='color:#F1C40F; font-size:16px;'>%1</b><br>"
                              "<span style='color:#ecf0f1; font-size:14px;'>%2</span>"
                              "</div>"
                              ).arg(logicRelic->getName(), logicRelic->getDescription());

    setToolTip(tooltipHtml); // 挂载悬停提示！

    // --- 信号绑定和弹跳动画保持原样喵 ---
    connect(logicRelic, &Relic::counterChanged, this, [this](int count){
        m_displayCounter = count;
        update();
    });

    connect(logicRelic, &Relic::relicActivated, this, [this]() {
        setTransformOriginPoint(boundingRect().center());
        QPropertyAnimation* pop = new QPropertyAnimation(this, "scale");
        pop->setDuration(300);
        pop->setKeyValueAt(0, 1.0);
        pop->setKeyValueAt(0.5, 1.5);
        pop->setKeyValueAt(1, 1.0);
        pop->setEasingCurve(QEasingCurve::OutBack);
        pop->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

QRectF RelicItem::boundingRect() const {
    return QRectF(0, 0, 48, 48); // 遗物通常是 48x48 的方块
}

void RelicItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    // ========================================================
    // 🎨 1. 绘制遗物本体 (坚决使用我们在构造函数里预加载的高速缓存！)
    // ========================================================
    if (!m_pixmap.isNull()) {
        painter->drawPixmap(boundingRect().toRect(), m_pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        // 【防闪退兜底机制】：没找到图片就画一个带字灰圈圈
        painter->setPen(QPen(QColor(200, 200, 200), 2));
        painter->setBrush(QColor(40, 40, 45));
        painter->drawEllipse(2, 2, 44, 44);
        painter->setPen(Qt::white);
        QFont font = painter->font();
        font.setPixelSize(12);
        painter->setFont(font);
        painter->drawText(boundingRect(), Qt::AlignCenter, m_logicRelic->getName().left(3));
    }

    // ========================================================
    // 🔢 2. 绘制高对比度计数值 (如果有)
    // ========================================================
    if (m_displayCounter >= 0) {
        QFont font = painter->font();
        font.setPixelSize(16); // 保持我们更具张力的 16 号字！
        font.setBold(true);
        painter->setFont(font);

        QString counterStr = QString::number(m_displayCounter);
        QRectF textRect(0, 28, 45, 20); // 数字显示在右下角

        // 🔴 细节魔法：黑色的“描边阴影”，防止数字和明亮的遗物图片混在一起！
        painter->setPen(QColor(0, 0, 0, 200));
        painter->drawText(textRect.translated(1, 1), Qt::AlignRight, counterStr);

        // 金灿灿的正文数字
        painter->setPen(QColor(241, 196, 15));
        painter->drawText(textRect, Qt::AlignRight, counterStr);
    }
}