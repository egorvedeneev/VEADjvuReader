#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QToolBar>
#include <QDockWidget>
#include <QTimer>
#include <memory>

class QAction;
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
    void scheduleRender();
    void loadThumbnails();
    void setDocumentActionsEnabled(bool enabled);

    std::unique_ptr<DjVuDocument> m_doc;
    PageView      *m_pageView      = nullptr;
    ThumbnailPanel *m_thumbPanel   = nullptr;
    QToolBar      *m_toolbar       = nullptr;
    QLabel        *m_statusLabel   = nullptr;
    QLabel        *m_zoomLabel     = nullptr;
    QSpinBox      *m_pageSpinBox   = nullptr;
    QLabel        *m_pageCountLabel = nullptr;
    QAction       *m_actPrev        = nullptr;
    QAction       *m_actNext        = nullptr;
    QAction       *m_actZoomIn      = nullptr;
    QAction       *m_actZoomOut     = nullptr;
    QAction       *m_actFit         = nullptr;
    QAction       *m_actPrevTb      = nullptr;
    QAction       *m_actNextTb      = nullptr;
    QTimer         m_renderTimer;

    int    m_currentPage = 0;
    double m_zoom        = 1.0;
};
