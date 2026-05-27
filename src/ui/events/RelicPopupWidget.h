#pragma once

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

class Relic;
class TextButton;

class RelicPopupWidget : public QObject {
    Q_OBJECT

public:
    explicit RelicPopupWidget(Relic* relic, QGraphicsScene* scene, QObject* parent = nullptr);
    ~RelicPopupWidget();

signals:
    void takeClicked();
    void skipClicked();

private:
    void setupUI();
    void showTooltip();
    void hideTooltip();

    Relic* m_relic;
    QGraphicsScene* m_scene;
    QGraphicsRectItem* m_root = nullptr;
    QGraphicsRectItem* m_cardBg = nullptr;
    QGraphicsPixmapItem* m_relicIcon = nullptr;
    QGraphicsTextItem* m_nameText = nullptr;
    QGraphicsTextItem* m_descText = nullptr;
    TextButton* m_takeBtn = nullptr;
    TextButton* m_skipBtn = nullptr;
    QGraphicsRectItem* m_tooltipBg = nullptr;
    QGraphicsTextItem* m_tooltipText = nullptr;
    bool m_tooltipInstalled = false;
};
