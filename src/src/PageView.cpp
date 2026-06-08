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

void PageView::setPage(const QPixmap &px, double zoomAtRender) {
    m_page = px;
    m_baseZoom = zoomAtRender;
    updateDisplay();
}

void PageView::setZoom(double z) {
    m_zoom = qBound(0.1, z, 5.0);
    m_fitMode = false;
    showPreview();
    emit zoomChanged(m_zoom);
}

void PageView::fitToWidth(int) {
    m_fitMode = true;
    m_zoom = 1.0;
    emit zoomChanged(m_zoom);
    emit viewChanged();
}

void PageView::updateDisplay() {
    if (m_page.isNull()) return;
    m_label->setPixmap(m_page);
    m_label->resize(m_page.size());
}

void PageView::showPreview() {
    if (m_page.isNull() || m_baseZoom <= 0) return;
    const double scale = m_zoom / m_baseZoom;
    if (qAbs(scale - 1.0) < 0.001) {
        updateDisplay();
        return;
    }
    const QSize sz = m_page.size() * scale;
    m_label->setPixmap(m_page.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_label->resize(sz);
}

void PageView::wheelEvent(QWheelEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const double delta = e->angleDelta().y() > 0 ? 1.12 : (1.0 / 1.12);
        setZoom(m_zoom * delta);
        e->accept();
    } else {
        QScrollArea::wheelEvent(e);
    }
}

void PageView::resizeEvent(QResizeEvent *e) {
    QScrollArea::resizeEvent(e);
    if (m_fitMode) emit viewChanged();
}
