#pragma once
#include <QListWidget>

class QWheelEvent;

class ThumbnailPanel : public QListWidget {
    Q_OBJECT
public:
    explicit ThumbnailPanel(QWidget *parent = nullptr);

    void addThumb(int pageIndex, const QPixmap &thumb);
    void clear();
    void setCurrentPage(int idx);

signals:
    void pageSelected(int idx);

protected:
    void wheelEvent(QWheelEvent *e) override;

private:
    static constexpr int THUMB_W = 100;
    static constexpr int THUMB_H = 130;
    static constexpr int WHEEL_PIXELS_PER_STEP = 80; // default ~160 px (one row)
};
