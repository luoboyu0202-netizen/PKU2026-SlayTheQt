#include <QApplication>
#include <QScrollArea>
#include "map/mapmanager.h"
#include "logic/GlobalSaveData.h" // 👈 引入你的全局数据中心

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 🔴 游戏开局：初始化全局存档数据！
    GlobalSaveData::getInstance()->initNewGame();

    // 1. 实例化大地图
    MapManager* mapWidget = new MapManager();

    // 2. 创造滚动外壳
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWindowTitle("Slay the Qt - 大地图测试");
    scrollArea->setWidget(mapWidget);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setFixedSize(1280, 720);
    scrollArea->show();

    return a.exec();
}