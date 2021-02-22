/******************************
 * Qt player using libVLC     *
 * By protonux                *
 *                            *
 * Under WTFPL                *
 ******************************/

#include "player.h"

typedef signed __int64 ssize_t;

#include <vlc/vlc.h>

#define qtu( i ) ((i).toUtf8().constData())

#include <QtGui>
#include <QMessageBox>
#include <QMenuBar>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

struct stVLCPlayerPrivate {
    libvlc_instance_t *vlcInstance;
    libvlc_media_player_t *vlcPlayer;
};

Mwindow::Mwindow()
{
    m_stPlayer = new stVLCPlayerPrivate;

    m_stPlayer->vlcPlayer = nullptr;

    /* Initialize libVLC */
    m_stPlayer->vlcInstance = libvlc_new(0, nullptr);

    /* Complain in case of broken installation */
    if (m_stPlayer->vlcInstance == nullptr) {
        QMessageBox::critical(this, "Qt libVLC player", "Could not init libVLC");
        exit(1);
    }

    /* Interface initialization */
    initUI();
}

Mwindow::~Mwindow() {
    /* Release libVLC instance on quit */
    if (m_stPlayer->vlcInstance)
        libvlc_release(m_stPlayer->vlcInstance);

    delete m_stPlayer;
}

void Mwindow::initUI()
{
    /* Buttons for the UI */
    playBut = new QPushButton("Play");
    QObject::connect(playBut, SIGNAL(clicked()), this, SLOT(play()));

    QPushButton *stopBut = new QPushButton("Stop");
    QObject::connect(stopBut, SIGNAL(clicked()), this, SLOT(stop()));

    QPushButton *muteBut = new QPushButton("Mute");
    QObject::connect(muteBut, SIGNAL(clicked()), this, SLOT(mute()));

    QPushButton *fsBut = new QPushButton("Fullscreen");
    QObject::connect(fsBut, SIGNAL(clicked()), this, SLOT(fullscreen()));

    volumeSlider = new QSlider(Qt::Horizontal);
    QObject::connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeVolume(int)));
    volumeSlider->setValue(80);

    slider = new QSlider(Qt::Horizontal);
    slider->setMaximum(1000);
    QObject::connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(changePosition(int)));

    /* A timer to update the sliders */
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));
    timer->start(100);

    /* Central Widgets */
    QWidget* centralWidget = this;
    videoWidget = new QWidget;

    videoWidget->setAutoFillBackground( true );
    QPalette plt = palette();
    plt.setColor( QPalette::Window, Qt::black );
    videoWidget->setPalette( plt );

    /* Put all in layouts */
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(playBut);
    layout->addWidget(stopBut);
    layout->addWidget(muteBut);
    layout->addWidget(fsBut);
    layout->addWidget(volumeSlider);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setMargin(0);
    layout2->addWidget(videoWidget);
    layout2->addWidget(slider);
    layout2->addLayout(layout);

    centralWidget->setLayout(layout2);
    //setCentralWidget(centralWidget);
    //resize( 600, 400);
}

void Mwindow::openFile(QString fileOpen)
{
    /* The basic file-select box */
    //QString fileOpen = QFileDialog::getOpenFileName(this, tr("Load a file"), "~");

    if(fileOpen.isNull())
        return;

    /* Stop if something is playing */
    if (m_stPlayer->vlcPlayer && libvlc_media_player_is_playing(m_stPlayer->vlcPlayer))
        stop();

    /* Create a new Media */

#if defined(Q_OS_WIN)
    fileOpen.replace("/","\\");
#endif

    //auto stdstrPath = fileOpen.toStdString();
    //const char* path = stdstrPath.c_str();
    libvlc_media_t *vlcMedia = libvlc_media_new_path(m_stPlayer->vlcInstance, qtu(fileOpen));
    if (!vlcMedia)
        return;

    /* Create a new libvlc player */
    m_stPlayer->vlcPlayer = libvlc_media_player_new_from_media (vlcMedia);

    /* Release the media */
    libvlc_media_release(vlcMedia);

    /* Integrate the video in the interface */
#if defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(vlcPlayer, (void *)videoWidget->winId());
#elif defined(Q_OS_UNIX)
    libvlc_media_player_set_xwindow(vlcPlayer, videoWidget->winId());
#elif defined(Q_OS_WIN)
    libvlc_media_player_set_hwnd(m_stPlayer->vlcPlayer, (HWND)videoWidget->winId());
#endif

    /* And start playback */
    libvlc_media_player_play (m_stPlayer->vlcPlayer);

    /* Update playback button */
    playBut->setText("Pause");
}

void Mwindow::play() {
    if (!m_stPlayer->vlcPlayer)
        return;

    if (libvlc_media_player_is_playing(m_stPlayer->vlcPlayer))
    {
        /* Pause */
        libvlc_media_player_pause(m_stPlayer->vlcPlayer);
        playBut->setText("Play");
    }
    else
    {
        /* Play again */
        libvlc_media_player_play(m_stPlayer->vlcPlayer);
        playBut->setText("Pause");
    }
}

int Mwindow::changeVolume(int vol) { /* Called on volume slider change */

    if (m_stPlayer->vlcPlayer)
        return libvlc_audio_set_volume (m_stPlayer->vlcPlayer,vol);

    return 0;
}

void Mwindow::changePosition(int pos) { /* Called on position slider change */

    if (m_stPlayer->vlcPlayer)
        libvlc_media_player_set_position(m_stPlayer->vlcPlayer, (float)pos/1000.0);
}

void Mwindow::updateInterface() { //Update interface and check if song is finished

    if (!m_stPlayer->vlcPlayer)
        return;

    /* update the timeline */
    float pos = libvlc_media_player_get_position(m_stPlayer->vlcPlayer);
    slider->setValue((int)(pos*1000.0));

    /* Stop the media */
    if (libvlc_media_player_get_state(m_stPlayer->vlcPlayer) == libvlc_Ended)
        this->stop();
}

void Mwindow::stop() {
    if(m_stPlayer->vlcPlayer) {
        /* stop the media player */
        libvlc_media_player_stop(m_stPlayer->vlcPlayer);

        /* release the media player */
        libvlc_media_player_release(m_stPlayer->vlcPlayer);

        /* Reset application values */
        m_stPlayer->vlcPlayer = nullptr;
        slider->setValue(0);
        playBut->setText("Play");
    }
}

void Mwindow::mute() {
    if(m_stPlayer->vlcPlayer) {
        if(volumeSlider->value() == 0) { //if already muted...

                this->changeVolume(80);
                volumeSlider->setValue(80);

        } else { //else mute volume

                this->changeVolume(0);
                volumeSlider->setValue(0);

        }
    }
}

void Mwindow::about()
{
    QMessageBox::about(this, "Qt libVLC player demo", QString::fromUtf8(libvlc_get_version()) );
}

void Mwindow::fullscreen()
{
   if (isFullScreen()) {
       showNormal();
       //menuWidget()->show();
   }
   else {
       showFullScreen();
       //menuWidget()->hide();
   }
}

void Mwindow::closeEvent(QCloseEvent *event) {
    stop();
    event->accept();
}
