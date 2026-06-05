QT       += core gui widgets

CONFIG += c++17
CONFIG += resources_big

# 核心：告诉 Qt 去哪里找你写好的头文件
INCLUDEPATH += $$PWD/src \
               $$PWD/src/logic \
               $$PWD/src/entities \
               $$PWD/src/ui

# 目前项目里唯一的源文件（注意路径已经变成了 src/main.cpp）
SOURCES += \
    src/api/EventLauncher.cpp \
    src/api/battlelauncher.cpp \
    src/entities/card.cpp \
    src/entities/enemy.cpp \
    src/entities/fighter.cpp \
    src/entities/player.cpp \
    src/entities/statusmanager.cpp \
    src/logic/battleengine.cpp \
    src/logic/cardfactory.cpp \
    src/logic/cardmanager.cpp \
    src/logic/relicfactory.cpp \
    src/main.cpp \
    src/map/mapmanager.cpp \
    src/map/titlemenuview.cpp \
    src/ui/RelicItem.cpp \
    src/ui/battlescene.cpp \
    src/ui/battleview.cpp \
    src/ui/cardbrowseroverlay.cpp \
    src/ui/carddraftoverlay.cpp \
    src/ui/carditem.cpp \
    src/ui/endturnbutton.cpp \
    src/ui/enemyitem.cpp \
    src/ui/energywidget.cpp \
    src/ui/gamewindow.cpp \
    src/ui/handlayoutmanager.cpp \
    src/ui/pileitem.cpp \
    src/ui/playeritem.cpp \
    src/ui/relictray.cpp \
    src/ui/events/CampfireView.cpp \
    src/ui/events/ChestView.cpp \
    src/ui/events/EventBaseView.cpp \
    src/ui/events/MerchantView.cpp \
    src/ui/events/BigFishView.cpp \
    src/ui/events/ClericView.cpp \
    src/ui/events/DesignerView.cpp \
    src/ui/events/SelfNoteView.cpp \
    src/ui/events/GoldenWingView.cpp \
    src/ui/events/WorldOfGoopView.cpp \
    src/ui/events/SsssserpentView.cpp \
    src/ui/events/GenericChoiceEventView.cpp \
    src/ui/events/EventOptionButton.cpp \
#    src/ui/events/ShopCardItem.cpp \
    src/ui/events/IconButton.cpp \
    src/ui/events/LeaveButton.cpp \
    src/ui/events/RelicPopupWidget.cpp \
    src/ui/events/TextButton.cpp \
    src/ui/rewardscreen.cpp \
    src/ui/shuffleeffectitem.cpp \
    src/ui/statusiconitem.cpp \
    src/ui/topbar.cpp

# 以后你每增加一个文件，Qt Creator 会自动往下面这几项里追加路径
HEADERS += \
    src/api/BattleAPI.h \
    src/api/EventAPI.h \
    src/api/EventLauncher.h \
    src/api/GameEnums.h \
    src/api/battlelauncher.h \
    src/entities/cards/BarricadeCard.h \
    src/entities/cards/BloodlettingCard.h \
    src/entities/cards/BurnCard.h \
    src/entities/cards/BurningPactCard.h \
    src/entities/cards/DarkEmbraceCard.h \
    src/entities/cards/DarkShacklesCard.h \
    src/entities/cards/DazedCard.h \
    src/entities/cards/FireSourceCard.h \
    src/entities/cards/HellFiendCard.h \
    src/entities/cards/InflameCard.h \
    src/entities/cards/MetallicizeCard.h \
    src/entities/cards/PommelStrikeCard.h \
    src/entities/cards/PourCard.h \
    src/entities/cards/PummelCard.h \
    src/entities/cards/ReaperCard.h \
    src/entities/cards/SecondWindCard.h \
    src/entities/cards/ShrugItOffCard.h \
    src/entities/cards/SlimedCard.h \
    src/entities/cards/ThunderclapCard.h \
    src/entities/cards/WoundCard.h \
    src/entities/cards/bashcard.h \
    src/entities/cards/card.h \
    src/entities/cards/defendcard.h \
    src/entities/cards/strikecard.h \
    src/entities/cards/testcard.h \
    src/entities/enemies/Cultist.h \
    src/entities/enemies/FatGremlin.h \
    src/entities/enemies/GremlinLeader.h \
    src/entities/enemies/HexaFlameItem.h \
    src/entities/enemies/Hexaghost.h \
    src/entities/enemies/JawWorm.h \
    src/entities/enemies/MadGremlin.h \
    src/entities/enemies/ShieldGremlin.h \
    src/entities/enemies/SlimeLarge.h \
    src/entities/enemies/SlimeSmall.h \
    src/entities/enemies/SneakyGremlin.h \
    src/entities/enemies/WizardGremlin.h \
    src/entities/enemy.h \
    src/entities/fighter.h \
    src/entities/player.h \
    src/entities/relics/BurningBloodRelic.h \
    src/entities/relics/HappyFlowerRelic.h \
    src/entities/relics/IceCreamRelic.h \
    src/entities/relics/LanternRelic.h \
    src/entities/relics/MeatOnTheBoneRelic.h \
    src/entities/relics/MercuryHourglassRelic.h \
    src/entities/relics/OrnamentalFanRelic.h \
    src/entities/relics/PenNibRelic.h \
    src/entities/relics/PrayerWheelRelic.h \
    src/entities/relics/ShurikenRelic.h \
    src/entities/relics/SmoothStoneRelic.h \
    src/entities/relics/SneckoEyeRelic.h \
    src/entities/relics/ToriiRelic.h \
    src/entities/relics/TungstenRodRelic.h \
    src/entities/relics/VajraRelic.h \
    src/entities/relics/OrichalcumRelic.h \
    src/entities/relics/AnchorRelic.h \
    src/entities/relics/BagOfPreparationRelic.h \
    src/entities/relics/relic.h \
    src/entities/relics/relicmanager.h \
    src/entities/statusmanager.h \
    src/logic/battleengine.h \
    src/logic/cardfactory.h \
    src/logic/cardmanager.h \
    src/logic/enemyfactory.h \
    src/logic/globalsavedata.h \
    src/map/mapmanager.h \
    src/logic/relicfactory.h \
    src/map/titlemenuview.h \
    src/ui/ConfirmButton.h \
    src/ui/RelicItem.h \
    src/ui/battlescene.h \
    src/ui/battleview.h \
    src/ui/cardbrowseroverlay.h \
    src/ui/carddraftoverlay.h \
    src/ui/carditem.h \
    src/ui/endturnbutton.h \
    src/ui/enemyitem.h \
    src/ui/energywidget.h \
    src/ui/gamewindow.h \
    src/ui/handlayoutmanager.h \
    src/ui/pileitem.h \
    src/ui/playeritem.h \
    src/ui/relictray.h \
    src/ui/events/CampfireView.h \
    src/ui/events/ChestView.h \
    src/ui/events/EventBaseView.h \
    src/ui/events/MerchantView.h \
    src/ui/events/BigFishView.h \
    src/ui/events/ClericView.h \
    src/ui/events/DesignerView.h \
    src/ui/events/SelfNoteView.h \
    src/ui/events/GoldenWingView.h \
    src/ui/events/WorldOfGoopView.h \
    src/ui/events/SsssserpentView.h \
    src/ui/events/GenericChoiceEventView.h \
    src/ui/events/EventOptionButton.h \
#    src/ui/events/ShopCardItem.h \
    src/ui/events/IconButton.h \
    src/ui/events/LeaveButton.h \
    src/ui/events/RelicPopupWidget.h \
    src/ui/events/TextButton.h \
    src/ui/rewardscreen.h \
    src/ui/shuffleeffectitem.h \
    src/ui/statusiconitem.h \
    src/ui/topbar.h

FORMS +=

RESOURCES += \
    resources.qrc

DISTFILES += \
    resources/images/attack.png \
    resources/images/attack_debuff.png \
    resources/images/buff.png \
    resources/images/debuff.png \
    resources/images/defend.png \
    resources/images/defend_buff.png