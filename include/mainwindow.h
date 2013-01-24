#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

#include <map>
#include <def_billon.h>
#include "sliceview.h"

#include <io/Pgm3dFactory.h>
#include <AntHillManager.hpp>

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void update_list_of_series() ;

    void openNewBillon();
    void drawSlice( bool newContent=true);
    void closeImage();
    bool eventFilter(QObject *obj, QEvent *event);
protected:
    void on__labelSliceView_customContextMenuRequested(const QPoint &pos);
    void gen_sep_pgm3d( ) ;
    void showDictionary( ) ;
private slots:
    void on_actionClose_folder_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_folder_triggered();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_sequenceSlider_sliderMoved(int position);

    void on_sequenceSlider_valueChanged(int value);

    void on_binPushButton_clicked();

    void on_spinMinIntensity_valueChanged(int arg1);

    void on_spinMaxIntensity_valueChanged(int arg1);

    void on_checkBox_stateChanged(int arg1);

    void on_binSpinBox_valueChanged(int arg1);

    void on_segmLoadButton_clicked();

    void on_segmCheckBox_stateChanged(int arg1);

    void on_x_shift_valueChanged(int arg1);

    void on_y_shift_valueChanged(int arg1);

    void on_z_shift_valueChanged(int arg1);

    void on_contentCheckBox_stateChanged(int arg1);

    void on_skelLoadButton_clicked();

    void on_y_shift_skel_valueChanged(int arg1);

    void on_x_shift_skel_valueChanged(int arg1);

    void on_contentSkelCheckBox_stateChanged(int arg1);

    void on_skelCheckBox_stateChanged(int arg1);

    void on_z_shift_skel_valueChanged(int arg1);



    void on_actionOpen_project_triggered();

private:
    Ui::MainWindow *_ui;
    QImage _mainPix;
    SliceView *_sliceView;

    QString _currentRepository ;

    Billon *_billon ;
    uint _currentSlice;
    uint _currentSerie ;
    BillonTpl<char> *_segmImg ;
    BillonTpl<char> *_skelImg ;
    Pgm3dFactory<char> factory ;

    bool _bViewSegm ;
    qreal _zoomFactor ;
	AntHillManager antHillMng ;
};

#endif // MAINWINDOW_H
