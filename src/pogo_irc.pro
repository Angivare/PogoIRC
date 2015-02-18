#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T23:00:36
#
#-------------------------------------------------

QT       += core gui network webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pogo_irc
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    docknick.cpp \
    topicview.cpp \
    dbg.cpp \
    jvc.cpp \
    config.cpp \
    modules.cpp \
    forumview.cpp \
    parser.cpp \
    spellchecker.cpp \
    textedit.cpp

HEADERS  += mainwindow.h \
    docknick.h \
    topicview.h \
    dbg.h \
    jvc.h \
    config.h \
    modules.h \
    forumview.h \
    data_structs.h \
    parser.h \
    spellchecker.h \
    textedit.h

FORMS    += mainwindow.ui

RESOURCES += res/resource.qrc

RC_FILE = res/res.rc

LIBS += -L $$_PRO_FILE_PWD_/../lib -llibhunspell
INCLUDEPATH += $$_PRO_FILE_PWD_/../include
