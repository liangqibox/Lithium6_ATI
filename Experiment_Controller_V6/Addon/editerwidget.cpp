#include "editerwidget.h"
#include "ui_editerwidget.h"

Editerwidget::Editerwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Editerwidget)
{
    ui->setupUi(this);
    seq_trigger = NULL;
    type = 0;
    Type = new QiCombobox(this);
    Type->setGeometry(5,5,61,22);
    Type->setFocusPolicy(Qt::StrongFocus);
    Type->addItems(QStringList() << "Void" << "Const" << "LRamp" << "Exp" << "Sin" << "Pulse" << "SRamp" << "Stand");
    connect(Type,SIGNAL(currentIndexChanged(int)),this,SLOT(Type_Changed(int)));

    for(int i=0; i<4; i++){
        Value[i] = new QLineEdit(this);
        Value[i]->setGeometry(5+i%2*65,30+i/2*25,60,22);
        Value[i]->setContextMenuPolicy(Qt::NoContextMenu);
        Value[i]->installEventFilter(this);
        Value[i]->setDisabled(true);

        QSignalMapper *signalmapper = new QSignalMapper(Value[i]);
        connect(Value[i],SIGNAL(editingFinished()),signalmapper,SLOT(map()));
        signalmapper->setMapping(Value[i],i);
        connect(signalmapper,SIGNAL(mapped(int)),this,SLOT(Value_changing(int)));
    }
}

Editerwidget::~Editerwidget()
{
    for(int i=0; i<4; i++)delete Value[i];
    delete ui;
}

void Editerwidget::initial(Sequence *iseq, QList<Variable *> *ivari){
    seq = iseq;
    variables = ivari;
    double *v = seq->get_value();

    if(int(v[0])==-1)type = DIGITAL_SEQUENCE;
    else type = ANALOG_SEQUENCE;
    Type->setCurrentIndex(int(v[0]));
    //on_Type_currentIndexChanged(int(v[0]));
    for(int i=0; i<4; i++){
        if(seq->get_variables()[i]==NULL)Value[i]->setText(QString::number(v[i+1]));
        else if(seq->get_variables()[i]->get_type()!=3){
            Value[i]->setDisabled(true);
            Value[i]->setText(seq->get_variables()[i]->get_name());
            connect(seq->get_variables()[i],SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
        }
        else{
            Value[i]->setEnabled(true);
            connect(seq->get_variables()[i],SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
        }
    }

    warning(seq->get_wrong());
    connect(seq,SIGNAL(Wrong(bool)),this,SLOT(warning(bool)));
    connect(this,SIGNAL(Right_clicked(int)),seq,SLOT(Right_clicked(int)));
    connect(seq,SIGNAL(Variable_add(QString)),this,SLOT(receive_variable(QString)));
    connect(seq,SIGNAL(Changed(bool)),this,SLOT(sequence_changed(bool)));
}

void Editerwidget::initial(Sequence *iseq, Sequence *iseq_trg, QList<Variable *> *ivari){
    seq = iseq;
    seq_trigger = iseq_trg;
    variables = ivari;
    double *v = seq->get_value();
    type = DDS_SEQUENCE;

    Type->setCurrentIndex(int(v[0]));
    //on_Type_currentIndexChanged(int(v[0]));
    for(int i=0; i<4; i++){
        if(seq->get_variables()[i]==NULL)Value[i]->setText(QString::number(v[i+1]));
        else if(seq->get_variables()[i]->get_type()!=3){
            Value[i]->setDisabled(true);
            Value[i]->setText(seq->get_variables()[i]->get_name());
            connect(seq->get_variables()[i],SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
        }
        else{
            Value[i]->setEnabled(true);
            connect(seq->get_variables()[i],SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
        }
    }

    warning(seq->get_wrong());
    connect(seq,SIGNAL(Wrong(bool)),this,SLOT(warning(bool)));
    connect(this,SIGNAL(Right_clicked(int)),seq,SLOT(Right_clicked(int)));
    connect(seq,SIGNAL(Variable_add(QString)),this,SLOT(receive_variable(QString)));
    connect(seq,SIGNAL(Changed(bool)),this,SLOT(sequence_changed(bool)));
}

bool Editerwidget::eventFilter(QObject *obj, QEvent *event){
    QMouseEvent *a = static_cast<QMouseEvent*>(event);
    if(event->type()==QEvent::MouseButtonPress&&a->button()==Qt::RightButton){
        for(int i=0; i<4; i++){
            if(obj==Value[i] && (Value[i]->isEnabled()||seq->get_variables()[i]!=NULL)){
                emit Right_clicked(i);
                return true;
            }
        }
    }
    return false;
}

void Editerwidget::mousePressEvent(QMouseEvent *event){
    if(event->button()==Qt::RightButton)emit Right_clicked(-1);
}

void Editerwidget::Type_Changed(int index)
{
    switch (index) {
    case -2:
        Type->clear();
        Type->addItem("DDS");
        Type->setDisabled(true);
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setEnabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Start Feq (MHz)");
        Value[2]->setToolTip("End Feq (MHz)");
        Value[3]->setToolTip("Amplitude (0~1)");
        break;
    case -1:
        Type->clear();
        Type->addItem("DIO");
        Type->setDisabled(true);
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Period (ms)");
        Value[2]->setToolTip("On Ratio");
        Value[3]->setToolTip("");
        break;
    case 0:
        Value[0]->setDisabled(true);
        Value[1]->setDisabled(true);
        Value[2]->setDisabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("");
        Value[1]->setToolTip("");
        Value[2]->setToolTip("");
        Value[3]->setToolTip("");
        break;
    case 1:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setDisabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Amplitude (V)");
        Value[2]->setToolTip("");
        Value[3]->setToolTip("");
        break;
    case 2:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Start (V)");
        Value[2]->setToolTip("End (V)");
        Value[3]->setToolTip("");
        break;
    case 3:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Start (V)");
        Value[2]->setToolTip("End (V)");
        Value[3]->setToolTip("");
        break;
    case 4:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setEnabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Amplitude (V)");
        Value[2]->setToolTip("Frequency (kHz)");
        Value[3]->setToolTip("Offset (V)");
        break;
    case 5:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setEnabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Amplitude (V)");
        Value[2]->setToolTip("Period (ms)");
        Value[3]->setToolTip("On Ratio");
        break;
    case 6:
        Value[0]->setEnabled(true);
        Value[1]->setEnabled(true);
        Value[2]->setEnabled(true);
        Value[3]->setEnabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("Start (V)");
        Value[2]->setToolTip("End (V)");
        Value[3]->setToolTip("Steepness");
        break;
    case 7:
        Value[0]->setEnabled(true);
        Value[1]->setDisabled(true);
        Value[2]->setDisabled(true);
        Value[3]->setDisabled(true);
        Value[0]->setToolTip("Time (ms)");
        Value[1]->setToolTip("");
        Value[2]->setToolTip("");
        Value[3]->setToolTip("");
        break;
    default:
        break;
    }
    for(int i=0; i<4; i++)if(seq->get_variables()[i]!=NULL)Value[i]->setDisabled(true);
    seq->set_value(0,double(index));
}

void Editerwidget::Value_changing(int i){
    if(!Value[i]->isEnabled())return;
    bool ok = false;
    double v = Value[i]->text().toDouble(&ok);
    if(ok){
        seq->set_value(i+1,v);
        seq->set_change(true);
    }
    else Value[i]->setText(QString::number(seq->get_value()[i+1]));
}

void Editerwidget::receive_variable(QString a){
    QStringList v = a.split('!');
    Variable *temp;
    int i = v.at(0).toInt();
    bool del = false;
    bool cal = false;
    if(v.at(2)=="del")del=true;
    for(int j=0; j<variables->size(); j++){
        if(variables->at(j)->get_name()==v.at(1)){
            temp = variables->at(j);
            if(temp->get_type()==3)cal=true;
            break;
        }
    }
    if(del){
        Value[i]->setText("0");
        disconnect(temp,SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
    }
    else{
        Value[i]->setText(v.at(1));
        connect(temp,SIGNAL(name_changed(QString)),Value[i],SLOT(setText(QString)));
    }
    Value[i]->setDisabled(!del);
    Value[i]->setEnabled(del|cal);
}

void Editerwidget::setWindowOpacity(qreal level){
    ui->background->setWindowOpacity(level);
    Type->setWindowOpacity(level);
    for(int i=0; i<4; i++)Value[i]->setWindowOpacity(level);
}

void Editerwidget::sequence_changed(bool changed){
    QColor marked = QColor(220,220,220);
    QColor unmarked = QColor(235,235,235);
    if(changed){
        set_color(marked);
    }
    else{
        set_color(unmarked);
    }
}

void Editerwidget::set_color(QColor color){
    QPixmap back = QPixmap(135,82);
    back.fill(color);
    ui->background->setPixmap(back);
}

void Editerwidget::warning(bool wrong){
    if(wrong){
        QPixmap red = QPixmap(15,15);
        red.fill(QColor(240,240,240));
        QPainter painter(&red);
        painter.setBrush(Qt::red);
        painter.drawEllipse(1,1,13,13);
        painter.end();
        ui->Warning->setPixmap(red);
    }
    else ui->Warning->clear();
}
