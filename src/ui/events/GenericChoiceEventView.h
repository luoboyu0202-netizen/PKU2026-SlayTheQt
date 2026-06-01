#pragma once
#include "EventBaseView.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <functional>
#include <QList>
#include "../../logic/GlobalSaveData.h" // 🔴 引入唯一真神！

class EventOptionButton;

class GenericChoiceEventView : public EventBaseView {
    Q_OBJECT
public:
    explicit GenericChoiceEventView(Player* player, CardManager* cardManager, 
                                     RelicManager* relicManager, QWidget* parent = nullptr);

    void setEventImage(const QString& path);
    void setTitle(const QString& title);
    void setDescription(const QString& desc);
    void addOption(const QString& text, std::function<void()> onClick, bool enabled = true);
    void clearOptions();
    void setOptionsEnabled(bool enabled); // 新增：控制所有选项的可用性

protected:
    void setupContent();

private:
    QGraphicsPixmapItem* m_bgItem = nullptr;
    QGraphicsPixmapItem* m_imageFrame = nullptr;
    QGraphicsPixmapItem* m_eventImageItem = nullptr;
    
    QGraphicsPixmapItem* m_titleRibbon = nullptr;
    QGraphicsTextItem* m_titleText = nullptr;
    
    // 替换为可滑动的 TextBrowser
    class QTextBrowser* m_descBrowser = nullptr;
    class QGraphicsProxyWidget* m_descProxy = nullptr;

    QList<EventOptionButton*> m_options;
    QGraphicsRectItem* m_optionsContainer = nullptr;
};
