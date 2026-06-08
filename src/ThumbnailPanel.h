#pragma once
#include <QListWidget>

class ThumbnailPanel : public QListWidget {
    Q_OBJECT
public:
    explicit ThumbnailPanel(QWidget *parent = nullptr);

    void addThumb(int pageIndex, const QPixmap &thumb);
    void clear();
    void setCurrentPage(int idx);

signals:
    void pageSelected(int idx);

private:
    static constexpr int THUMB_W = 100;
    static constexpr int THUMB_H = 130;
};
