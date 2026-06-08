#include "PageView.h"
#include <QScrollBar>
#include <QResizeEvent>

PageView::PageView(QWidget *parent) : QScrollArea(parent) {
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setWidget(m_label);
    setWidgetResizable(false);
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background: #2b2b2b;");
}

void PageView::setPage(const QPixmap &px) {
    m_original = px;
    m_fitMode  = true;
    applyZoom();
}

void PageView::setZoom(double z) {
    m_zoom    = qBound(0.1, z, 5.0);
    m_fitMode = false;
    applyZoom();
    emit zoomChanged(m_zoom);
}

void PageView::fitToWidth(int) {
    m_fitMode = true;
    applyZoom();
}

void PageView::applyZoom() {
    if (m_original.isNull()) return;
    if (m_fitMode) {
        int vw = viewport()->width() - 16;
        m_zoom = vw / double(m_original.width());
        m_zoom = qBound(0.05, m_zoom, 5.0);
    }
    QSize sz = m_original.size() * m_zoom;
    m_label->setPixmap(m_original.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_label->resize(sz);
}

void PageView::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        double delta = e->angleDelta().y() > 0 ? 1.12 : (1.0 / 1.12);
        setZoom(m_zoom * delta);
        e->accept();
    } else {
        QScrollArea::wheelEvent(e);
    }
}

void PageView::resizeEvent(QResizeEvent *e) {
    QScrollArea::resizeEvent(e);
    if (m_fitMode) applyZoom();
}
