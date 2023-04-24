#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_chatmainwindow.h"
#include <QDebug>
#include "chatclient.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QSqlQuery>
#include <QSqlError>
#include <QtXml>
#include <QTextStream>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chatClient(new ChatClient(this)) // create the chat client
    , m_chatModel(new QStandardItemModel(this)) // create the model to hold the messages
    , m_contactModel(new QStandardItemModel(this)) // create the model to hold the contacts
{
    ui->setupUi(this);
    QString user = "";
    QString password = "";
    QString messageDestination = "";
    //int messageDestinationRow = 0;//Deafult row
    //int usersSigndeUp = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete chat;
}



void MainWindow::on_signinButton_clicked()
{
    QTextEdit *userEntered = ui->userName;
    QTextEdit *passwordEntered = ui->password;

    qDebug() << userEntered->toPlainText();
    qDebug() << passwordEntered->toPlainText();

    user = userEntered->toPlainText();
    password = passwordEntered->toPlainText();

    if(filePath.isNull()){
        qDebug() << QDir::currentPath();
        //New path to the file since it isn't in the build I will redirect it
        QRegularExpression rx("[/ ]");// match a comma or a space
        QStringList list = QDir::currentPath().split(rx, Qt::SkipEmptyParts);

        for(int i(0); i < list.size()-1; i++){
            if(i == 0){
                filePath = list[i] + "/";
            }else{
                filePath = filePath + list[i] + "/";
            }

        }
        filePath = filePath + QCoreApplication::applicationName() + "/";
        qDebug() << "FilePath: " + filePath;
    }

    //Check that user and password match
    QDomDocument usersXML;
    QFile xmlFile(filePath + "xmlLogin.xml");
    if (!xmlFile.open(QIODevice::ReadOnly ))
    {
        // Error while loading file
        xmlFile.close();
    }
    usersXML.setContent(&xmlFile);
    xmlFile.close();

    QDomElement root = usersXML.documentElement();
    QDomElement node = root.firstChild().toElement();

    QString datas = "";
    QString contactInit = "";
    bool sucessful = false;
    while(node.isNull() == false)
    {
        //qDebug() << node.tagName();
        if(node.tagName() == "userInfo"){
            while(!node.isNull()){
                QString userNameXML = node.attribute("userName","userName");
                QString passwordXML = node.attribute("password","password");

                datas.append(userNameXML).append(" - ").append(passwordXML).append("\n");

                if(userNameXML == user && passwordXML == password){
                    sucessful = true;
                    QString contactsList = node.attribute("contacts", "contacts");
                    contactInit = contactsList;
                }
                node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }
    if(!sucessful){
        QMessageBox::critical(this, tr("Error"), "Wrong Username or Password");
        qDebug() << "Inside xml: " + datas;


    }else{
        qDebug() << datas;

        /*
        This isn't needed anymore
        Dialog d;
        d.setModal(true);
        d.exec();
        hide();
        chatDialog = new Dialog(this);
        chatDialog -> show();*/

        chat =new Ui::ChatMainWindow;
        chat->setupUi(this); //Switch the view from login to chat view

        // the model for the messages will have 1 column
        m_chatModel->insertColumn(0);
        m_contactModel->insertColumn(0);

        if(!contactInit.isNull()){
            //Parse to add all contacts to the list that were selected previously
            QRegularExpression rx("[, ]");// match a comma or a space
            QStringList list = contactInit.split(rx, Qt::SkipEmptyParts);

            qDebug() << list;
            for(int i(0); i < list.size(); i++){
                m_contactModel->insertRow(i);

                m_contactModel->setData(m_contactModel->index(i, 0), list.at(i));

                m_contactModel->setData(m_contactModel->index(i, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

            }

        }
        // set the model as the data source vor the list view
        chat->chatView->setModel(m_chatModel);
        chat->contactsView->setModel(m_contactModel);

        // connect the signals from the chat client to the slots in this ui
        connect(m_chatClient, &ChatClient::connected, this, &MainWindow::connectedToServer);
        connect(m_chatClient, &ChatClient::loggedIn, this, &MainWindow::loggedIn);
        connect(m_chatClient, &ChatClient::loginError, this, &MainWindow::loginFailed);
        connect(m_chatClient, &ChatClient::messageReceived, this, &MainWindow::messageReceived);
        connect(m_chatClient, &ChatClient::disconnected, this, &MainWindow::disconnectedFromServer);
        connect(m_chatClient, &ChatClient::error, this, &MainWindow::error);
        connect(m_chatClient, &ChatClient::userJoined, this, &MainWindow::userJoined);
        connect(m_chatClient, &ChatClient::userLeft, this, &MainWindow::userLeft);


        // connect the connect button to a slot that will attempt the connection
        connect(chat->connectButton, &QPushButton::clicked, this, &MainWindow::attemptConnection);
        // connect the click of the "send" button and the press of the enter while typing to the slot that sends the message
        connect(chat->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
        connect(chat->messageEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    }


}




void MainWindow::attemptConnection()
{
    // We ask the user for the address of the server, we use 127.0.0.1 (aka localhost) as default
    const QString hostAddress = QInputDialog::getText(
        this
        , tr("Chose Server")
        , tr("Server Address")
        , QLineEdit::Normal
        , QStringLiteral("127.0.0.1")
        );
    if (hostAddress.isEmpty())
        return; // the user pressed cancel or typed nothing
    // disable the connect button to prevent the user clicking it again
    chat->connectButton->setEnabled(false);
    // tell the client to connect to the host using the port 1967
    m_chatClient->connectToServer(QHostAddress(hostAddress), 1967);
}

void MainWindow::connectedToServer()
{
    //Changed login
    attemptLogin(user);}


void MainWindow::attemptLogin(const QString &userName)
{
    // use the client to attempt a log in with the given username
    m_chatClient->login(userName);
}

void MainWindow::loggedIn()
{
    // once we successully log in we enable the ui to display and send messages
    chat->sendButton->setEnabled(true);
    chat->messageEdit->setEnabled(true);
    chat->chatView->setEnabled(true);
    // clear the user name record
    m_lastUserName.clear();
}

void MainWindow::loginFailed(const QString &reason)
{
    // the server rejected the login attempt
    // display the reason for the rejection in a message box
    QMessageBox::critical(this, tr("Error"), reason);
    // allow the user to retry, execute the same slot as when just connected
    connectedToServer();
}

void MainWindow::messageReceived(const QString &sender, const QString &text)
{
    // store the index of the new row to append to the model containing the messages
    int newRow = m_chatModel->rowCount();
    // we display a line containing the username only if it's different from the last username we displayed
    if (m_lastUserName != sender) {
        // store the last displayed username
        m_lastUserName = sender;
        // create a bold default font
        QFont boldFont;
        boldFont.setBold(true);
        // insert 2 row, one for the message and one for the username
        m_chatModel->insertRows(newRow, 2);
        // store the username in the model
        m_chatModel->setData(m_chatModel->index(newRow, 0), sender + QLatin1Char(':'));
        // set the alignment for the username
        m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        // set the for the username
        m_chatModel->setData(m_chatModel->index(newRow, 0), boldFont, Qt::FontRole);
        ++newRow;
    } else {
        // insert a row for the message
        m_chatModel->insertRow(newRow);
    }

    //Only add text to view if that contact is currently opened
    if(messageDestination == sender){
        // store the message in the model
        m_chatModel->setData(m_chatModel->index(newRow, 0), text);
        // set the alignment for the message
        m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        // scroll the view to display the new message
        chat->chatView->scrollToBottom();
    }else{
        qDebug() << "Chat view isn't for the opened chat, will load later";
    }

}

void MainWindow::sendMessage()
{
    // we use the client to send the message that the user typed

    m_chatClient->sendMessage(chat->messageEdit->text(), messageDestination, user);
    //Add message to users col
    /*const int row = m_messagesSave->rowCount();
    m_messagesSave->insertRow(row);
    m_messagesSave->setData(m_messagesSave->index(row, messageDestinationRow*2), chat->messageEdit->text());
    m_messagesSave->setData(m_messagesSave->index(row, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
*/


    const int newRow = m_chatModel->rowCount();
    m_chatModel->insertRow(newRow);
    m_chatModel->setData(m_chatModel->index(newRow, 0), chat->messageEdit->text());
    m_chatModel->setData(m_chatModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

    if(messageDestination.isNull()){
        QMessageBox::information(this, "ERROR", "Select a contact before you send the message!!");
    }else{
        QJsonDocument doc;
        QJsonObject saveMsg;
        saveMsg.insert("text", chat->messageEdit->text());
        saveMsg.insert("destination", messageDestination);
        saveMsg.insert("source", user);
        QTime time = QTime::currentTime();
        QDate date = QDate::currentDate();
        QString dateTime = time.toString() + " " + date.toString();
        saveMsg.insert("TimeStamp", dateTime);
        QJsonObject content;
        content.insert("SENT", saveMsg);
        QJsonObject overall;
        overall.insert("Messages", content);
        doc.setObject(overall );


        QFile file(filePath + "clientJson.xml");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //Failed to open
            file.close();

        }
        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();

        QJsonObject RootObject = document.object();
        QJsonArray messageArray = RootObject[messageDestination].toArray();
        messageArray.append(content);
        RootObject[messageDestination] = messageArray;
        //RootObject["Messages"] = messageArray;
        document.setObject(RootObject);
        QByteArray bytes = document.toJson( QJsonDocument::Indented );

        if( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            QTextStream iStream( &file );
            iStream << bytes;
            file.close();
        }
        else
        {
            qDebug() << "File open failed: ";
        }


        // clear the content of the message editor
        chat->messageEdit->clear();
        // scroll the view to display the new message
        chat->chatView->scrollToBottom();
        // reset the last printed username
        m_lastUserName.clear();
    }


}

void MainWindow::disconnectedFromServer()
{
    // if the client loses connection to the server
    // comunicate the event to the user via a message box
    QMessageBox::warning(this, tr("Disconnected"), tr("The host terminated the connection"));
    // disable the ui to send and display messages
    chat->sendButton->setEnabled(false);
    chat->messageEdit->setEnabled(false);
    chat->chatView->setEnabled(false);
    // enable the button to connect to the server again
    chat->connectButton->setEnabled(true);
    // reset the last printed username
    m_lastUserName.clear();
}

void MainWindow::userJoined(const QString &username)
{
    // store the index of the new row to append to the model containing the messages
    const int newRow = m_chatModel->rowCount();
    // insert a row
    m_chatModel->insertRow(newRow);
    // store in the model the message to comunicate a user joined
    m_chatModel->setData(m_chatModel->index(newRow, 0), tr("%1 Joined the Chat").arg(username));
    // set the alignment for the text
    m_chatModel->setData(m_chatModel->index(newRow, 0), Qt::AlignCenter, Qt::TextAlignmentRole);
    // set the color for the text
    m_chatModel->setData(m_chatModel->index(newRow, 0), QBrush(Qt::blue), Qt::ForegroundRole);
    // scroll the view to display the new message
    chat->chatView->scrollToBottom();
    // reset the last printed username
    m_lastUserName.clear();
}
void MainWindow::userLeft(const QString &username)
{
    // store the index of the new row to append to the model containing the messages
    const int newRow = m_chatModel->rowCount();
    // insert a row
    m_chatModel->insertRow(newRow);
    // store in the model the message to comunicate a user left
    m_chatModel->setData(m_chatModel->index(newRow, 0), tr("%1 Left the Chat").arg(username));
    // set the alignment for the text
    m_chatModel->setData(m_chatModel->index(newRow, 0), Qt::AlignCenter, Qt::TextAlignmentRole);
    // set the color for the text
    m_chatModel->setData(m_chatModel->index(newRow, 0), QBrush(Qt::red), Qt::ForegroundRole);
    // scroll the view to display the new message
    chat->chatView->scrollToBottom();
    // reset the last printed username
    m_lastUserName.clear();
}

void MainWindow::error(QAbstractSocket::SocketError socketError)
{
    // show a message to the user that informs of what kind of error occurred
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::ProxyConnectionClosedError:
        return; // handled by disconnectedFromServer
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The host refused the connection"));
        break;
    case QAbstractSocket::ProxyConnectionRefusedError:
        QMessageBox::critical(this, tr("Error"), tr("The proxy refused the connection"));
        break;
    case QAbstractSocket::ProxyNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the proxy"));
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::critical(this, tr("Error"), tr("Could not find the server"));
        break;
    case QAbstractSocket::SocketAccessError:
        QMessageBox::critical(this, tr("Error"), tr("You don't have permissions to execute this operation"));
        break;
    case QAbstractSocket::SocketResourceError:
        QMessageBox::critical(this, tr("Error"), tr("Too many connections opened"));
        break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::warning(this, tr("Error"), tr("Operation timed out"));
        return;
    case QAbstractSocket::ProxyConnectionTimeoutError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy timed out"));
        break;
    case QAbstractSocket::NetworkError:
        QMessageBox::critical(this, tr("Error"), tr("Unable to reach the network"));
        break;
    case QAbstractSocket::UnknownSocketError:
        QMessageBox::critical(this, tr("Error"), tr("An unknown error occured"));
        break;
    case QAbstractSocket::UnsupportedSocketOperationError:
        QMessageBox::critical(this, tr("Error"), tr("Operation not supported"));
        break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        QMessageBox::critical(this, tr("Error"), tr("Your proxy requires authentication"));
        break;
    case QAbstractSocket::ProxyProtocolError:
        QMessageBox::critical(this, tr("Error"), tr("Proxy comunication failed"));
        break;
    case QAbstractSocket::TemporaryError:
    case QAbstractSocket::OperationError:
        QMessageBox::warning(this, tr("Error"), tr("Operation failed, please try again"));
        return;
    default:
        Q_UNREACHABLE();
    }
    // enable the button to connect to the server again
    chat->connectButton->setEnabled(true);
    // disable the ui to send and display messages
    chat->sendButton->setEnabled(false);
    chat->messageEdit->setEnabled(false);
    chat->chatView->setEnabled(false);
    // reset the last printed username
    m_lastUserName.clear();
}


void MainWindow::on_signupButton_clicked()
{
    qDebug() <<"signup";
    QTextEdit *userEntered = ui->userName;
    QTextEdit *passwordEntered = ui->password;


    if(filePath.isNull()){
        qDebug() << QDir::currentPath();
        //New path to the file since it isn't in the build I will redirect it
        QRegularExpression rx("[/ ]");// match a comma or a space
        QStringList list = QDir::currentPath().split(rx, Qt::SkipEmptyParts);

        for(int i(0); i < list.size()-1; i++){
            if(i == 0){
                filePath = list[i] + "/";
            }else{
                filePath = filePath + list[i] + "/";
            }

        }
        filePath = filePath + QCoreApplication::applicationName() + "/";
        qDebug() << "FilePath: " + filePath;
    }

    QFile xmlFile(filePath + "xmlLogin.xml");
    if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }


    QDomDocument usersXML;
    usersXML.setContent(&xmlFile); //Keeps the original file the same
    QDomElement root = usersXML.documentElement();
    xmlFile.close();


    if(root.isNull()){
        qDebug() << "Empty file";
        usersXML.setContent("<UserList>");
        QDomDocument doc;
        doc.setContent("\<UserList/>");
        usersXML.appendChild(usersXML.firstChild());
        usersXML.appendChild(doc.firstChild());

        if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
        {
            qDebug() << "Already opened or there is another issue";
            xmlFile.close();
        }
        QTextStream xmlContent(&xmlFile);
        xmlContent << usersXML.toString();
        xmlFile.close();
        qDebug()<<"finished.";
    }

    //have to reopen file incase it was empty
    if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }

    usersXML.setContent(&xmlFile); //Keeps the original file the same
    root = usersXML.documentElement();
    xmlFile.close();
    QDomElement student = usersXML.createElement("userInfo");
    student.setAttribute("userName", userEntered->toPlainText());
    student.setAttribute("password", passwordEntered->toPlainText());
    student.setAttribute("contacts", "");

    root.appendChild(student);

    if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }
    QTextStream xmlContent(&xmlFile);
    xmlContent << usersXML.toString();
    xmlFile.close();
    qDebug()<<"finished.";
    usersSignedUp++;

}

void MainWindow::on_logOutButton_clicked()
{
    m_contactModel->clear();
    m_chatModel->clear();
    ui->setupUi(this);
}


void MainWindow::on_addContactButton_clicked()
{
    QString newContact = QInputDialog::getText(this, tr("Enter Contact Name"), tr("Text"));
    qDebug() << "New contact: " + newContact;

    const int newRow = m_contactModel->rowCount();
    qDebug() << newRow;
    m_contactModel->insertRow(newRow);

    m_contactModel->setData(m_contactModel->index(newRow, 0), newContact);

    m_contactModel->setData(m_contactModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);

    QDomDocument usersXML;
    //have to reopen file incase it was empty
    QFile xmlFile(filePath + "xmlLogin.xml");
    if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }

    usersXML.setContent(&xmlFile); //Keeps the original file the same
    xmlFile.close();

    QDomElement root = usersXML.documentElement();
    QDomElement node = root.firstChild().toElement();
    while(node.isNull() == false)
    {
        //qDebug() << node.tagName();
        if(node.tagName() == "userInfo"){
            while(!node.isNull()){

                QString userNameXML = node.attribute("userName","userName");
                QString contactsXML = node.attribute("contacts","contacts");
                QString newContactString = "";
                if(userNameXML == user){
                    if(contactsXML.isNull()){
                        newContactString = newContact;
                    }else{
                        newContactString = contactsXML + "," + newContact;
                    }

                    node.setAttribute("contacts", newContactString);
                }
                 node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }

    usersXML.appendChild(node);

    if (!xmlFile.open(QFile::ReadWrite | QFile::Text ))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }
    QTextStream xmlContent(&xmlFile);
    xmlContent << usersXML.toString();
    xmlFile.close();
    qDebug()<<"finished.";

    chat->contactsView->scrollToBottom();

}


void MainWindow::on_contactsView_activated(const QModelIndex &index)
{
    qDebug() << "Contact Activated";
    qDebug() << index.column();
}

void MainWindow::on_pushButton_clicked()
{
}


void MainWindow::on_contactsView_pressed(const QModelIndex &index)
{
    //qDebug() << "Contact Pressed";
    QString itemText = index.data(Qt::DisplayRole).toString();

    if(itemText == messageDestination){
        //qDebug() << "Nothing changed";
    }else{
        qDebug() << "Contact pressed and changed: " + itemText;
        messageDestination = itemText;
        messageDestinationRow = index.row();


        QFile file(filePath + "clientJson.xml");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //Failed to open
            file.close();

        }
        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();

        QJsonObject RootObject = document.object();
        QJsonArray messageArray = RootObject[messageDestination].toArray();
        qDebug() << messageArray.isEmpty();//IF EMPTY WE NEED TO RESET CHAT BEFORE OVERWRITING SO GET ROWS FROM CHAT VIEW
        chat->chatView->reset();
        m_chatModel->clear();
        // the model for the messages will have 1 column
        m_chatModel->insertColumn(0);
        // set the model as the data source vor the list view
        chat->chatView->setModel(m_chatModel);

        for(int i(0); i < messageArray.size(); i++){

            QJsonObject msgSent = messageArray[i].toObject();
            QJsonObject sent = msgSent.value(QString("SENT")).toObject();
            QJsonValue value = sent.value(QString("text"));
            qDebug() << value.toString();

            if(value.toString().isNull()){
                qDebug() << "Receive Instead: ";
                QJsonObject sent = msgSent.value(QString("RECV")).toObject();
                QJsonValue value = sent.value(QString("text"));
                QJsonValue destination = sent.value(QString("destination"));
                if(destination == user){
                    m_chatModel->insertRow(i);

                    // store the message in the model
                    m_chatModel->setData(m_chatModel->index(i, 0), value);
                    m_chatModel->setData(m_chatModel->index(i, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
                }

            }else{
                QJsonValue source = sent.value(QString("source"));
                if(source == user){
                    m_chatModel->insertRow(i);
                    m_chatModel->setData(m_chatModel->index(i, 0), value);
                    m_chatModel->setData(m_chatModel->index(i, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
                }
            }

        }

        // scroll the view to display the new message
        chat->chatView->scrollToBottom();


    }

}

