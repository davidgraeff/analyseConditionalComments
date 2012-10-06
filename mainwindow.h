#pragma once

#include <QMainWindow>

class QListWidgetItem;
class AnalyserManager;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private Q_SLOTS:
    void finished(bool aborted);
  
    void on_btnSearch_clicked();

    void on_listConcerns_itemChanged(QListWidgetItem *item);

    void on_btnRefresh_clicked();

    void on_btnToggle_clicked();

    void on_btnAll_clicked();

    void on_btnFilter_clicked();

    void on_btnToggle_2_clicked();

    void on_btnAll_2_clicked();

    void on_btnFilter_2_clicked();

    void on_btnSwitchToOutput_clicked();

    void on_btnOutputINI_clicked();

    void on_btnOutputSVG_clicked();

    void on_btnAnalyse_clicked();

    void on_btnSearchINI_clicked();

    void on_btnOutputPDF_clicked();

    void on_btnOutputPNG_clicked();

    void on_listFiles_itemChanged(QListWidgetItem *item);

    void on_chkVertical_stateChanged(int arg1);

    void on_chkUmbruch_stateChanged(int arg1);

    void on_chkVariableSize_stateChanged(int arg1);

    void on_chkHideFiles_stateChanged(int arg1);

    void on_spinUmbruch_valueChanged(int arg1);

    void on_spinMax_valueChanged(int arg1);

    void on_spinMin_valueChanged(int arg1);

    void on_spinWidth_valueChanged(int arg1);

    void on_btnEnableSelected_clicked();

    void on_btndisableSelected_clicked();

    void on_btnChangeColorSelected_clicked();

    void on_btnEnableSelected_2_clicked();

    void on_btndisableSelected_2_clicked();

    void on_btnChangeColorSelected_2_clicked();

    void on_chkHideFiles2_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    AnalyserManager* m_analysemanager;
    void setItemColor(QListWidgetItem*item, QColor color, bool isFileItem);
private Q_SLOTS:
    void progressValueChanged(int);
    void svgGenerated();
    void on_chkSortFilesSize_clicked();
};
