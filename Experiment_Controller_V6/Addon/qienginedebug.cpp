#include "qienginedebug.h"
#include "ui_qienginedebug.h"

QiEngineDebug::QiEngineDebug(QWidget *parent, QScriptEngine *engine) :
    QWidget(parent),
    ui(new Ui::QiEngineDebug)
{
    ui->setupUi(this);

    QiEngine = engine;
}

QiEngineDebug::~QiEngineDebug()
{
    delete ui;
}

void QiEngineDebug::on_input_returnPressed()
{
    ui->output->append(QiEngine->evaluate(ui->input->text()).toString());
}

void QiEngineDebug::on_pushButton_clicked()
{
    ui->output->clear();
}
