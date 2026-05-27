#include <QApplication>
#include <QScrollArea>
#include "map/mapmanager.h"
#include "logic/GlobalSaveData.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    GlobalSaveData::getInstance()->initNewGame();

    MapManager* mapWidget = new MapManager();

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWindowTitle("Slay the Qt - 大地图测试");
    scrollArea->setWidget(mapWidget);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setFixedSize(1280, 720);
    scrollArea->show();

    return a.exec();
}
