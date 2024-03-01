#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtGui>
#include <QtCore>
#include <QVideoFrame>
#include <QCamera>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    qreal mapToRange(qreal number, qreal inputStart, qreal inputEnd, qreal outputStart, qreal outputEnd);

    void renderImageAsASCII(QString filename = "", QImage image = QImage());
    void renderImageAsRect(QString filename = "", QImage image = QImage());

    void prepareRects();
    void prepareText();
    void setCopyButtonState(bool enabled);

public slots:
    void updateFrameRects(QVideoFrame frame);
    void updateFrameText(QVideoFrame frame);
    void img_to_ascii_btn_clicked();
    void img_to_block_btn_clicked();
    void stop_video();
    void loadImage();
    void play();
    void copyAscii();

private:
    QImage m_img;
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QGraphicsTextItem *m_videoText;
    QList<QGraphicsRectItem*> m_canvasPixels;
    QList<QGraphicsSimpleTextItem*> m_canvasText;
    QCamera *m_camera;

    QPushButton *m_loadImageBtn;
    QRadioButton *img_ascii_radio;
    QRadioButton *img_blocks_radio;
    QButtonGroup *m_img_radio_group;
    QPushButton *m_playVideoButton;
    QRadioButton *start_ascii_radio;
    QRadioButton *start_blocks_radio;
    QButtonGroup *m_video_radio_group;
    QPushButton *m_stop_video_btn;
    QPushButton *m_copyAscii_btn;

    QString m_ascii_string;
    const QString DENSITY_STRING = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'.                                                ";    
};
#endif // MAINWINDOW_H
