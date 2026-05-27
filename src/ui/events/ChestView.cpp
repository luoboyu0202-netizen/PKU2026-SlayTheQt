#include "ChestView.h"
#include "RelicPopupWidget.h"
#include "../../entities/relics/Relic.h"
#include "../../entities/relics/RelicManager.h"
#include "../../entities/Player.h"
#include "../../logic/RelicFactory.h"
#include <QRandomGenerator>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>

ChestView::ChestView(Player* player, RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, nullptr, relicManager, parent)
{
    setupContent();
}

void ChestView::setupContent() {
    // 1. Background image
    QPixmap bg(":/resources/images/events/Chest/Background.png");
    if (!bg.isNull()) {
        m_bgItem = new QGraphicsPixmapItem();
        m_bgItem->setPixmap(bg.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_bgItem->setPos(0, 0);
        m_bgItem->setZValue(-10);
        m_scene->addItem(m_bgItem);
    }

    // 2. Player on left side (matching battle perspective)
    if (m_playerImage) {
        m_playerImage->setPixmap(m_playerPixmap.scaled(450, 675, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_playerImage->setPos(100, 450);
        m_playerImage->setZValue(10);
    }

    // 3. Chest image (700px, Y lowered to match player ground level)
    QPixmap chestClosed(":/resources/images/events/Chest/Chest.png");
    const int chestSize = 700;
    m_chestItem = new QGraphicsPixmapItem();
    if (!chestClosed.isNull()) {
        m_chestItem->setPixmap(chestClosed.scaled(chestSize, chestSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_chestItem->setPos(1000, 430);
    m_chestItem->setZValue(15);
    m_scene->addItem(m_chestItem);

    // Preload open chest pixmap
    m_chestOpenPixmap.load(":/resources/images/events/Chest/Chest_open.png");

    // 4. Sparkle particles around chest
    auto* rng = QRandomGenerator::global();
    const QPointF chestCenter(1350, 780);

    for (int i = 0; i < 15; ++i) {
        qreal size = 4 + rng->bounded(8);
        auto* sparkle = new QGraphicsEllipseItem(-size / 2, -size / 2, size, size, nullptr);
        sparkle->setBrush(QColor(255, 250, 180, 200 + rng->bounded(55)));
        sparkle->setPen(Qt::NoPen);

        qreal angle = rng->bounded(360) * M_PI / 180.0;
        qreal dist = 100 + rng->bounded(250);
        sparkle->setPos(chestCenter.x() + cos(angle) * dist,
                        chestCenter.y() + sin(angle) * dist - rng->bounded(120));
        sparkle->setZValue(16);
        m_scene->addItem(sparkle);
        m_sparkleParticles.append(sparkle);

        auto* opacityEff = new QGraphicsOpacityEffect();
        opacityEff->setOpacity(0.3 + rng->bounded(70) / 100.0);
        sparkle->setGraphicsEffect(opacityEff);

        auto* twinkle = new QVariantAnimation(opacityEff);
        twinkle->setDuration(600 + rng->bounded(800));
        twinkle->setLoopCount(-1);
        twinkle->setKeyValueAt(0.0, 0.3);
        twinkle->setKeyValueAt(0.5, 1.0);
        twinkle->setKeyValueAt(1.0, 0.3);
        twinkle->setEasingCurve(QEasingCurve::InOutSine);
        connect(twinkle, &QVariantAnimation::valueChanged, opacityEff,
                [opacityEff](const QVariant& v) { opacityEff->setOpacity(v.toReal()); });
        twinkle->start();
    }

    // 5. Show skip button from the start (icon has baked-in text)
    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Chest/Skip-055246b5-b7d6-46de-9fe4-8d4af5c814cc.jpg");
        m_leaveBtn->setText("");
        m_leaveBtn->setPos(1600, 880);
        m_leaveBtn->show();
    }
}

void ChestView::mousePressEvent(QMouseEvent* event) {
    QPointF scenePt = mapToScene(event->pos());

    // Guard: don't treat clicks on the leave-button area as chest clicks
    if (m_leaveBtn && m_leaveBtn->isVisible()) {
        QRectF btnArea = m_leaveBtn->sceneBoundingRect();
        btnArea.adjust(-20, -20, 20, 20); // small tolerance
        if (btnArea.contains(scenePt)) {
            QGraphicsView::mousePressEvent(event);
            return;
        }
    }

    // Chest click area (matches chest pixmap area, not overlapping button)
    QRectF chestRect(1000, 430, 500, 550);
    if (!m_chestOpened && chestRect.contains(scenePt)) {
        onChestClicked();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ChestView::onChestClicked() {
    m_chestOpened = true;
    qDebug() << "[ChestView] Chest clicked!";

    // Clear sparkle particles
    for (auto* p : m_sparkleParticles) {
        m_scene->removeItem(p);
        delete p;
    }
    m_sparkleParticles.clear();

    // Generate random relic
    m_offeredRelic = RelicFactory::generateRandomRelic(this);
    if (!m_offeredRelic) {
        qDebug() << "[ChestView] ERROR: RelicFactory returned null!";
        return;
    }
    qDebug() << "[ChestView] Generated relic:" << m_offeredRelic->getName();

    // Swap to open chest image
    if (!m_chestOpenPixmap.isNull()) {
        m_chestItem->setPixmap(m_chestOpenPixmap.scaled(700, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    qDebug() << "[ChestView] Chest image swapped, scheduling popup...";

    // Show relic popup after brief delay
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "[ChestView] Creating RelicPopupWidget...";
        m_relicPopup = new RelicPopupWidget(m_offeredRelic, m_scene, this);

        connect(m_relicPopup, &RelicPopupWidget::takeClicked, this, [this]() {
            qDebug() << "[ChestView] Player chose to take relic";
            onTakeRelic(m_offeredRelic);
        });
        connect(m_relicPopup, &RelicPopupWidget::skipClicked, this, [this]() {
            qDebug() << "[ChestView] Player chose to skip relic";
            onSkipRelic();
        });

        qDebug() << "[ChestView] RelicPopupWidget created and connected.";
    });
}

void ChestView::onTakeRelic(Relic* relic) {
    if (m_relicPopup) {
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }
    m_relicManager->addRelic(relic);
    showResult();
}

void ChestView::onSkipRelic() {
    if (m_relicPopup) {
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }
    if (m_offeredRelic) {
        delete m_offeredRelic;
        m_offeredRelic = nullptr;
    }
    showResult();
}

void ChestView::showResult() {
    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
        m_leaveBtn->setText("");
        m_leaveBtn->show();
    }
}
