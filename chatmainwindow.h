
#ifndef CHATMAINWINDOW_H
#define CHATMAINWINDOW_H

#include <QMainWindow>
#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class ChatMainWindow; }
QT_END_NAMESPACE

class ChatMainWindow : public QMainWindow

{
    Q_OBJECT

public:
    ChatMainWindow(QWidget *parent = nullptr);
    ~ChatMainWindow();

private:
    Ui::ChatMainWindow *chat;

};

#endif // CHATMAINWINDOW_H
