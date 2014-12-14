#pragma once

#include <QString>

#include "html.h"
#include "document.h"
#include "buffer.h"

class MarkdownRenderer
{
public:
    MarkdownRenderer();
    ~MarkdownRenderer();
    QString convert(const QString& input) const;
private:
    hoedown_renderer* renderer{nullptr};
    hoedown_document* document{nullptr};
};
