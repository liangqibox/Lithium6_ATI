#include "remotecontrolserver.h"

RemoteControlServer::RemoteControlServer(QWidget *parent) :
    QWidget(parent)
{
    tcpServer = new QTcpServer;
    tcpSocket = new QTcpSocket;
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(TCPServer_NewConnection()));
}

RemoteControlServer::~RemoteControlServer()
{
    tcpServer->close();
    delete tcpServer;
}

bool RemoteControlServer::TCPServer_Listen_Start(int port){
    if(tcpServer->listen(QHostAddress::Any,port))return true;
    else return false;
}

void RemoteControlServer::TCPServer_Close(){
    tcpServer->close();
}

void RemoteControlServer::TCPServer_NewConnection(){
    tcpSocket = tcpServer->nextPendingConnection();
    if(tcpServer->isListening()){
        connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(TCP_Incoming_Message()));
    }
}

void RemoteControlServer::TCP_Incoming_Message(){
    QString msg = QString(tcpSocket->readAll());
    emit RemoteCommand(msg);

    LOG.append(QDateTime::currentDateTime().toString("yyyyMMddHHmmss")+":"+msg);
    if(LOG.size()>100)LOG.removeFirst();
}

void RemoteControlServer::Display_Log(){
    /*QString msg;
    for(int i=0; i<LOG.size(); i++)msg.append(LOG.at(i)+"\n");

    Systemlog *a = new Systemlog(0,msg);
    a->show();
    connect(a,SIGNAL(accepted()),a,SLOT(deleteLater()));
    connect(a,SIGNAL(rejected()),a,SLOT(deleteLater()));*/
}
