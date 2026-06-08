#include "MainWindow.h"
#include "DjVuDocument.h"
#include "PageView.h"
#include "ThumbnailPanel.h"

#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QSpinBox>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_doc(std::make_unique<DjVuDocument>(this))
{
    setupUi();
    setupActions();
    setupStyle();
    setAcceptDrops(true);

    connect(m_doc.get(), &DjVuDocument::documentOpened,
            this, &MainWindow::onDocumentOpened);
    connect(m_doc.get(), &DjVuDocument::error,
            this, [this](const QString &msg){
                m_statusLabel->setText("Error: " + msg);
                QMessageBox::warning(this, "Error", msg);
            });
}

MainWindow::~MainWindow() {}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupUi() {
    setWindowTitle("DjVu Reader");
    resize(1100, 780);

    // Central page view
    m_pageView = new PageView(this);
    setCentralWidget(m_pageView);
    connect(m_pageView, &PageView::zoomChanged, this, [this](double z){
        m_zoom = z;
        m_zoomLabel->setText(QString("%1%").arg(int(z * 100)));
    });

    // Thumbnail dock
    m_thumbPanel = new ThumbnailPanel(this);
    auto *dock = new QDockWidget("Pages", this);
    dock->setWidget(m_thumbPanel);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    connect(m_thumbPanel, &ThumbnailPanel::pageSelected,
            this, &MainWindow::onPageSelected);

    // Toolbar widgets
    m_toolbar = addToolBar("Main");
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize({20, 20});

    // Status bar
    m_statusLabel = new QLabel("Open a DjVu file to start", this);
    statusBar()->addPermanentWidget(m_statusLabel, 1);

    m_zoomLabel = new QLabel("100%", this);
    m_zoomLabel->setMinimumWidth(48);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(m_zoomLabel);
}

void MainWindow::setupActions() {
    // ── File menu ──────────────────────────────────────────────────────────
    auto *fileMenu  = menuBar()->addMenu("&File");
    auto *actOpen   = fileMenu->addAction("&Open…", this, &MainWindow::onOpenFile,
                                          QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", qApp, &QApplication::quit, QKeySequence::Quit);

    // ── View menu ──────────────────────────────────────────────────────────
    auto *viewMenu  = menuBar()->addMenu("&View");
    auto *actZoomIn  = viewMenu->addAction("Zoom &In",  this, &MainWindow::onZoomIn,  QKeySequence::ZoomIn);
    auto *actZoomOut = viewMenu->addAction("Zoom &Out", this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    auto *actFit     = viewMenu->addAction("&Fit Width", this, &MainWindow::onZoomFit, Qt::Key_F);
    viewMenu->addSeparator();
    auto *actPrev    = viewMenu->addAction("&Previous Page", this, &MainWindow::onPrevPage, Qt::Key_Left);
    auto *actNext    = viewMenu->addAction("&Next Page",     this, &MainWindow::onNextPage,  Qt::Key_Right);

    // ── Toolbar ────────────────────────────────────────────────────────────
    m_toolbar->addAction(actOpen);
    m_toolbar->addSeparator();

    auto *actPrevTb = m_toolbar->addAction("◀", this, &MainWindow::onPrevPage);
    actPrevTb->setToolTip("Previous page (←)");

    // Page spin
    m_pageSpinBox = new QSpinBox(this);
    m_pageSpinBox->setMinimum(1);
    m_pageSpinBox->setMaximum(1);
    m_pageSpinBox->setMinimumWidth(52);
    m_pageSpinBox->setAlignment(Qt::AlignCenter);
    m_toolbar->addWidget(m_pageSpinBox);

    m_pageCountLabel = new QLabel(" / 0", this);
    m_toolbar->addWidget(m_pageCountLabel);

    auto *actNextTb = m_toolbar->addAction("▶", this, &MainWindow::onNextPage);
    actNextTb->setToolTip("Next page (→)");

    m_toolbar->addSeparator();
    m_toolbar->addAction("−", this, &MainWindow::onZoomOut)->setToolTip("Zoom out (Ctrl+−)");
    m_toolbar->addAction("+", this, &MainWindow::onZoomIn)->setToolTip("Zoom in (Ctrl++)");
    m_toolbar->addAction("Fit", this, &MainWindow::onZoomFit)->setToolTip("Fit to width (F)");

    connect(m_pageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onPageSpinChanged);

    // Disable nav until doc open
    for (auto *a : {actPrev, actNext, actZoomIn, actZoomOut, actFit})
        a->setEnabled(false);
    actPrevTb->setEnabled(false);
    actNextTb->setEnabled(false);
}

void MainWindow::setupStyle() {
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #252525; color: #ddd; }
        QMenuBar { background: #1a1a1a; color: #ccc; padding: 2px; }
        QMenuBar::item:selected { background: #3a3a3a; border-radius: 4px; }
        QMenu { background: #2a2a2a; border: 1px solid #444; color: #ccc; }
        QMenu::item:selected { background: #404040; }
        QToolBar {
            background: #1a1a1a;
            border-bottom: 1px solid #333;
            spacing: 4px;
            padding: 4px 8px;
        }
        QToolBar QToolButton {
            background: transparent;
            color: #ccc;
            border: 1px solid transparent;
            border-radius: 5px;
            padding: 4px 8px;
            font-size: 13px;
        }
        QToolBar QToolButton:hover  { background: #3a3a3a; border-color: #555; }
        QToolBar QToolButton:pressed { background: #444; }
        QSpinBox {
            background: #333;
            color: #ddd;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 2px 4px;
        }
        QStatusBar { background: #1a1a1a; color: #888; font-size: 12px; padding: 2px 8px; }
        QDockWidget { color: #ccc; }
        QDockWidget::title {
            background: #1a1a1a;
            padding: 6px;
            font-size: 12px;
            letter-spacing: 0.5px;
        }
        QScrollBar:vertical, QScrollBar:horizontal {
            background: #2a2a2a; width: 8px; height: 8px;
        }
        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: #555; border-radius: 4px; min-height: 24px;
        }
        QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
    )");
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onOpenFile() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open DjVu File", {},
        "DjVu Files (*.djvu *.djv);;All Files (*)");
    if (!path.isEmpty()) openFile(path);
}

void MainWindow::openFile(const QString &path) {
    m_thumbPanel->clear();
    m_doc->open(path);
}

void MainWindow::onDocumentOpened(int pageCount) {
    setWindowTitle(QFileInfo(m_doc->filePath()).fileName() + " — DjVu Reader");
    m_pageSpinBox->setMaximum(pageCount);
    m_pageCountLabel->setText(QString(" / %1").arg(pageCount));
    m_currentPage = 0;

    loadThumbnails();
    goToPage(0);

    m_statusLabel->setText(
        QString("Opened: %1  (%2 pages)")
        .arg(QFileInfo(m_doc->filePath()).fileName())
        .arg(pageCount));
}

void MainWindow::loadThumbnails() {
    m_thumbPanel->clear();
    int n = m_doc->pageCount();
    for (int i = 0; i < n; ++i) {
        QImage img = m_doc->renderPage(i, 100);
        m_thumbPanel->addThumb(i, QPixmap::fromImage(img));
        QApplication::processEvents(); // не замораживаем UI
    }
}

void MainWindow::goToPage(int idx) {
    if (!m_doc->isOpen()) return;
    idx = qBound(0, idx, m_doc->pageCount() - 1);
    m_currentPage = idx;
    renderCurrentPage();
    m_thumbPanel->setCurrentPage(idx);
    m_pageSpinBox->blockSignals(true);
    m_pageSpinBox->setValue(idx + 1);
    m_pageSpinBox->blockSignals(false);
}

void MainWindow::renderCurrentPage() {
    int vw = m_pageView->viewport()->width();
    QImage img = m_doc->renderPage(m_currentPage, qMax(vw, 400));
    m_pageView->setPage(QPixmap::fromImage(img));
}

void MainWindow::onPageSelected(int idx) { goToPage(idx); }
void MainWindow::onPrevPage()  { goToPage(m_currentPage - 1); }
void MainWindow::onNextPage()  { goToPage(m_currentPage + 1); }
void MainWindow::onZoomIn()    { m_pageView->setZoom(m_pageView->zoom() * 1.2); }
void MainWindow::onZoomOut()   { m_pageView->setZoom(m_pageView->zoom() / 1.2); }
void MainWindow::onZoomFit()   { m_pageView->fitToWidth(0); }
void MainWindow::onPageSpinChanged(int val) { goToPage(val - 1); }

void MainWindow::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Left:  case Qt::Key_PageUp:   onPrevPage(); break;
    case Qt::Key_Right: case Qt::Key_PageDown: onNextPage(); break;
    case Qt::Key_Home:  goToPage(0); break;
    case Qt::Key_End:   goToPage(m_doc->pageCount() - 1); break;
    default: QMainWindow::keyPressEvent(e);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e) {
    const auto urls = e->mimeData()->urls();
    if (!urls.isEmpty()) openFile(urls.first().toLocalFile());
}
