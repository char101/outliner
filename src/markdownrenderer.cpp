#include "markdownrenderer.h"

#include <QDebug>

static const int html_flags = HOEDOWN_HTML_USE_XHTML;
static const int enabled_exts = HOEDOWN_EXT_TABLES | HOEDOWN_EXT_FENCED_CODE | HOEDOWN_EXT_FOOTNOTES |
                                HOEDOWN_EXT_AUTOLINK | HOEDOWN_EXT_STRIKETHROUGH | HOEDOWN_EXT_UNDERLINE |
                                HOEDOWN_EXT_HIGHLIGHT | HOEDOWN_EXT_QUOTE | HOEDOWN_EXT_SUPERSCRIPT | HOEDOWN_EXT_MATH;

MarkdownRenderer::MarkdownRenderer()
{

    // if nesting = 0, no text is being renderered
    renderer = hoedown_html_renderer_new(static_cast<hoedown_html_flags>(html_flags), 16); // render flags, nesting (toc depth)
    document = hoedown_document_new(renderer, static_cast<hoedown_extensions>(enabled_exts), 16); // extensions, nesting
}

MarkdownRenderer::~MarkdownRenderer()
{
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
}

QString MarkdownRenderer::convert(const QString& input) const
{
    hoedown_buffer* html = hoedown_buffer_new(64);
    QByteArray data = input.toUtf8();
    hoedown_document_render(document, html, reinterpret_cast<const uint8_t*>(data.data()), data.length());
    QString result = QString::fromUtf8(hoedown_buffer_cstr(html)).trimmed();
    hoedown_buffer_free(html);
    return result;
}
