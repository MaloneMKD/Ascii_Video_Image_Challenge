#include "mainwindow.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QGraphicsRectItem>
#include <random>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QGraphicsVideoItem>
#include <QVideoSink>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //ui->setupUi(this);

    // Window settings
    this->setWindowTitle("");

    this->setStyleSheet("QMainWindow {"
                        "color: #FFFFFF;"
                        "background-color: #454545;"
                        "font-family: Corbel Light;"
                        "}");

    // Misc
    srand(time(0));
    m_camera = nullptr;

    // Create camera
    QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    if(availableCameras.length() != 0)
        m_camera = new QCamera(availableCameras.first(), this);

    // Create the layouts
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QHBoxLayout *viewLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(viewLayout);

    // Copy ascii button
    m_copyAscii_btn = new QPushButton("Get Image Ascii");
    setCopyButtonState(false);
    m_copyAscii_btn->setFixedSize(120, 40);
    buttonLayout->addWidget(m_copyAscii_btn);
    connect(m_copyAscii_btn, SIGNAL(clicked(bool)), this, SLOT(copyAscii()));

    // Create buttons, scene and view
        // Load image buttons
    m_loadImageBtn = new QPushButton("Load Image");
    m_loadImageBtn->setStyleSheet("QPushButton{"
                                  "background-color: #FFFFFF;"
                                  " color: #242424;"
                                  "font-family: Segoe UI;"
                                  " border-radius: 5;"
                                  "}"
                                  ""
                                  "QPushButton:hover{"
                                  "color: #FFFFFF;"
                                  " background-color: #4E8A56;"
                                  "}"
                                  "");
    m_loadImageBtn->setFixedSize(120, 40);
    buttonLayout->addWidget(m_loadImageBtn);

    img_ascii_radio = new QRadioButton("To Ascii");
    img_blocks_radio = new QRadioButton("To Blocks");
    img_ascii_radio->setStyleSheet("QRadioButton{"
                                   "    color: #FFFFFF;"
                                   "}");
    img_blocks_radio->setStyleSheet("QRadioButton{"
                                   "    color: #FFFFFF;"
                                   "}");

    img_ascii_radio->setFixedSize(120, 30);
    img_blocks_radio->setFixedSize(120, 30);
    img_ascii_radio->setChecked(true);
    m_img_radio_group = new QButtonGroup();
    m_img_radio_group->addButton(img_ascii_radio, 0);
    m_img_radio_group->addButton(img_blocks_radio, 1);
    m_img_radio_group->setExclusive(true);

    buttonLayout->addWidget(img_ascii_radio);
    buttonLayout->addWidget(img_blocks_radio);
    buttonLayout->addSpacing(200);

        // Play and stop buttons
    m_playVideoButton = new QPushButton("Play");
    m_playVideoButton->setStyleSheet("QPushButton{"
                                     "color: #FFFFFF;"
                                     "background-color: #454545;"
                                     "background-image: url(:/images/images/play.png);"
                                     "background-repeat: no-repeat;"
                                     "border-radius: 5;"
                                     "width: 50px;"
                                     "height: 25px;"
                                     "}"
                                     ""
                                     "QPushButton:hover{"
                                     "  background-image: url(:/images/images/play_hover.png);"
                                     "}");

    m_stop_video_btn = new QPushButton("Stop");
    m_stop_video_btn->setStyleSheet("QPushButton{"
                                    "color: #FFFFFF;"
                                     "background-color: #454545;"
                                     "background-image: url(:/images/images/stop.png);"
                                     "background-repeat: no-repeat;"
                                    "border-radius: 5;"
                                     "width: 50px;"
                                     "height: 25px;"
                                     "}"
                                    ""
                                    "QPushButton:hover{"
                                    "   background-image: url(:/images/images/stop_hover.png);"
                                    "}");
    m_playVideoButton->setFixedSize(120, 40);
    m_stop_video_btn->setFixedSize(120, 40);
    buttonLayout->addWidget(m_playVideoButton);
    buttonLayout->addWidget(m_stop_video_btn);

    start_ascii_radio = new QRadioButton("Ascii Video");
    start_blocks_radio = new QRadioButton("Blocks Video");

    start_ascii_radio->setStyleSheet("QRadioButton{"
                                     "    color: #FFFFFF;"
                                     "}");
    start_blocks_radio->setStyleSheet("QRadioButton{"
                                    "    color: #FFFFFF;"
                                    "}");

    start_ascii_radio->setFixedSize(120, 30);
    start_blocks_radio->setFixedSize(120, 30);

    start_ascii_radio->setChecked(true);

    m_video_radio_group = new QButtonGroup();
    m_video_radio_group->addButton(start_ascii_radio, 0);
    m_video_radio_group->addButton(start_blocks_radio, 1);
    m_video_radio_group->setExclusive(true);

    buttonLayout->addWidget(start_ascii_radio);
    buttonLayout->addWidget(start_blocks_radio);
    buttonLayout->addSpacing(50);

    connect(m_loadImageBtn, SIGNAL(clicked(bool)), this, SLOT(loadImage()));
    connect(m_playVideoButton, SIGNAL(clicked(bool)), this, SLOT(play()));
    connect(m_stop_video_btn, SIGNAL(clicked(bool)), this, SLOT(stop_video()));

    m_scene = new QGraphicsScene(QRectF(), this);
    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setAlignment(Qt::AlignCenter);
    viewLayout->addWidget(m_view);

    QWidget *dummy = new QWidget(this);
    dummy->setLayout(mainLayout);
    this->setCentralWidget(dummy);

    // Start ascii video
    // play();
}

MainWindow::~MainWindow()
{
}

qreal MainWindow::mapToRange(qreal number, qreal inputStart, qreal inputEnd, qreal outputStart, qreal outputEnd)
{
    qreal r = inputEnd - inputStart;
    qreal R = outputEnd - outputStart;
    qreal x = number - inputStart;
    qreal y = (R / r) * x;
    return outputStart + y;
}

void MainWindow::renderImageAsASCII(QString filename, QImage image)
{
    m_view->setAlignment(Qt::AlignCenter);
    QImage img;
    // Get the image and scale it
    if(filename != "")
    {
        img = QImage(filename);
        /*if(img.height() > img.width())
            img = img.scaledToHeight(85);
        else
            img = img.scaledToWidth(185);*/
        img = img.scaledToHeight(89);
    }
    else
        img = image;

    // Draw the character
    qreal x_pos = 0;
    qreal y_pos = 0;
    QGraphicsTextItem *text = nullptr;
    m_ascii_string = "";

    for(int i = 0; i < img.height(); i++)
    {
        for(int j = 0; j < img.width(); j++)
        {
            QColor temp = img.pixelColor(j, i);
            int brightness = temp.value();

            int mappedValue = mapToRange(brightness, 0, 255, 0, DENSITY_STRING.length() - 1);
            text = new QGraphicsTextItem(DENSITY_STRING[mappedValue]);
            m_ascii_string += DENSITY_STRING[mappedValue];
            text->setPos(x_pos, y_pos);
            x_pos += -text->font().pixelSize() * 8;
            m_scene->addItem(text);
        }
        x_pos = 0;
        m_ascii_string += "\n";
        if(text != nullptr)
            y_pos += -text->font().pixelSize() * 8;
    }
}

void MainWindow::renderImageAsRect(QString filename, QImage image)
{
    m_view->setAlignment(Qt::AlignCenter);
    QImage img = QImage();
    // Get the image and scale it
    if(filename != "")
    {
        img = QImage(filename);
        /*if(img.height() > img.width())
            img = img.scaledToHeight(72);
        else
            img = img.scaledToWidth(148);*/
        img = img.scaledToHeight(144);
    }
    else
        img = image;

    // Draw rectangles
    qreal x_pos = 0;
    qreal y_pos = 0;
    qreal pix_width = 5;
    qreal pix_height = 5;
    for(int i = 0; i < img.height(); i++)
    {
        for(int j = 0; j < img.width(); j++)
        {
            QGraphicsRectItem *pix = new QGraphicsRectItem(QRectF(x_pos, y_pos, pix_width, pix_height));
            pix->setPen(Qt::NoPen); //QPen(QColor("#242424"))

            QColor temp = img.pixelColor(j, i);
            int brightness = temp.value();
            pix->setBrush(QColor(brightness, brightness, brightness));

            m_canvasPixels.append(pix);
            m_scene->addItem(pix);
            x_pos += pix_width;
        }
        x_pos = 0;
        y_pos += pix_height;
    }
}

void MainWindow::prepareRects()
{
    // Draw rectangles
    qreal x_pos = 0;
    qreal y_pos = 0;
    qreal pix_width = 10;
    qreal pix_height = 10;
    for(int i = 0; i < 72; i++)
    {
        for(int j = 0; j < 148; j++)
        {
            QGraphicsRectItem *pix = new QGraphicsRectItem(QRectF(x_pos, y_pos, pix_width, pix_height));
            pix->setPen(QPen(QColor("#242424"))); //QPen(QColor("#242424"))
            m_canvasPixels.append(pix);
            m_scene->addItem(pix);
            x_pos += pix_width;
        }
        x_pos = 0;
        y_pos += pix_height;
    }
}

void MainWindow::prepareText()
{
    qreal x_pos = 0;
    qreal y_pos = 0;
    QGraphicsSimpleTextItem *text = nullptr;
    m_canvasText.clear();
    m_view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    for(int i = 0; i < 72; i++)
    {
        for(int j = 0; j < 150; j++)
        {
            text = new QGraphicsSimpleTextItem("8");
            text->setPos(x_pos, y_pos);
            text->setBrush(QBrush(Qt::black));
            text->setFont(QFont("Segoe UI", 8));
            text->setData(0, QPoint(j, i + 15));
            text->setFlag(QGraphicsItem::ItemIsSelectable);
            x_pos += -text->font().pixelSize() * 10;
            m_scene->addItem(text);
            m_canvasText.append(text);
        }
        x_pos = 0;
        if(text != nullptr)
            y_pos += -text->font().pixelSize() * 10;
    }
}

void MainWindow::setCopyButtonState(bool enabled)
{
    if(enabled == true){
        m_copyAscii_btn->setStyleSheet("QPushButton{"
                                       "background-color: #FFFFFF;"
                                       " color: #242424;"
                                       "font-family: Segoe UI;"
                                       " border-radius: 5;"
                                       "}"
                                       ""
                                       "QPushButton:hover{"
                                       "color: #FFFFFF;"
                                       " background-color: #4E8A56;"
                                       "}"
                                       "");
        m_copyAscii_btn->setEnabled(true);
    }
    else
    {
        m_copyAscii_btn->setStyleSheet("QPushButton{"
                                       "background-color: #555555;"
                                       " color: #D2D2D2;"
                                       "font-family: Segoe UI;"
                                       " border-radius: 5;"
                                       "}"
                                       ""
                                       "QPushButton:hover{"
                                       "color: #FFFFFF;"
                                       " background-color: #4E8A56;"
                                       "}"
                                       "");
        m_copyAscii_btn->setEnabled(false);
    }
}

void MainWindow::updateFrameRects(QVideoFrame frame)
{
    m_view->setAlignment(Qt::AlignCenter);
    QImage img = frame.toImage().scaledToWidth(148);
    img.mirror(true, false);

    // Draw rectangles
    qreal x_pos = 0;
    qreal y_pos = 0;
    for(int i = 0; i < 72; i++)
    {
        for(int j = 0; j < 148; j++)
        {
            QColor temp = img.pixelColor(j, i);
            int brightness = temp.value();
            m_canvasPixels[i * 148 + j]->setBrush(QColor(brightness, brightness, brightness));
            x_pos += 5;
        }
        x_pos = 0;
        y_pos += 5;
    }
}

void MainWindow::updateFrameText(QVideoFrame frame)
{
    m_img = frame.toImage().scaledToWidth(180);
    m_img.mirror(true, false);

    // Draw the character
    for( int i = 0; i < m_canvasText.length(); i++)
    {
        QColor temp = m_img.pixelColor(m_canvasText[i]->data(0).toPoint());
        int brightness = temp.value();
        int mappedValue = mapToRange(brightness, 0, 255, 0, DENSITY_STRING.length() - 1);
        m_canvasText[i]->setText(DENSITY_STRING[mappedValue]);
    }
    m_scene->update();
}

void MainWindow::img_to_ascii_btn_clicked()
{
    // Stop the camera if it's running
    if(m_camera->isActive())
        stop_video();

    // Get the filename
    QString filename = QFileDialog::getOpenFileName(this, "Choose Image", QDir::homePath());
    if(filename != "")
    {
        // Enable copy button
        setCopyButtonState(true);

        // Recreate the scene
        delete m_scene;
        m_scene = new QGraphicsScene(QRectF(), this);
        m_view->setScene(m_scene);

        renderImageAsASCII(filename);
    }
}

void MainWindow::img_to_block_btn_clicked()
{
    // Stop the camera if it's running
    if(m_camera->isActive())
        stop_video();

    // Disable copy button
    setCopyButtonState(false);

    // Get the filename
    QString filename = QFileDialog::getOpenFileName(this, "Choose Image", QDir::homePath());
    if(filename != "")
    {
        // Recreate the scene
        delete m_scene;
        m_scene = new QGraphicsScene(QRectF(), this);
        m_view->setScene(m_scene);

        renderImageAsRect(filename);
    }
}

void MainWindow::stop_video()
{
    // Stop the camera if it's running
    if(m_camera != nullptr && m_camera->isActive())
    {
        m_camera->stop();
        delete m_camera->captureSession()->videoSink();
        delete m_camera->captureSession();
    }
}

void MainWindow::loadImage()
{
    // Stop the camera if it's running
    if(m_camera != nullptr && m_camera->isActive())
        stop_video();

    // Get the filename
    QString filename = QFileDialog::getOpenFileName(this, "Choose Image", QDir::homePath());

    // Call appropriate function
    if(m_img_radio_group->checkedId() == 0) // ascii
    {
        if(filename != "")
        {
            // Recreate the scene
            delete m_scene;
            m_scene = new QGraphicsScene(QRectF(), this);
            m_view->setScene(m_scene);

            renderImageAsASCII(filename);

            // Enable the button
            setCopyButtonState(true);
        }
    }
    else
    {
        if(filename != "")
        {
            // Recreate the scene
            delete m_scene;
            m_scene = new QGraphicsScene(QRectF(), this);
            m_view->setScene(m_scene);

            renderImageAsRect(filename);

            // Disable copy button
            setCopyButtonState(false);
        }
    }
}

void MainWindow::play()
{
    // Stop the camera if it's running
    if(m_camera != nullptr && m_camera->isActive())
        stop_video();

    if(m_video_radio_group->checkedId() == 0) // ascii
    {
        // Disable copy button
        setCopyButtonState(false);

        // Recreate the scene
        delete m_scene;
        m_scene = new QGraphicsScene(QRectF(), this);
        m_view->setScene(m_scene);

        // Prepare the text
        prepareText();

        if(m_camera != nullptr)
        {
            // Create a media session
            QMediaCaptureSession *session = new QMediaCaptureSession(this);

            // Give the camera as he video input for the session
            session->setCamera(m_camera);

            // Create a video sink to capture frame by frame data
            QVideoSink *sink = new QVideoSink(this);
            session->setVideoSink(sink);
            connect(sink, SIGNAL(videoFrameChanged(QVideoFrame)), this, SLOT(updateFrameText(QVideoFrame)));

            //Start camera
            m_camera->start();
        }
    }
    else
    {
        // Disable copy button
        setCopyButtonState(false);

        // Recreate the scene
        delete m_scene;
        m_scene = new QGraphicsScene(QRectF(), this);
        m_view->setScene(m_scene);

        prepareRects();

        if(m_camera != nullptr)
        {
            // Create a media session
            QMediaCaptureSession *session = new QMediaCaptureSession(this);

            // Give the camera as he video input for the session
            session->setCamera(m_camera);

            // Create a video sink to capture frame by frame data
            QVideoSink *sink = new QVideoSink(this);
            session->setVideoSink(sink);
            connect(sink, SIGNAL(videoFrameChanged(QVideoFrame)), this, SLOT(updateFrameRects(QVideoFrame)));

            //Start camera
            m_camera->start();
        }
    }
}

void MainWindow::copyAscii()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setWindowTitle("Image ascii");
    mess->setFont(QFont("Segoe UI", 12));
    mess->setText("Ascii copied to clipboard!");
    mess->addButton(QMessageBox::Close);
    //mess->setGeometry(500, 500, 500, 300);
    mess->show();
    qApp->clipboard()->setText(m_ascii_string);
}

