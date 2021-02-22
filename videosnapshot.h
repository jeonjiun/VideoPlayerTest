#ifndef VIDEOSNAPSHOT_H
#define VIDEOSNAPSHOT_H

#include <QString>


#define THUMBNAIL_POSITION (30.F/100.F)


class VideoSnapshotPrivate;

class VideoSnapshot
{
public:
    VideoSnapshot();
    ~VideoSnapshot();

    bool open(const QString &filename);
    bool snapshot(const QString &outFilename, float pos = THUMBNAIL_POSITION);

    static bool snapshot(const QString &videoPath, const QString &outImage, float pos = THUMBNAIL_POSITION);

private:
    VideoSnapshotPrivate *m_private;
};

#endif // VIDEOSNAPSHOT_H
