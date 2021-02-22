#include "mainwindow.h"

#include <QApplication>

//#include "vlc_on_qt.h"

#if 0

#include <stdio.h>
#include <stdlib.h>

typedef signed __int64 ssize_t;


#include <vlc/vlc.h>

#include <QThread>

#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();



    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    /* Load the VLC engine */
    inst = libvlc_new (0, NULL);

    /* Create a new item */
    try{
    //m = libvlc_media_new_location (inst, "http://mycool.movie.com/test.mov");
    m = libvlc_media_new_path (inst, "d:\\test.mp4");
    //m = libvlc_media_new_path (inst, "/path/to/test.mov");
    }catch(...)
    {
        qDebug() << "err" ;
    }

    /* Create a media player playing environement */
    mp = libvlc_media_player_new_from_media (m);

    /* No need to keep the media now */
    libvlc_media_release (m);


#if 0
    /* This is a non working code that show how to hooks into a window,
     * if we have a window around */
     libvlc_media_player_set_xwindow (mp, xid);
    /* or on windows */
     libvlc_media_player_set_hwnd (mp, hwnd);
    /* or on mac os */
     libvlc_media_player_set_nsobject (mp, view);
 #endif

    /* play the media_player */
    libvlc_media_player_play (mp);

    QThread::msleep(5000); /* Let it play a bit */

    /* Stop playing */
    libvlc_media_player_stop (mp);

    /* Free the media_player */
    libvlc_media_player_release (mp);

    libvlc_release (inst);

    return 0;
}

#else

#include "player.h"
#include "videosnapshot.h"

#include <QFileDialog>
int main(int argc, char *argv[])
{


    QApplication a(argc, argv);

    QString fileOpen = QFileDialog::getOpenFileName(nullptr, "Load a file", "~");


    VideoSnapshot::snapshot(fileOpen,"D:/a.jpg");

    return a.exec();

    Mwindow player;
    player.resize(600,480);
    player.show();


    player.openFile(fileOpen);
    return a.exec();
}

#endif
