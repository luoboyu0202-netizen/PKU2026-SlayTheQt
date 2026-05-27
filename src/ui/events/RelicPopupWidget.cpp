#include "RelicPopupWidget.h"
#include "TextButton.h"
#include "../../entities/relics/Relic.h"
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPixmap>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QDebug>

RelicPopupWidget::RelicPopupWidget(Relic* relic, QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_relic(relic)
    , m_scene(scene)
{
    setupUI();
}

RelicPopupWidget::~RelicPopupWidget() {
    // Root item owns all children; removing it from scene cleans up everything
    if (m_root) {
        m_scene->removeItem(m_root);
        delete m_root;
    }
}

void RelicPopupWidget::setupUI() {
    // Root item — transparent full-screen container
    m_root = new QGraphicsRectItem(0, 0, 1920, 1080);
    m_root->setZValue(200);
    m_root->setPen(Qt::NoPen);
    m_root->setBrush(QColor(0, 0, 0, 120));
    m_scene->addItem(m_root);

    // Central card panel
    m_cardBg = new QGraphicsRectItem(0, 0, 600, 400, m_root);
    m_cardBg->setPos(660, 320);
    m_cardBg->setBrush(QColor(30, 25, 20, 240));
    m_cardBg->setPen(QPen(QColor(180, 150, 100), 3));

    // Title
    auto* titleText = new QGraphicsTextItem("发现遗物！", m_cardBg);
    titleText->setDefaultTextColor(QColor(255, 215, 120));
    titleText->setFont(QFont("Microsoft YaHei", 28, QFont::Bold));
    titleText->setPos(300 - titleText->boundingRect().width() / 2, 18);

    // Relic icon
    m_relicIcon = new QGraphicsPixmapItem(m_cardBg);
    QPixmap iconPixmap(":/resources/images/relics/" + m_relic->getId() + ".png");
    if (iconPixmap.isNull()) {
        iconPixmap = QPixmap(80, 80);
        iconPixmap.fill(QColor(100, 180, 220));
    }
    m_relicIcon->setPixmap(iconPixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_relicIcon->setPos(60, 90);

    // Relic name
    m_nameText = new QGraphicsTextItem(m_relic->getName(), m_cardBg);
    m_nameText->setDefaultTextColor(QColor(255, 250, 240));
    m_nameText->setFont(QFont("Microsoft YaHei", 22, QFont::Bold));
    m_nameText->setPos(160, 95);

    // Relic short description
    m_descText = new QGraphicsTextItem(m_relic->getDescription(), m_cardBg);
    m_descText->setDefaultTextColor(QColor(200, 195, 185));
    m_descText->setFont(QFont("Microsoft YaHei", 14));
    m_descText->setPos(60, 190);
    m_descText->setTextWidth(480);

    // Take button
    m_takeBtn = new TextButton("拾取", 180, 50, m_cardBg);
    m_takeBtn->setPos(190, 310);
    connect(m_takeBtn, &TextButton::clicked, this, &RelicPopupWidget::takeClicked);

    // Skip button
    m_skipBtn = new TextButton("跳过", 180, 50, m_cardBg);
    m_skipBtn->setPos(410, 310);
    connect(m_skipBtn, &TextButton::clicked, this, &RelicPopupWidget::skipClicked);

    qDebug() << "[RelicPopupWidget] Created popup for relic:" << m_relic->getName();
}
