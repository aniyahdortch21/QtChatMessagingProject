
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
//#include <dialog.h>
#include <chatclient.h>
#include <chatmainwindow.h>
#include <QWidget>
#include <QAbstractSocket>
class ChatClient;
class QStandardItemModel;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_signinButton_clicked();

private:
    Ui::MainWindow *ui;
    //Dialog *chatDialog;
    ChatClient *m_chatClient;
    QStandardItemModel *m_chatModel;
    QStandardItemModel *m_contactModel;
    QStandardItemModel *m_messagesSave;
    QString m_lastUserName;
    Ui::ChatMainWindow *chat;
    QString user;
    QString password;
    QString messageDestination;
    QString filePath;
    int messageDestinationRow;
    int usersSignedUp   = 0;
    int firstMessage = 1;
    bool userConnectedToServer = false;

private slots:
    void attemptConnection();
    void connectedToServer();
    void attemptLogin(const QString &userName);
    void loggedIn();
    void loginFailed(const QString &reason);
    void messageReceived(const QString &sender, const QString &text);
    void sendMessage();
    void disconnectedFromServer();
    void userJoined(const QString &username);
    void userLeft(const QString &username);
    void error(QAbstractSocket::SocketError socketError);
    void on_signupButton_clicked();
    void on_pushButton_clicked();
    void on_logOutButton_clicked();
    void on_addContactButton_clicked();
    void on_contactsView_activated(const QModelIndex &index);
    void on_contactsView_pressed(const QModelIndex &index);
};

#endif // MAINWINDOW_H
