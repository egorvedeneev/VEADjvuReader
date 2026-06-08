#include "ThumbnailPanel.h"
#include <QListWidgetItem>
#include <QScrollBar>
#include <QWheelEvent>

ThumbnailPanel::ThumbnailPanel(QWidget *parent) : QListWidget(parent) {
    setViewMode(QListView::ListMode);
    setFlow(QListView::TopToBottom);
    setIconSize({THUMB_W, THUMB_H});
    setGridSize({THUMB_W + 16, THUMB_H + 30});
    setFixedWidth(THUMB_W + 32);
    setSpacing(4);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setStyleSheet(R"(
        QListWidget {
            background: #1e1e1e;
            border: none;
            padding: 8px 0;
        }
        QListWidget::item {
            color: #aaa;
            font-size: 11px;
            padding: 4px 0;
            border-radius: 6px;
            margin: 2px 6px;
        }
        QListWidget::item:selected {
            background: #3a3a3a;
            color: #fff;
        }
        QListWidget::item:hover:!selected {
            background: #2e2e2e;
        }
    )");
    connect(this, &QListWidget::currentRowChanged,
            this, &ThumbnailPanel::pageSelected);
}

void ThumbnailPanel::addThumb(int pageIndex, const QPixmap &thumb) {
    QPixmap scaled = thumb.scaled(THUMB_W, THUMB_H,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    auto *item = new QListWidgetItem(QIcon(scaled),
                                    QString::number(pageIndex + 1));
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    addItem(item);
}

void ThumbnailPanel::clear() { QListWidget::clear(); }

void ThumbnailPanel::setCurrentPage(int idx) {
    if (idx >= 0 && idx < count()) {
        blockSignals(true);
        setCurrentRow(idx);
        blockSignals(false);
    }
}

void ThumbnailPanel::wheelEvent(QWheelEvent *e) {
    const QPoint pixelDelta = e->pixelDelta();
    int dy = 0;
    if (!pixelDelta.isNull()) {
        dy = pixelDelta.y() * WHEEL_PIXELS_PER_STEP / 160;
    } else {
        dy = e->angleDelta().y() * WHEEL_PIXELS_PER_STEP / 120;
    }
    if (dy != 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
        e->accept();
        return;
    }
    QListWidget::wheelEvent(e);
}
