#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QMainWindow>

namespace Ui {
class Simulator;
}

class Simulator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Simulator(QWidget *parent = 0);
    ~Simulator();

private slots:
    void on_Open_triggered();

    void on_Save_triggered();

    void on_Quit_triggered();

    void on_Clear_clicked();

    void on_Run_triggered();

    void on_Clear_Mem_clicked();

    void on_Clr_Sym_clicked();

private:
    Ui::Simulator *ui;
};

#endif // SIMULATOR_H
