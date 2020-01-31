#include "external.h"
#include "ui_external.h"
#include <QDebug>

External::External(QWidget *parent, QList<Variable*> *vari, QList<QStringList> *files, int *tal, int cycle, int scan) :
    QWidget(parent),
    ui(new Ui::External)
{
    ui->setupUi(this);

    ui->File_list->setColumnWidth(0,62);
    ui->File_list->setColumnWidth(1,120);
    ui->File_list->setColumnWidth(2,120);
    ui->File_list->setColumnWidth(3,60);

    variables = vari;
    External_files = files;
    total = tal;
    cycle_time = 0;
    number_of_scan = scan;
    number_of_run = cycle;

    tcpSocket = new QTcpSocket(this);
    udpSocket = new QUdpSocket(this);
    tcpSocket->bind(65320);
    udpSocket->bind(65321);

    for(int i=0; i<External_files->size(); i++){
        //QTableWidgetItem *active = new QTableWidgetItem(External_files->at(i).at(0));
        QStringList protocol;
        protocol << "Disable" << "TCP" << "UDP" << "Save";
        QComboBox *active = new QComboBox(ui->File_list);
        active->addItems(protocol);
        active->setCurrentIndex(External_files->at(i).at(0).toInt());
        QTableWidgetItem *ip = new QTableWidgetItem(External_files->at(i).at(1));
        QTableWidgetItem *com = new QTableWidgetItem(External_files->at(i).at(2));
        QTableWidgetItem *item = new QTableWidgetItem(External_files->at(i).at(3));
        ui->File_list->setRowCount(ui->File_list->rowCount()+1);
        //ui->File_list->setItem(ui->File_list->rowCount()-1,0,active);
        ui->File_list->setCellWidget(ui->File_list->rowCount()-1,0,active);
        ui->File_list->setItem(ui->File_list->rowCount()-1,1,ip);
        ui->File_list->setItem(ui->File_list->rowCount()-1,2,com);
        ui->File_list->setItem(ui->File_list->rowCount()-1,3,item);
    }
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
    setAcceptDrops(true);
}

External::~External()
{
    delete ui;
}

void External::dragEnterEvent(QDragEnterEvent *event){
    if (event -> mimeData()->hasUrls()){
        event -> acceptProposedAction();
    }
}

void External::dropEvent(QDropEvent *event){
    QStringList files;
    foreach (QUrl url, event->mimeData()->urls()) {
        files.append(url.path().remove(0,1));
    }
    add_files(files);
}

void External::Reset(QList<Variable*> *vari){
    variables = vari;
    disconnect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
    ui->File_list->clear();
    ui->File_list->setRowCount(0);
    for(int i=0; i<External_files->size(); i++){
        //QTableWidgetItem *active = new QTableWidgetItem(External_files->at(i).at(0));
        QStringList protocol;
        protocol << "Disable" << "TCP" << "UDP" << "Save";
        QComboBox *active = new QComboBox(ui->File_list);
        active->addItems(protocol);
        active->setCurrentIndex(External_files->at(i).at(0).toInt());
        QTableWidgetItem *ip = new QTableWidgetItem(External_files->at(i).at(1));
        QTableWidgetItem *com = new QTableWidgetItem(External_files->at(i).at(2));
        QTableWidgetItem *item = new QTableWidgetItem(External_files->at(i).at(3));
        ui->File_list->setRowCount(ui->File_list->rowCount()+1);
        //ui->File_list->setItem(ui->File_list->rowCount()-1,0,active);
        ui->File_list->setCellWidget(ui->File_list->rowCount()-1,0,active);
        ui->File_list->setItem(ui->File_list->rowCount()-1,1,ip);
        ui->File_list->setItem(ui->File_list->rowCount()-1,2,com);
        ui->File_list->setItem(ui->File_list->rowCount()-1,3,item);
        connect(active,SIGNAL(currentIndexChanged(int)),this,SLOT(File_save()));
    }
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
}

void External::File_save(){
    External_files->clear();
    for(int i=0; i<ui->File_list->rowCount(); i++){
        QComboBox *protocol = static_cast<QComboBox*>(ui->File_list->cellWidget(i,0));
        QStringList temp;
        temp.append(QString::number(protocol->currentIndex()));
        for(int j=1; j<4; j++)temp.append(ui->File_list->item(i,j)->text());
        External_files->append(temp);
    }
}

void External::on_File_load_clicked()
{
    QStringList filename = QFileDialog::getOpenFileNames(this,"Import Files",0,tr("Setting File (*.txt)"));
    add_files(filename);
}

void External::add_files(QStringList filename){
    ui->File_list->disconnect();
    if(!filename.isEmpty()){
        for(int i=0; i<filename.size(); i++){
            //QTableWidgetItem *active = new QTableWidgetItem("TCP");
            QStringList protocol;
            protocol << "Disable" << "TCP" << "UDP" << "Save";
            QComboBox *active = new QComboBox(ui->File_list);
            active->addItems(protocol);
            QTableWidgetItem *ip = new QTableWidgetItem(QString("192.168.2.0:8000"));
            QTableWidgetItem *item = new QTableWidgetItem(filename.at(i));
            QString name = filename.at(i);
            name.chop(4);
            int j;
            for(j=name.size()-1;j>-1;j--)if(name.at(j)=='/')break;
            name.remove(0,j+1);
            QTableWidgetItem *com = new QTableWidgetItem(name);

            QStringList temp;
            temp.append("UDP");
            temp.append("192.168.2.0:8000");
            temp.append(filename.at(i));
            temp.append(name);
            External_files->append(temp);
            ui->File_list->setRowCount(ui->File_list->rowCount()+1);
            //ui->File_list->setItem(ui->File_list->rowCount()-1,0,active);
            ui->File_list->setCellWidget(ui->File_list->rowCount()-1,0,active);
            ui->File_list->setItem(ui->File_list->rowCount()-1,1,ip);
            ui->File_list->setItem(ui->File_list->rowCount()-1,2,com);
            ui->File_list->setItem(ui->File_list->rowCount()-1,3,item);
        }
    }
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
}

void External::on_File_remove_clicked()
{
    disconnect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
    QList<QTableWidgetItem*> removing = ui->File_list->selectedItems();
    for(int i=0; i<removing.size(); i++){
        ui->File_list->removeRow(ui->File_list->row(removing.at(i)));
    }
    File_save();
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
}

void External::on_Move_up_clicked()
{
    ui->File_list->disconnect();
    QList<QTableWidgetItem*> items = ui->File_list->selectedItems();
    if(items.isEmpty())return;
    else{
        if(items.first()->row()==0)return;
        else{
            int currentRow = items.first()->row();
            for(int i=1; i<4; i++)ui->File_list->takeItem(currentRow,i);
            for(int i=1; i<4; i++)ui->File_list->setItem(currentRow,i,ui->File_list->takeItem(currentRow-1,i));
            for(int i=0; i<items.size(); i++)ui->File_list->setItem(currentRow-1,i+1,items.at(i));
            ui->File_list->selectRow(currentRow-1);
        }
    }
    File_save();
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
}

void External::on_Move_down_clicked()
{
    ui->File_list->disconnect();
    QList<QTableWidgetItem*> items = ui->File_list->selectedItems();
    if(items.isEmpty())return;
    else{
        if(items.first()->row()==(ui->File_list->rowCount()-1))return;
        else{
            int currentRow = items.first()->row();
            for(int i=1; i<4; i++)ui->File_list->takeItem(currentRow,i);
            for(int i=1; i<4; i++)ui->File_list->setItem(currentRow,i,ui->File_list->takeItem(currentRow+1,i));
            for(int i=0; i<items.size(); i++)ui->File_list->setItem(currentRow+1,i+1,items.at(i));
            ui->File_list->selectRow(currentRow+1);
        }
    }
    File_save();
    connect(ui->File_list,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(File_save()));
}

void External::on_Test_clicked(){
    int connect = 0;
    for(int i=0; i<ui->File_list->rowCount(); i++){
        QComboBox *protocol = static_cast<QComboBox*>(ui->File_list->cellWidget(i,0));
        if(protocol->currentIndex()!=0){
            char msg[7] = {'H','e','l','l','o','\r','\n'};
            if(protocol->currentIndex()==1){
                tcpSocket->connectToHost(Get_IP(i),Get_Port(i));
                if(tcpSocket->waitForConnected(300)){
                    connect++;
                    tcpSocket->write(msg,sizeof(msg));
                    tcpSocket->waitForBytesWritten();
                }
                else{
                    ui->state->append("File "+QString::number(i+1)+" fail to connect");
                }
                tcpSocket->close();
            }
            else if(protocol->currentIndex()==2){
                connect++;
                udpSocket->writeDatagram(msg,QHostAddress(Get_IP(i)),Get_Port(i));
                udpSocket->waitForBytesWritten();
            }
            else if(protocol->currentIndex()==3){
                connect++;
                QString adress_name = ui->File_list->item(i,1)->text();
                QFile destination(adress_name);
                if (!destination.open(QIODevice::WriteOnly | QIODevice::Text)){
                    ui->state->append("File "+QString::number(i+1)+" can not write to.");
                }
                QTextStream out(&destination);
                out << "Hello";
                destination.close();
            }
        }
    }
    ui->state->append(QString::number(connect) + " files connected");
    ui->state->append("*************************************");
}

void External::reload(QString msg){
    QStringList msglist = msg.split(':');
    cycle_time = msglist.at(0).toInt();
    number_of_run = msglist.at(1).toInt();
    number_of_scan = msglist.at(2).toInt();
    int error = Send_file();
    emit External_Loaded(error);
}

int External::Send_file(){
    int error = 0;
    for(int i=0; i<ui->File_list->rowCount(); i++){
        QComboBox *protocol = static_cast<QComboBox*>(ui->File_list->cellWidget(i,0));
        if(protocol->currentIndex()!=0){
            if(protocol->currentIndex()==1){
                tcpSocket->connectToHost(Get_IP(i),Get_Port(i));
                if(tcpSocket->waitForConnected(300)){
                    QString filename = ui->File_list->item(i,3)->text();
                    QFile file(filename);
                    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))error++;
                    else{
                        QTextStream in(&file);
                        while(!in.atEnd()){
                            QString read = in.readLine();
                            QString m = translate_command(read);
                            tcpSocket->write(m.toLatin1(),m.size());
                            tcpSocket->waitForBytesWritten();
                        }
                    }
                    file.close();
                }
                else error++;
                tcpSocket->close();
            }
            else if(protocol->currentIndex()==2){
                QString filename = ui->File_list->item(i,3)->text();
                QFile file(filename);
                if(!file.open(QIODevice::ReadOnly|QIODevice::Text))error++;
                else{
                    QTextStream in(&file);
                    while(!in.atEnd()){
                        QString m = translate_command(in.readLine());
                        udpSocket->writeDatagram(m.toLatin1(),QHostAddress(Get_IP(i)),Get_Port(i));
                        udpSocket->waitForBytesWritten();
                    }
                }
                file.close();
            }
            else if(protocol->currentIndex()==3){
                QString filename = ui->File_list->item(i,3)->text();
                QFile file(filename);
                QString adress_name = ui->File_list->item(i,1)->text();
                QFile destination(adress_name);
                if(!file.open(QIODevice::ReadOnly|QIODevice::Text))error++;
                else{
                    if(!destination.open(QIODevice::WriteOnly | QIODevice::Text)){
                        error++;
                    }
                    else{
                        QTextStream in(&file);
                        QTextStream out(&destination);
                        while(!in.atEnd()){
                            QString m = translate_command(in.readLine());
                            out << m << endl;
                        }
                    }
                }
                file.close();
                destination.close();
            }
        }
    }
    return error;
}

QString External::Get_IP(int file_no){
    QString adr = ui->File_list->item(file_no,1)->text();
    if(adr.contains(':')){
        adr.remove(adr.indexOf(':'),7);
    }
    return adr;
}

int External::Get_Port(int file_no){
    QString adr = ui->File_list->item(file_no,1)->text();
    if(adr.contains(':')){
        bool ok = false;
        int port = adr.remove(0,adr.indexOf(':')+1).toInt(&ok);
        if(ok) return port;
    }
    return 0;
}

QString External::translate_command(QString str){
    if(str.contains('%')){
        for(int i=0; i<str.size(); i++){
            if(str.at(i)=='%'){
                str.chop(str.size()-i);
                break;
            }
        }
    }
    if(str.contains("[cycle_time]"))str.replace("[cycle_time]",QString::number(cycle_time));
    if(str.contains("[number_of_scan]"))str.replace("[number_of_scan]",QString::number(number_of_scan));
    if(str.contains("[number_of_run]"))str.replace("[number_of_run]",QString::number(number_of_run));
    if(str.contains("[datetime]",Qt::CaseInsensitive))str.replace("[datetime]",QDateTime::currentDateTime().toString("yyyyMMddHHmmss"),Qt::CaseInsensitive);
    for(int i=0; i<variables->size(); i++){
        QString name = "["+variables->at(i)->get_name()+"]";
        if(str.contains(name)){
        switch(variables->at(i)->get_type()){
        case 0:
            str.replace(name,QString::number(variables->at(i)->get_value()[0]));
            break;
        case 1:
            str.replace(name,QString::number(variables->at(i)->get_scan(number_of_scan)));
            break;
        case 2:
            str.replace(name,QString::number(variables->at(i)->get_calculated(number_of_scan,0,0,0,0)));
            break;
        default:
            break;
        }
        }
    }
    return str;
}

void External::on_File_send_clicked()
{
    Send_file();
}

void External::on_File_list_cellDoubleClicked(int row, int column)
{
    if(column==1){
        QComboBox *protocol = static_cast<QComboBox*>(ui->File_list->cellWidget(row,0));
        if(protocol->currentIndex()==3){
            QString filename =  QFileDialog::getSaveFileName(this, tr("Save File"),0,tr("*.txt"));
            ui->File_list->item(row,column)->setText(filename);
        }
    }
}
