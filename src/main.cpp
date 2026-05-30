#include <QApplication>
#include "ui/GameWindow.h"
#include "logic/GlobalSaveData.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. 初始化全局数据（给主角发牌、发初始血量）
    GlobalSaveData::getInstance()->initNewGame();

    // 2. 启动最高司令部！
    // (它现在内部已经默认会先展示 TitleMenuView 开始界面，并负责后续的所有黑场转场)
    GameWindow window;
    window.show();

    // 3. 进入 Qt 的主事件循环
    return a.exec();
}