#include "chatmainwindow.h"
#include "ui_chatmainwindow.h"

ChatMainWindow::ChatMainWindow(QWidget *parent) :
    QMainWindow(parent),
    chat(new Ui::ChatMainWindow)
{
    chat->setupUi(this);
}

ChatMainWindow::~ChatMainWindow()
{
    delete chat;
}



