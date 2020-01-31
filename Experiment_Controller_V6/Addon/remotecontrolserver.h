#ifndef REMOTECONTROLSERVER_H
#define REMOTECONTROLSERVER_H

#include <QWidget>
#include <QDateTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include "Addon/systemlog.h"

class RemoteControlServer : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteControlServer(QWidget *parent = 0);
    ~RemoteControlServer();

    bool TCPServer_Listen_Start(int port);
    void TCPServer_Close();

signals:
    void RemoteCommand(QString);

public slots:
    void Display_Log();

private slots:
    void TCPServer_NewConnection();
    void TCP_Incoming_Message();

private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;

    QList<QString> LOG;

};

#endif // REMOTECONTROLSERVER_H
