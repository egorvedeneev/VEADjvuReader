#pragma once
#include <QObject>
#include <QImage>
#include <QString>
#include <QSize>
#include <memory>

#ifndef DJVU_STUB_MODE
#  include <libdjvu/ddjvuapi.h>
#endif

class DjVuDocument : public QObject {
    Q_OBJECT
public:
    explicit DjVuDocument(QObject *parent = nullptr);
    ~DjVuDocument() override;

    bool   open(const QString &path);
    void   close();
    bool   isOpen() const { return m_open; }

    int    pageCount()  const { return m_pageCount; }
    QString filePath()  const { return m_filePath; }

    // Рендерит страницу в QImage с заданной шириной (высота пропорциональна)
    QImage renderPage(int pageIndex, int targetWidth, double zoom = 1.0);
    QSize  pageSize(int pageIndex) const;

signals:
    void documentOpened(int pageCount);
    void documentClosed();
    void error(const QString &msg);

private:
#ifndef DJVU_STUB_MODE
    ddjvu_context_t  *m_ctx  = nullptr;
    ddjvu_document_t *m_doc  = nullptr;
    void handleMessages();
#endif
    bool    m_open      = false;
    int     m_pageCount = 0;
    QString m_filePath;
};
