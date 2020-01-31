#ifndef VARIABLECLASSDISPLAY_H
#define VARIABLECLASSDISPLAY_H

#include <QWidget>
#include "include/variableclass.h"
#include "Addon/variablewidget.h"

namespace Ui {
class VariableClassDisplay;
}

class VariableClassDisplay : public QWidget
{
    Q_OBJECT

signals:
    void DisplayRemove(VariableClassDisplay*);
    void DisplayMoveLeft(VariableClassDisplay*);
    void DisplayMoveRight(VariableClassDisplay*);
    void DisplayMoveUp(VariableClassDisplay*);
    void DisplayMoveDown(VariableClassDisplay*);

public:
    explicit VariableClassDisplay(QWidget *parent = 0, VariableClass *vc = NULL);
    ~VariableClassDisplay();
    void set_height(int height);
    VariableClass* get_VariableClass();

public slots:
    void Update();

private slots:
    void on_Delete_clicked();
    void on_MoveLeft_clicked();
    void on_MoveRight_clicked();
    void on_MoveUp_clicked();
    void on_MoveDown_clicked();

private:
    Ui::VariableClassDisplay *ui;
    VariableClass *variableClass;
};

#endif // VARIABLECLASSDISPLAY_H
