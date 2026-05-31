#include "GenericChoiceEventView.h"
#include "EventOptionButton.h"
#include <QGraphicsRectItem>
#include <QGraphicsOpacityEffect>
#include <QTextBrowser>
#include <QGraphicsProxyWidget>
#include <QScrollBar>
#include <QDebug>

GenericChoiceEventView::GenericChoiceEventView(Player* player, CardManager* cardManager, 
                                               RelicManager* relicManager, QWidget* parent)
    : EventBaseView(player, cardManager, relicManager, parent)
{
    // setupContent() will be called by subclasses
}

void GenericChoiceEventView::setupContent() {
    // 1. Universal Background
    QPixmap bgPix(":/resources/images/events/Random/Univ/background.jpeg");
    m_bgItem = new QGraphicsPixmapItem();
    if (!bgPix.isNull()) {
        m_bgItem->setPixmap(bgPix.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    m_bgItem->setPos(0, 0);
    m_bgItem->setZValue(-100);
    m_scene->addItem(m_bgItem);

    // 2. Event Image (Left)
    m_eventImageItem = new QGraphicsPixmapItem();
    m_eventImageItem->setPos(180, 240); 
    m_eventImageItem->setZValue(10);
    m_scene->addItem(m_eventImageItem);

    // 3. Title Ribbon (Overlap top-left of the Illustration)
    QPixmap titlePix(":/resources/images/events/Random/Univ/title.png");
    m_titleRibbon = new QGraphicsPixmapItem();
    if (!titlePix.isNull()) {
        m_titleRibbon->setPixmap(titlePix.scaled(800, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_titleRibbon->setPos(100, 160); 
    m_titleRibbon->setZValue(50); 
    m_scene->addItem(m_titleRibbon);

    m_titleText = new QGraphicsTextItem(m_titleRibbon);
    m_titleText->setDefaultTextColor(QColor(255, 215, 0)); 
    m_titleText->setFont(QFont("Microsoft YaHei", 32, QFont::Bold));
    m_titleText->setPos(400 - m_titleText->boundingRect().width() / 2, 20);

    // 4. Right Panel Content (Scrollable Text)
    const int panelX = 1000;

    m_descBrowser = new QTextBrowser();
    m_descBrowser->setReadOnly(true);
    m_descBrowser->setFixedSize(750, 350); 
    m_descBrowser->setStyleSheet(
        "QTextBrowser {"
        "  background: transparent;"
        "  border: none;"
        "  color: white;"
        "}"
        "QScrollBar:vertical {"
        "  width: 8px;"
        "  background: rgba(255, 255, 255, 20);"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: rgba(255, 255, 255, 50);"
        "  border-radius: 4px;"
        "}"
    );
    
    m_descProxy = new QGraphicsProxyWidget();
    m_descProxy->setWidget(m_descBrowser);
    m_descProxy->setPos(panelX, 280);
    m_descProxy->setZValue(10);
    m_scene->addItem(m_descProxy);

    // Options Container
    m_optionsContainer = new QGraphicsRectItem();
    m_optionsContainer->setPen(Qt::NoPen);
    m_optionsContainer->setPos(panelX - 50, 650); 
    m_optionsContainer->setZValue(20);
    m_scene->addItem(m_optionsContainer);

    if (m_playerImage) m_playerImage->hide();
    if (m_leaveBtn) m_leaveBtn->hide();
}

void GenericChoiceEventView::setEventImage(const QString& path) {
    QPixmap pix(path);
    if (!pix.isNull()) {
        m_eventImageItem->setPixmap(pix.scaled(650, 650, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void GenericChoiceEventView::setTitle(const QString& title) {
    m_titleText->setPlainText(title);
    m_titleText->setPos(400 - m_titleText->boundingRect().width() / 2, 20);
}

void GenericChoiceEventView::setDescription(const QString& desc) {
    QString formattedDesc = desc;
    formattedDesc.replace("\n", "<br>");

    QString html = QString(
        "<html><body style='font-family:\"Microsoft YaHei\"; font-size:20pt; color:white;'>"
        "%1"
        "</body></html>"
    ).arg(formattedDesc);
    
    m_descBrowser->setHtml(html);
}

void GenericChoiceEventView::addOption(const QString& text, std::function<void()> onClick, bool enabled) {
    auto* btn = new EventOptionButton(text, onClick, m_optionsContainer);
    btn->setPos(0, m_options.size() * 80);
    btn->setEnabled(enabled);
    m_options.append(btn);
}

void GenericChoiceEventView::clearOptions() {
    for (auto* opt : m_options) {
        m_scene->removeItem(opt);
        delete opt;
    }
    m_options.clear();
}

void GenericChoiceEventView::setOptionsEnabled(bool enabled) {
    for (auto* opt : m_options) {
        opt->setEnabled(enabled);
    }
}
