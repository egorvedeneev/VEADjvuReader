#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QToolBar>
#include <QDockWidget>
#include <memory>

class DjVuDocument;
class PageView;
class ThumbnailPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openFile(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void onOpenFile();
    void onDocumentOpened(int pageCount);
    void onPageSelected(int idx);
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onPrevPage();
    void onNextPage();
    void onPageSpinChanged(int val);

private:
    void setupUi();
    void setupActions();
    void setupStyle();
    void goToPage(int idx);
    void renderCurrentPage();
    void loadThumbnails();

    std::unique_ptr<DjVuDocument> m_doc;
    PageView      *m_pageView      = nullptr;
    ThumbnailPanel *m_thumbPanel   = nullptr;
    QToolBar      *m_toolbar       = nullptr;
    QLabel        *m_statusLabel   = nullptr;
    QLabel        *m_zoomLabel     = nullptr;
    QSpinBox      *m_pageSpinBox   = nullptr;
    QLabel        *m_pageCountLabel = nullptr;

    int    m_currentPage = 0;
    double m_zoom        = 1.0;
};
