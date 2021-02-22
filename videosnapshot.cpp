#include "videosnapshot.h"

typedef signed __int64 ssize_t;

#include <vlc/vlc.h>

#include <QMutex>
#include <QDeadlineTimer>
#include <QWaitCondition>

class VideoSnapshotPrivate
{
public:
    VideoSnapshotPrivate()
    {
        static const char* const args[] = {
            "--intf", "dummy",                  /* no interface                   */
            "--vout", "dummy",                  /* we don't want video (output)   */
            "--no-audio",                       /* we don't want audio (decoding) */
            "--no-video-title-show",            /* nor the filename displayed     */
            "--no-stats",                       /* no stats                       */
            "--no-sub-autodetect-file",         /* we don't want subtitles        */
            "--no-inhibit",                     /* we don't want interfaces       */
            "--no-disable-screensaver",         /* we don't want interfaces       */
            "--no-snapshot-preview",            /* no blending in dummy vout      */
    #ifndef NDEBUG
            "--verbose=2",                      /* full log                       */
    #endif
        };

        //return

        /* Initialize libVLC */
        vlcInstance = libvlc_new(sizeof args / sizeof *args, args);
    }
    ~VideoSnapshotPrivate()
    {
        clearPlayer();
        clearMedia();
        clearInstance();
    }
public:

    void clearInstance()
    {
        if(vlcInstance)
        {
            libvlc_release(vlcInstance);
            vlcInstance = nullptr;
        }
    }
    void clearPlayer()
    {
        if(vlcPlayer)
        {
            libvlc_media_player_release(vlcPlayer);
            vlcPlayer = nullptr;
        }
    }
    void clearMedia()
    {
        if(vlcMedia)
        {
            libvlc_media_release(vlcMedia);
            vlcMedia = nullptr;
        }
    }


    static void callback(const libvlc_event_t *ev, void *param)
    {
        float new_position;
        VideoSnapshotPrivate *_this = static_cast<VideoSnapshotPrivate*>(param);

        _this->mutex.lock();
        switch (ev->type) {
        case libvlc_MediaPlayerPositionChanged:
            new_position = ev->u.media_player_position_changed.new_position;
            if (new_position < THUMBNAIL_POSITION * .9F /* 90% margin */)
                break;
            _this->done = true;
            _this->wait.wakeAll();
            break;
        case libvlc_MediaPlayerSnapshotTaken:
            _this->done = true;
            _this->wait.wakeAll();
            break;

        default:
            assert(0);
        }

        _this->mutex.unlock();
    }

    bool event_wait(const char *)
    {
        bool ret;
        mutex.lock();
        ret = done ? true : wait.wait(&mutex, QDeadlineTimer(10000));
        mutex.unlock();

        return ret;
    }

    void set_position(libvlc_media_player_t *mp, float pos)
    {
        libvlc_event_manager_t *em = libvlc_media_player_event_manager(mp);
        assert(em);

        libvlc_event_attach(em, libvlc_MediaPlayerPositionChanged, callback, this);
        done = false;
        libvlc_media_player_set_position(mp, pos);

        event_wait("Couldn't set position");
        libvlc_event_detach(em, libvlc_MediaPlayerPositionChanged, callback, this);
    }

    void snapshot(libvlc_media_player_t *mp, unsigned int width, const char *out_with_ext)
    {
        libvlc_event_manager_t *em = libvlc_media_player_event_manager(mp);
        assert(em);

        libvlc_event_attach(em, libvlc_MediaPlayerSnapshotTaken, callback, this);
        done = false;
        libvlc_video_take_snapshot(vlcPlayer, 0, out_with_ext, width, 0);

        event_wait("Snapshot has not been written");
        libvlc_event_detach(em, libvlc_MediaPlayerSnapshotTaken, callback, this);
    }

public:
    libvlc_instance_t *vlcInstance = nullptr;
    libvlc_media_player_t *vlcPlayer = nullptr;
    libvlc_media_t *vlcMedia = nullptr;

    bool   done;
    QMutex mutex;
    QWaitCondition wait;
};

VideoSnapshot::VideoSnapshot()
{
    m_private = new VideoSnapshotPrivate;
}

VideoSnapshot::~VideoSnapshot()
{
    if(m_private)
        delete m_private;
}

bool VideoSnapshot::open(const QString &filename)
{
    QString fileOpen = filename;
#if defined(Q_OS_WIN)
    fileOpen.replace("/","\\");
#endif

    /* Complain in case of broken installation */
    if (m_private->vlcInstance == nullptr)
        return false;

    m_private->clearMedia();

    m_private->vlcMedia = libvlc_media_new_path(m_private->vlcInstance, fileOpen.toUtf8().constData());

    if (m_private->vlcMedia == nullptr)
        return false;

    return true;
}


bool VideoSnapshot::snapshot(const QString &outFilename, float pos)
{
    if(m_private->vlcMedia == nullptr)
        return false;

    QString fileOut = outFilename;
#if defined(Q_OS_WIN)
    fileOut.replace("/","\\");
#endif

    m_private->vlcPlayer = libvlc_media_player_new_from_media(m_private->vlcMedia);

    libvlc_media_player_play(m_private->vlcPlayer);

    m_private->set_position(m_private->vlcPlayer,pos);

    m_private->snapshot(m_private->vlcPlayer,0,fileOut.toUtf8().constData());

    return true;
}

bool VideoSnapshot::snapshot(const QString &videoPath, const QString &outImage, float pos)
{
    VideoSnapshot s;

    if (!s.open(videoPath))
        return false;

    if (!s.snapshot(outImage,pos))
        return false;

    return true;
}
