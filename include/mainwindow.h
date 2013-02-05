#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

#include <map>
#include <def_billon.h>

#include <io/Pgm3dFactory.h>
#include <AntHillManager.hpp>
#include <QGroupBox>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void updateProjectsList() ;

    void openNewBillon();
    void drawSlice( bool newContent=true);
    void closeImage();
    bool eventFilter(QObject *obj, QEvent *event);
protected:
    void on__labelSliceView_customContextMenuRequested(const QPoint &pos);
    void updateDictionary( ) ;
    void initRessources( QWidget *parent = 0 ) ;
    
private slots:
    void on_actionClose_folder_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_folder_triggered();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);


    void on_binPushButton_clicked();

    void on_segmLoadButton_clicked();
    void on_skelLoadButton_clicked();

	void toggled_config_view(bool checked ) ;
    void on_actionOpen_project_triggered();
	void changeRessourcesConfigView(int,int);
	void updateRessources() ;
	void resetRessources() ;
	void changeRessourceColor(int row,int column) ;
	void onChangeAxis( int ) ;
	
	void onChangeBoolParameter(bool v) ;
	void onChangeIntParameter(int v) ;

    void on_segmCheckBox_stateChanged(int arg1);
    void on_sequenceSlider_sliderMoved(int position);
    void on_sequenceSlider_valueChanged(int value);
    void on_checkBox_stateChanged(int arg1);
	
private:
	QColor getColorOf( uint row ) ;
	QColor getColorOf( const QString & search ) ;
	
    Ui::MainWindow *_ui;
    QImage _mainPix;
    
    QGroupBox *_ressourcesGroupBox ;
    QTableWidget *_ressourcesTable ;
    
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
