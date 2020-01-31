#ifndef VARIABLEEDITDIALOG_H
#define VARIABLEEDITDIALOG_H

#include <QDialog>

namespace Ui {
class VariableEditDialog;
}

class VariableEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VariableEditDialog(QWidget *parent = 0);
    ~VariableEditDialog();

private:
    Ui::VariableEditDialog *ui;
};

#endif // VARIABLEEDITDIALOG_H
