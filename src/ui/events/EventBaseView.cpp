#include "EventBaseView.h"
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "../RelicTray.h"

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
    // 1. 左下角玩家画像（越肩视角）
    m_playerPixmap.load(":/resources/images/ironclad.png");
    if (!m_playerPixmap.isNull()) {
        m_playerImage = new QGraphicsPixmapItem();
        QPixmap scaled = m_playerPixmap.scaled(800, 1200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_playerImage->setPixmap(scaled);
        m_playerImage->setPos(playerImagePos());
        m_playerImage->setZValue(50); 
        m_scene->addItem(m_playerImage);
    }

    // 2. 右下角离开按钮
    m_leaveBtn = new LeaveButton();
    m_leaveBtn->setPos(leaveButtonPos());
    m_leaveBtn->setZValue(120);
    m_scene->addItem(m_leaveBtn);

    connect(m_leaveBtn, &LeaveButton::clicked, this, [this]() {
        emit eventFinished();
    });

    // 3. 顶部状态栏（最后添加以确保显示，强制 (0,0)）
    m_topBar = new TopBar();
    m_topBar->bindPlayer(m_player);
    m_topBar->setPos(0, 0);
    m_topBar->setZValue(200);
    m_scene->addItem(m_topBar);

    // 4. 遗物栏（匹配战斗模块布局）
    m_relicTray = new RelicTray();
    m_relicTray->bindManager(m_relicManager);
    m_relicTray->setPos(450, 6); // 在角色名和HP右侧
    m_relicTray->setZValue(200);
    m_scene->addItem(m_relicTray);
}

QPointF EventBaseView::playerImagePos() const {
    return QPointF(-200, 380); // 大幅放大并下移，营造近景越肩感
}

QPointF EventBaseView::leaveButtonPos() const {
    return QPointF(1530, 865);
}

void EventBaseView::setLeaveButtonVisible(bool visible) {
    if (m_leaveBtn) {
        m_leaveBtn->setVisible(visible);
    }
}

void EventBaseView::showDarkOverlay(const QString& text) {
    if (!m_darkOverlay) {
        m_darkOverlay = new QGraphicsRectItem(0, 0, 1920, 1080);
        m_darkOverlay->setBrush(QColor(0, 0, 0, 80));
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
