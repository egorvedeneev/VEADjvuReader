#include "DjVuDocument.h"
#include <QPainter>
#include <QFileInfo>
#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
//  STUB MODE: если libdjvulibre не установлена — рисуем заглушку
// ─────────────────────────────────────────────────────────────────────────────
#ifdef DJVU_STUB_MODE

DjVuDocument::DjVuDocument(QObject *parent) : QObject(parent) {}
DjVuDocument::~DjVuDocument() {}

bool DjVuDocument::open(const QString &path) {
    QFileInfo fi(path);
    if (!fi.exists()) { emit error("File not found: " + path); return false; }
    m_filePath  = path;
    m_pageCount = 5;   // stub: притворяемся что 5 страниц
    m_open      = true;
    emit documentOpened(m_pageCount);
    return true;
}

void DjVuDocument::close() {
    m_open = false; m_pageCount = 0; m_filePath.clear();
    emit documentClosed();
}

QSize DjVuDocument::pageSize(int) const { return {595, 842}; } // A4

QImage DjVuDocument::renderPage(int pageIndex, int targetWidth, double zoom) {
    QSize sz = pageSize(pageIndex);
    int w = qMax(1, int(targetWidth * zoom));
    int h = qMax(1, int(sz.height() * w / double(sz.width())));
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setPen(QPen(QColor(200,200,200), 1));
    p.drawRect(0, 0, w - 1, h - 1);
    p.setPen(QColor(160,160,160));
    QFont f; f.setPixelSize(18); p.setFont(f);
    p.drawText(img.rect(), Qt::AlignCenter,
               QString("Page %1\n(stub — install libdjvulibre)").arg(pageIndex + 1));
    return img;
}

// ─────────────────────────────────────────────────────────────────────────────
//  REAL MODE: libdjvulibre
// ─────────────────────────────────────────────────────────────────────────────
#else

DjVuDocument::DjVuDocument(QObject *parent) : QObject(parent) {
    m_ctx = ddjvu_context_create("DjVuReader");
}

DjVuDocument::~DjVuDocument() { close(); if (m_ctx) ddjvu_context_release(m_ctx); }

void DjVuDocument::handleMessages() {
    // Сливаем очередь сообщений синхронно
    const ddjvu_message_t *msg;
    while ((msg = ddjvu_message_peek(m_ctx)) != nullptr) {
        if (msg->m_any.tag == DDJVU_ERROR)
            emit error(QString::fromUtf8(msg->m_error.message));
        ddjvu_message_pop(m_ctx);
    }
}

bool DjVuDocument::open(const QString &path) {
    close();
    QByteArray utf8 = path.toUtf8();
    m_doc = ddjvu_document_create_by_filename_utf8(m_ctx, utf8.constData(), TRUE);
    if (!m_doc) { emit error("Cannot open: " + path); return false; }

    // Ждём завершения декодирования заголовка
    ddjvu_message_t *msg;
    while (!ddjvu_document_decoding_done(m_doc)) {
        msg = ddjvu_message_wait(m_ctx);
        if (msg && msg->m_any.tag == DDJVU_ERROR) {
            emit error(QString::fromUtf8(msg->m_error.message));
            ddjvu_document_release(m_doc); m_doc = nullptr; return false;
        }
        ddjvu_message_pop(m_ctx);
    }

    m_pageCount = ddjvu_document_get_pagenum(m_doc);
    m_filePath  = path;
    m_open      = true;
    emit documentOpened(m_pageCount);
    return true;
}

void DjVuDocument::close() {
    if (m_doc) { ddjvu_document_release(m_doc); m_doc = nullptr; }
    m_open = false; m_pageCount = 0; m_filePath.clear();
    emit documentClosed();
}

QSize DjVuDocument::pageSize(int idx) const {
    if (!m_doc || idx < 0 || idx >= m_pageCount) return {595, 842};
    ddjvu_pageinfo_t info{};
    ddjvu_document_get_pageinfo(m_doc, idx, &info);
    return {info.width, info.height};
}

QImage DjVuDocument::renderPage(int pageIndex, int targetWidth, double zoom) {
    if (!m_doc || pageIndex < 0 || pageIndex >= m_pageCount) return {};

    ddjvu_page_t *page = ddjvu_page_create_by_pageno(m_doc, pageIndex);
    if (!page) return {};

    // Ждём декодирования страницы
    ddjvu_message_t *msg;
    while (!ddjvu_page_decoding_done(page)) {
        msg = ddjvu_message_wait(m_ctx);
        if (msg && msg->m_any.tag == DDJVU_ERROR) {
            ddjvu_page_release(page); return {};
        }
        ddjvu_message_pop(m_ctx);
    }

    int origW = ddjvu_page_get_width(page);
    int origH = ddjvu_page_get_height(page);
    if (origW <= 0 || origH <= 0) { ddjvu_page_release(page); return {}; }

    double scale = (zoom * targetWidth) / double(origW);
    int w = int(origW * scale);
    int h = int(origH * scale);

    ddjvu_rect_t prect = {0, 0, (unsigned)w, (unsigned)h};
    ddjvu_rect_t rrect = prect;

    // RGB формат (3 байта на пиксель)
    ddjvu_format_t *fmt = ddjvu_format_create(DDJVU_FORMAT_BGR24, 0, nullptr);
    ddjvu_format_set_row_order(fmt, 1); // top-to-bottom

    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::white);

    // Рендерим через временный буфер BGR24
    QByteArray buf(w * h * 3, 0);
    int ok = ddjvu_page_render(page, DDJVU_RENDER_COLOR,
                               &prect, &rrect, fmt,
                               w * 3, buf.data());

    ddjvu_format_release(fmt);
    ddjvu_page_release(page);

    if (!ok) return img;

    // Конвертируем BGR24 → QImage RGB32
    for (int y = 0; y < h; ++y) {
        const uchar *src = reinterpret_cast<const uchar*>(buf.constData()) + y * w * 3;
        QRgb *dst = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            dst[x] = qRgb(src[x*3+2], src[x*3+1], src[x*3]);
        }
    }
    return img;
}

#endif // DJVU_STUB_MODE
