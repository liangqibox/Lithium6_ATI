#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent, QStringList setting, RemoteControlServer *rcs) :
    QWidget(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    RCS = rcs;

    ui->File_list->setColumnWidth(0,60);
    ui->File_list->setColumnWidth(1,85);
    ui->File_list->setColumnWidth(2,150);
    ui->File_list->setColumnWidth(3,35);
    ui->File_list->setColumnWidth(4,35);
    program_setting = setting;

    ui->Cpu_type->setCurrentIndex(program_setting.at(0).toInt());
    ui->Analog_channel->setCurrentIndex(program_setting.at(1).toInt());
    ui->Digital_channel->setCurrentIndex(program_setting.at(2).toInt());
    if(program_setting.at(3)=="online")ui->Offline_mode->setChecked(false);
    else ui->Offline_mode->setChecked(true);
    if(program_setting.at(4)=="preon")ui->Precalculation->setChecked(true);
    if(program_setting.at(5)=="tcpon"){
        ui->TCP_Port->setText(program_setting.at(6));
        ui->TCP_switch->setChecked(true);
        on_TCP_switch_clicked();
    }
    if(program_setting.at(7)=="StopAfterScan")ui->StopafterScan->setChecked(true);

    Mission = false;
    Mission_used_new_profile = false;
    run_count = 0;

    setAcceptDrops(true);
    load_setting_list();
}

Setting::~Setting()
{
    write_setting_list();
    emit Closed();
    delete ui;
}

void Setting::load_setting_list(){
    QList<QStringList> list;
    QDir SettingDir("Cycle_setting\\");
    if(SettingDir.exists()){
        SettingDir.setSorting(QDir::Time);
        SettingDir.setFilter(QDir::Files);
        QStringList filelist = SettingDir.entryList(QDir::Files,QDir::Time);
        SettingDir.makeAbsolute();
        for(int i=0; i<filelist.size(); i++){
            QStringList file;
            file << SettingDir.filePath(filelist.at(i)) << filelist.at(i) << " " << "0" << "0";
            list.append(file);
        }
    }

    /*QFile file(Default_Setting_List);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return;
    QTextStream in(&file);
    while(!in.atEnd()){
        QString temp = in.readLine();
        if(!temp.isEmpty())list.append(temp.split('!'));
    }*/
    for(int i=0; i<list.size(); i++){
        ui->File_list->setRowCount(ui->File_list->rowCount()+1);
        for(int j=0; j<5; j++){
            QTableWidgetItem *item = new QTableWidgetItem(list.at(i).at(j));
            ui->File_list->setItem(ui->File_list->rowCount()-1,j,item);
        }
    }
    //file.close();
}

void Setting::write_setting_list(){
    /*QFile file(Default_Setting_List);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    for(int i=0; i<ui->File_list->rowCount(); i++){
        QString temp;
        for(int j=0; j<5; j++){
            if(!ui->File_list->item(i,j)->text().isEmpty())temp += ui->File_list->item(i,j)->text();
            else temp += "   ";
            temp += '!';
        }
        temp.chop(1);
        out << temp << endl;
    }
    file.close();*/
}

void Setting::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasUrls()){
        event->acceptProposedAction();
    }
}

void Setting::dropEvent(QDropEvent *event){
    QStringList files;
    foreach (QUrl url, event->mimeData()->urls()) {
        files.append(url.path().remove(0,1));
    }
    add_files(files);
}

void Setting::Setting_refresh(QStringList set){
    ui->Cpu_type->setCurrentIndex(set.at(0).toInt());
    ui->Analog_channel->setCurrentIndex(set.at(1).toInt());
    ui->Digital_channel->setCurrentIndex(set.at(2).toInt());
}

void Setting::on_File_import_clicked()
{
    QStringList filename = QFileDialog::getOpenFileNames(this,"Import Setting Files",0,tr("Setting File (*.adcs)"));
    if(!filename.isEmpty()){
        add_files(filename);
    }
}

void Setting::add_files(QStringList filename){
    for(int i=0; i<filename.size(); i++){
        QTableWidgetItem *item = new QTableWidgetItem(filename.at(i));
        QTableWidgetItem *temp1 = new QTableWidgetItem("0");
        QTableWidgetItem *temp2 = new QTableWidgetItem("0");
        QString name = filename.at(i);
        name.chop(5);
        int j;
        for(j=name.size()-1;j>-1;j--)if(name.at(j)=='/')break;
        name.remove(0,j+1);
        QTableWidgetItem *com = new QTableWidgetItem(name);
        ui->File_list->setRowCount(ui->File_list->rowCount()+1);
        ui->File_list->setItem(ui->File_list->rowCount()-1,0,item);
        ui->File_list->setItem(ui->File_list->rowCount()-1,1,com);
        ui->File_list->setItem(ui->File_list->rowCount()-1,3,temp1);
        ui->File_list->setItem(ui->File_list->rowCount()-1,4,temp2);
    }
}

void Setting::on_File_remove_clicked()
{
    QList<QTableWidgetItem*> removing = ui->File_list->selectedItems();
    for(int i=0; i<removing.size(); i++){
        ui->File_list->removeRow(ui->File_list->row(removing.at(i)));
    }
}

void Setting::on_File_load_clicked()
{
    QList<QTableWidgetItem*> loading = ui->File_list->selectedItems();
    if(loading.empty()){
        QMessageBox msgBox;
        QFont font;
        font.setBold(true);
        font.setPointSize(12);
        msgBox.setFont(font);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("No file is selected.");
        msgBox.exec();
    }
    else{
        QString filename;
        if(loading.first()->column()==0)filename = loading.first()->text();
        else filename = ui->File_list->item(loading.first()->row(),0)->text();
        emit Setting_file_changed(filename);
    }
}

void Setting::on_Move_up_clicked()
{
    QList<QTableWidgetItem*> items = ui->File_list->selectedItems();
    if(items.isEmpty())return;
    else{
        if(items.first()->row()==0)return;
        else{
            int currentRow = items.first()->row();
            for(int i=0; i<3; i++)ui->File_list->takeItem(currentRow,i);
            for(int i=0; i<3; i++)ui->File_list->setItem(currentRow,i,ui->File_list->takeItem(currentRow-1,i));
            for(int i=0; i<items.size(); i++)ui->File_list->setItem(currentRow-1,i,items.at(i));
            ui->File_list->selectRow(currentRow-1);
        }
    }
}

void Setting::on_Move_down_clicked()
{
    QList<QTableWidgetItem*> items = ui->File_list->selectedItems();
    if(items.isEmpty())return;
    else{
        if(items.first()->row()==(ui->File_list->rowCount()-1))return;
        else{
            int currentRow = items.first()->row();
            for(int i=0; i<3; i++)ui->File_list->takeItem(currentRow,i);
            for(int i=0; i<3; i++)ui->File_list->setItem(currentRow,i,ui->File_list->takeItem(currentRow+1,i));
            for(int i=0; i<items.size(); i++)ui->File_list->setItem(currentRow+1,i,items.at(i));
            ui->File_list->selectRow(currentRow+1);
        }
    }
}

void Setting::on_Offline_mode_clicked()
{
    on_Apply_clicked();
}

void Setting::on_Apply_clicked()
{
    QStringList newSetting;
    newSetting.append(QString::number(ui->Cpu_type->currentIndex()));
    newSetting.append(QString::number(ui->Analog_channel->currentIndex()));
    newSetting.append(QString::number(ui->Digital_channel->currentIndex()));
    if(ui->Offline_mode->isChecked())newSetting.append("offline");
    else newSetting.append("online");
    if(ui->Precalculation->isChecked())newSetting.append("preon");
    else newSetting.append("preoff");
    if(ui->TCP_switch->isChecked())newSetting.append("tcpon");
    else newSetting.append("tcpoff");
    newSetting.append(ui->TCP_Port->text());
    if(ui->StopafterScan->isChecked())newSetting.append("StopAfterScan");
    else newSetting.append("ContinueAfterScan");


    bool need_reset = false;
    if(program_setting.at(1)!=newSetting.at(1))need_reset = true;
    if(program_setting.at(2)!=newSetting.at(2))need_reset = true;

    if(need_reset){
        QMessageBox msgBox;
        msgBox.setText("The program will be reset. All unsaved setting will be lost.");
        msgBox.setInformativeText("Do you want to proceed?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Ok:
            emit System_setting_apply(newSetting);
            return;
            break;
        case QMessageBox::Cancel:
            return;
            break;
        default:
            break;
        }
    }
    else{
        emit System_setting_apply(newSetting);
    }
}

void Setting::on_Mission_start_clicked()
{
    if(Mission){
        Mission = false;
        ui->Mission_start->setText("Mission\nStart");
        cycle_mission_check(-2);
    }
    else{
        Mission = true;
        ui->Mission_start->setText("Mission\nStop");
        ui->File_list->setDisabled(true);
        cycle_mission_check(-1);
    }
}

void Setting::cycle_mission_check(int state){
    if(!Mission||ui->File_list->selectedItems().isEmpty()){
        Mission = false;
        ui->Mission_start->setText("Mission\nStart");
        ui->File_list->setEnabled(true);
        return;
    }
    int row = ui->File_list->selectedItems().first()->row();
    int run = ui->File_list->item(row,3)->text().toInt();
    int scan = ui->File_list->item(row,4)->text().toInt();

    switch (state){
    case -2: {   //Mission Stop
        emit Run_Clicked();
        break;
    }
    case -1: {   //Mission Start
        if((run+scan)!=0){
            on_File_load_clicked();
            Delay(500);
            Mission_used_new_profile = true;
            emit Load_Clicked();
        }
        else if((row+1)==ui->File_list->rowCount()){
            Mission = false;
            ui->Mission_start->setText("Mission\nStart");
            ui->File_list->setEnabled(true);
        }
        else{
            ui->File_list->selectRow(row+1);
            cycle_mission_check(-1);
        }
        break;
    }
    case 0: {    //Run Complete
        if(run<=1 && scan!=0)emit Scan_Clicked();
        else if(run<=1)emit Run_Clicked();
        ui->File_list->item(row,3)->setText(QString::number(run-1));
        break;
    }
    case 1: {    //Scan Complete
        if(scan<=1)emit Run_Clicked();
        ui->File_list->item(row,4)->setText(QString::number(scan-1));
        break;
    }
    case 2: {    //Reload Complete
        if(Mission_used_new_profile){
            if(run!=0)emit Run_Clicked();
            else emit Scan_Clicked();
            Mission_used_new_profile = false;
        }
        break;
    }
    case 3: {    //Cycle Stopped
        if(row+1==ui->File_list->rowCount()){
            Mission = false;
            ui->Mission_start->setText("Mission\nStart");
            ui->File_list->setEnabled(true);
        }
        else{
            ui->File_list->selectRow(row+1);
            Delay(100);
            cycle_mission_check(-1);
        }
        break;
    }
    }
}

void Setting::Delay(int millisecondsToWait){
    QTime dieTime = QTime::currentTime().addMSecs(millisecondsToWait);
    while(QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}

void Setting::on_TCP_switch_clicked(){
    if(ui->TCP_switch->isChecked()){
        bool ok = false;
        int port = ui->TCP_Port->text().toInt(&ok);
        if(ok){
            if(RCS->TCPServer_Listen_Start(port))ui->TCP_Port->setDisabled(true);
            else{
                RCS->TCPServer_Close();
                ui->TCP_switch->setChecked(false);

                QMessageBox msgBox;
                QFont font;
                font.setBold(true);
                font.setPointSize(12);
                msgBox.setFont(font);
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText("Fail to Open Port "+QString::number(port));
                msgBox.exec();
            }
        }
        else{
            RCS->TCPServer_Close();
            ui->TCP_switch->setChecked(false);

            QMessageBox msgBox;
            QFont font;
            font.setBold(true);
            font.setPointSize(12);
            msgBox.setFont(font);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText("Invalid Port Number.");
            msgBox.exec();
        }
    }
    else{
        RCS->TCPServer_Close();
        ui->TCP_switch->setChecked(false);
        ui->TCP_Port->setEnabled(true);
    }
    on_Apply_clicked();
}

void Setting::on_Precalculation_clicked()
{
    on_Apply_clicked();
}

void Setting::on_StopafterScan_clicked()
{
    on_Apply_clicked();
}
