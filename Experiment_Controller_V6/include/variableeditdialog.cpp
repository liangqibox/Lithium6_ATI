#include "variableeditdialog.h"
#include "ui_variableeditdialog.h"

VariableEditDialog::VariableEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VariableEditDialog)
{
    ui->setupUi(this);
}

VariableEditDialog::~VariableEditDialog()
{
    delete ui;
}
