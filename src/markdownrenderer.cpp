#include "markdownrenderer.h"

#include <QDebug>

MarkdownRenderer::MarkdownRenderer()
{
    // if nesting = 0, no text is being renderered
    renderer =  hoedown_html_renderer_new(static_cast<hoedown_html_flags>(0), 16); // render flags, nesting (toc depth)
    document = hoedown_document_new(renderer, static_cast<hoedown_extensions>(0), 16); // extensions, nesting
}

MarkdownRenderer::~MarkdownRenderer()
{
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
}

QString MarkdownRenderer::convert(const QString& input)
{
    hoedown_buffer* html = hoedown_buffer_new(64);
    QByteArray data = input.toUtf8();
    hoedown_document_render(document, html, reinterpret_cast<const uint8_t*>(data.data()), data.length());
    QString result = QString::fromUtf8(hoedown_buffer_cstr(html)).trimmed();
    hoedown_buffer_free(html);
    return result;
}
