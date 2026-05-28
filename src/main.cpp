#include <QApplication>
#include "ui/GameWindow.h"
#include "logic/GlobalSaveData.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 初始化全局数据
    GlobalSaveData::getInstance()->initNewGame();

    // 启动最高司令部！
    GameWindow window;
    window.show();

    return a.exec();
}