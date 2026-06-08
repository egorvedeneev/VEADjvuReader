#pragma once
#include <QScrollArea>
#include <QLabel>
#include <QWheelEvent>
#include <QPixmap>

class PageView : public QScrollArea {
    Q_OBJECT
public:
    explicit PageView(QWidget *parent = nullptr);

    void setPage(const QPixmap &px);
    void setZoom(double zoom);
    double zoom() const { return m_zoom; }
    void fitToWidth(int viewportWidth);

signals:
    void zoomChanged(double zoom);

protected:
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    QLabel  *m_label;
    QPixmap  m_original;
    double   m_zoom    = 1.0;
    bool     m_fitMode = true;

    void applyZoom();
};
