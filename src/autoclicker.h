#ifndef AUTOCLICKER_H
#define AUTOCLICKER_H

#include <QMainWindow>

namespace Ui {
class AutoClicker;
}

class AutoClicker : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutoClicker(QWidget *parent = 0);
    ~AutoClicker();

private slots:
    void on_actionExit_2_triggered();

    void on_doubleClickBox_clicked();

    void on_minCPSBox_valueChanged(int arg1);

    void on_maxCPSBox_valueChanged(int arg1);

    void on_maxRandomBox_clicked();

    void on_legitLookingBox_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_msBox_valueChanged(int arg1);

    void on_wtapBox_clicked();

    void on_wtapValueBox_valueChanged(int arg1);

    void on_wtapValueBox_2_valueChanged(int arg1);

    void on_actionSave_triggered();

private:
    Ui::AutoClicker *ui;
};

#endif // AUTOCLICKER_H
