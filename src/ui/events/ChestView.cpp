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
#include "logic/GlobalSaveData.h"
#include <QPropertyAnimation>

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

    // 清理粒子
    for (auto* p : m_sparkleParticles) {
        m_scene->removeItem(p);
        delete p;
    }
    m_sparkleParticles.clear();

    // 智能搖獎
    GlobalSaveData* save = GlobalSaveData::getInstance();
    QString droppedRelicId = RelicFactory::generateRandomRelic(save->relicIds);

    if (droppedRelicId.isEmpty()) {
        showResult();
        return;
    }

    m_offeredRelic = RelicFactory::createRelic(droppedRelicId, this);

    // 🌟 寶箱 Q 彈換圖 (使用 QVariantAnimation 解決 Error 1)
    if (!m_chestOpenPixmap.isNull()) {
        m_chestItem->setPixmap(m_chestOpenPixmap.scaled(700, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_chestItem->setTransformOriginPoint(m_chestItem->boundingRect().center());

        QVariantAnimation* popAnim = new QVariantAnimation(this);
        popAnim->setDuration(400);
        popAnim->setStartValue(1.0);
        popAnim->setKeyValueAt(0.4, 1.15); // 膨脹
        popAnim->setEndValue(1.0);
        popAnim->setEasingCurve(QEasingCurve::OutBack);
        connect(popAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v){
            m_chestItem->setScale(v.toReal());
        });
        popAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 🌟 延遲 400ms 等寶箱彈完，再優雅地召喚隊友的選擇 UI！
    QTimer::singleShot(400, this, [this]() {
        m_relicPopup = new RelicPopupWidget(m_offeredRelic, m_scene, this);

        connect(m_relicPopup, &RelicPopupWidget::takeClicked, this, [this]() {
            onTakeRelic(m_offeredRelic);
        });
        connect(m_relicPopup, &RelicPopupWidget::skipClicked, this, [this]() {
            onSkipRelic();
        });
    });
}

void ChestView::onTakeRelic(Relic* relic) {
    // 🔴 修正 Error 2：不要呼叫 hide()，直接 delete，隊友的解構子會自動清空畫面！
    if (m_relicPopup) {
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }

    // 1. 寫入存檔
    GlobalSaveData::getInstance()->relicIds.append(relic->getId());

    // 2. 觸發流星 (從寶箱口飛出)
    QPointF startPos = m_chestItem->sceneBoundingRect().center();
    startPos.setY(startPos.y() - 150);
    playFlightEffect(relic, startPos); // 👈 這裡呼叫我們上一回合寫好的特效函數

    // 3. 延遲發貨
    QTimer::singleShot(800, this, [this, relic]() {
        emit relicObtained(relic);
        showResult();
    });
}

void ChestView::onSkipRelic() {
    // 直接銷毀 UI
    if (m_relicPopup) {
        delete m_relicPopup;
        m_relicPopup = nullptr;
    }
    // 玩家不要這個遺物，無情銷毀它的肉身！
    if (m_offeredRelic) {
        delete m_offeredRelic;
        m_offeredRelic = nullptr;
    }

    // 顯示離開按鈕
    showResult();
}

void ChestView::showResult() {
    if (m_leaveBtn) {
        m_leaveBtn->setIcon(":/resources/images/events/Campfire/GO_ahead.png");
        m_leaveBtn->setText("");
        m_leaveBtn->show();
    }
}

void ChestView::playFlightEffect(Relic* relic, const QPointF& startPos) {
    // 建立跨次元結界
    QGraphicsView* fxView = new QGraphicsView(this->window());
    fxView->resize(this->window()->size());
    fxView->setStyleSheet("background: transparent; border: none;");
    fxView->setAttribute(Qt::WA_TransparentForMouseEvents);
    fxView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fxView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* fxScene = new QGraphicsScene(0, 0, fxView->width(), fxView->height(), fxView);
    fxView->setScene(fxScene);
    fxView->show();
    fxView->raise();

    // 計算精準落點
    GlobalSaveData* save = GlobalSaveData::getInstance();
    int currentIndex = save->relicIds.size() - 1; // 抵銷提早入帳的偏移量
    if (currentIndex < 0) currentIndex = 0;

    QPointF endPos(10 + currentIndex * (48 + 8) + 24, 55 + 24);

    // 轉換坐標系
    QPoint viewPos = this->mapFromScene(startPos);
    QPointF absoluteStartPos = this->mapTo(this->window(), viewPos);

    QPointF ctrlPos(absoluteStartPos.x() + (endPos.x() - absoluteStartPos.x()) * 0.4, absoluteStartPos.y() - 350);

    const int totalSteps = 50;
    const int interval = 16;

    // 繪製高能光球
    auto* glowOrb = new QGraphicsEllipseItem(-25, -25, 50, 50);
    QRadialGradient gradient(0, 0, 25);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));
    gradient.setColorAt(0.3, QColor(50, 200, 255, 255)); // 遺物專屬幽藍
    gradient.setColorAt(1.0, QColor(0, 100, 255, 0));
    glowOrb->setBrush(gradient);
    glowOrb->setPen(Qt::NoPen);
    glowOrb->setPos(absoluteStartPos);

    auto* blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(10);
    glowOrb->setGraphicsEffect(blur);
    fxScene->addItem(glowOrb);

    struct TrailParticle { QGraphicsEllipseItem* dot; qreal dx, dy; int age; int life; };
    auto* trails = new QList<TrailParticle>();
    int* pStep = new int(0);
    auto* timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [=]() {
        (*pStep)++;
        int s = *pStep;
        qreal t = qreal(s) / totalSteps;
        qreal easedT = t * t * (3 - 2 * t);
        qreal u = 1.0 - easedT;
        QPointF currentPos = u * u * absoluteStartPos + 2 * u * easedT * ctrlPos + easedT * easedT * endPos;
        glowOrb->setPos(currentPos);
        glowOrb->setScale(1.0 + sin(t * 3.14159) * 0.3);

        for(int i = 0; i < 2; i++) {
            auto* dot = new QGraphicsEllipseItem(-4, -4, 8, 8);
            dot->setBrush(QColor(100, 220, 255, 200));
            dot->setPen(Qt::NoPen);
            dot->setPos(currentPos + QPointF((QRandomGenerator::global()->generateDouble()-0.5)*20, (QRandomGenerator::global()->generateDouble()-0.5)*20));
            fxScene->addItem(dot);

            qreal dx = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            qreal dy = (QRandomGenerator::global()->generateDouble() - 0.5) * 4;
            trails->append({dot, dx, dy, 0, 10 + QRandomGenerator::global()->bounded(8)});
        }

        for (int i = trails->size() - 1; i >= 0; --i) {
            auto& tr = (*trails)[i];
            tr.age++;
            tr.dot->moveBy(tr.dx, tr.dy);
            qreal lifeRatio = qreal(tr.age) / tr.life;
            tr.dot->setOpacity(1.0 - lifeRatio);
            tr.dot->setScale(1.0 - lifeRatio * 0.5);
            if (tr.age >= tr.life) {
                fxScene->removeItem(tr.dot);
                delete tr.dot;
                trails->removeAt(i);
            }
        }

        if (s >= totalSteps) {
            timer->stop();
            for(int i = 0; i < 10; i++) {
                auto* spark = new QGraphicsEllipseItem(-3, -3, 6, 6);
                spark->setBrush(Qt::white);
                spark->setPen(Qt::NoPen);
                spark->setPos(endPos);
                fxScene->addItem(spark);

                qreal angle = i * (3.14159 * 2 / 10.0);
                qreal speed = 5.0 + QRandomGenerator::global()->generateDouble() * 3.0;
                qreal vX = cos(angle) * speed;
                qreal vY = sin(angle) * speed;

                auto* sparkTimer = new QTimer(fxView);
                int* sparkAge = new int(0);
                connect(sparkTimer, &QTimer::timeout, fxView, [=]() {
                    (*sparkAge)++;
                    spark->moveBy(vX, vY);
                    spark->setOpacity(1.0 - (*sparkAge) / 12.0);
                    if(*sparkAge >= 12) {
                        sparkTimer->stop();
                        fxScene->removeItem(spark);
                        delete spark;
                        delete sparkAge;
                        sparkTimer->deleteLater();
                    }
                });
                sparkTimer->start(16);
            }

            for (auto& tr : *trails) { fxScene->removeItem(tr.dot); delete tr.dot; }
            delete trails;
            fxScene->removeItem(glowOrb);
            delete glowOrb;
            delete pStep;
            timer->deleteLater();

            QTimer::singleShot(500, fxView, [fxView]() {
                fxView->hide();
                fxView->deleteLater();
            });
        }
    });
    timer->start(interval);
}