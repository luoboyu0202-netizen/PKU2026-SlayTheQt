#include "EventBaseView.h"
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "../RelicTray.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

EventBaseView::EventBaseView(Player* player, CardManager* cardManager,
                             RelicManager* relicManager, QWidget* parent)
    : QGraphicsView(parent)
    , m_player(player)
    , m_cardManager(cardManager)
    , m_relicManager(relicManager)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, 1920, 1080);
    setScene(m_scene);

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setFixedSize(1600, 900);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setStyleSheet("background-color: black; border: none;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 使用 fitInView 确保 1920x1080 完整缩放到 1600x900
    fitInView(0, 0, 1920, 1080, Qt::IgnoreAspectRatio);

    setupCommonUI();
}

void EventBaseView::setupCommonUI() {
    // ========================================================
    // 🔴 架构大扫除：剥夺基类的硬编码贴图权！
    // 基类只负责准备好用来挂贴图的“空画板”，绝对不写死任何具体路径！
    // ========================================================
    m_playerImage = new QGraphicsPixmapItem();
    m_playerImage->setZValue(50); // 默认层级
    m_scene->addItem(m_playerImage); // 把空画板挂到场景里

    // 2. 右下角离开按钮（保留，这是通用的）
    m_leaveBtn = new LeaveButton();
    m_leaveBtn->setPos(leaveButtonPos());
    m_leaveBtn->setZValue(120);
    m_scene->addItem(m_leaveBtn);

    // ========================================================
    // 🟢 极简主义归位：点击按钮，直接向上级汇报！绝不多管闲事！
    // ========================================================
    connect(m_leaveBtn, &LeaveButton::clicked, this, [this]() {
        emit eventFinished();
    });

}

QPointF EventBaseView::playerImagePos() const {
    return QPointF(-200, 380); // 大幅放大并下移，营造近景越肩感
}

QPointF EventBaseView::leaveButtonPos() const {
    return QPointF(1650, 920);
}

void EventBaseView::setLeaveButtonVisible(bool visible) {
    if (m_leaveBtn) {
        m_leaveBtn->setVisible(visible);
    }
}

void EventBaseView::showDarkOverlay(const QString& text) {
    if (!m_darkOverlay) {
        m_darkOverlay = new QGraphicsRectItem(-5000, -5000, 12000, 12000);
        m_darkOverlay->setBrush(QColor(0, 0, 0, 220));
        m_darkOverlay->setZValue(95);
        m_scene->addItem(m_darkOverlay);

        auto* effect = new QGraphicsOpacityEffect();
        effect->setOpacity(0.0);
        m_darkOverlay->setGraphicsEffect(effect);

        auto* anim = new QPropertyAnimation(effect, "opacity");
        anim->setDuration(250);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (!text.isEmpty()) {
        m_overlayText = new QGraphicsTextItem(text);
        m_overlayText->setDefaultTextColor(Qt::white);
        QFont f("Microsoft YaHei", 24, QFont::Bold);
        m_overlayText->setFont(f);
        m_overlayText->setPos(960 - m_overlayText->boundingRect().width() / 2, 400);
        m_overlayText->setZValue(95);
        m_scene->addItem(m_overlayText);
    }
}

void EventBaseView::hideDarkOverlay() {
    if (m_darkOverlay) {
        m_scene->removeItem(m_darkOverlay);
        delete m_darkOverlay;
        m_darkOverlay = nullptr;
    }
    if (m_overlayText) {
        m_scene->removeItem(m_overlayText);
        delete m_overlayText;
        m_overlayText = nullptr;
    }
}

