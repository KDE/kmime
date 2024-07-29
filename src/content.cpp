/*
    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the Content class.

  @brief
  Defines the Content class.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>
*/
#include "content.h"
#include "content_p.h"
#include "message.h"
#include "headerparsing.h"
#include "headerparsing_p.h"
#include "parsers_p.h"
#include "util_p.h"

#include <KCodecs>

#include <QStringDecoder>
#include <QStringEncoder>

using namespace KMime;

namespace KMime
{

Content::Content(Content *parent)
    : d_ptr(new ContentPrivate)
{
    d_ptr->parent = parent;
}

Content::~Content()
{
    Q_D(Content);
    qDeleteAll(d->headers);
    d->headers.clear();
    delete d_ptr;
    d_ptr = nullptr;
}

bool Content::hasContent() const
{
    return !d_ptr->head.isEmpty() || !d_ptr->body.isEmpty() || !d_ptr->contents().isEmpty();
}

void Content::setContent(const QByteArray &s)
{
    Q_D(Content);
    KMime::HeaderParsing::extractHeaderAndBody(s, d->head, d->body);
}

QByteArray Content::head() const
{
    return d_ptr->head;
}

void Content::setHead(const QByteArray &head)
{
    d_ptr->head = head;
    if (!head.endsWith('\n')) {
        d_ptr->head += '\n';
    }
}

QByteArray Content::body() const
{
    return d_ptr->body;
}

void Content::setBody(const QByteArray &body)
{
    d_ptr->body = body;
    d_ptr->m_decoded = true;
}

void Content::setEncodedBody(const QByteArray &body)
{
    d_ptr->body = body;
    d_ptr->m_decoded = false;
}

QByteArray Content::preamble() const
{
    return d_ptr->preamble;
}

void Content::setPreamble(const QByteArray &preamble)
{
    d_ptr->preamble = preamble;
}

QByteArray Content::epilogue() const
{
    return d_ptr->epilogue;
}

void Content::setEpilogue(const QByteArray &epilogue)
{
    d_ptr->epilogue = epilogue;
}

void Content::parse()
{
    Q_D(Content);

    // Clean up old headers and parse them again.
    qDeleteAll(d->headers);
    d->headers.clear();
    d->headers = HeaderParsing::parseHeaders(d->head);
    if (const auto cte = contentTransferEncoding(false); cte) {
        d->m_decoded = (cte->encoding() == Headers::CE7Bit || cte->encoding() == Headers::CE8Bit);
    }

    // If we are frozen, save the body as-is. This is done because parsing
    // changes the content (it loses preambles and epilogues, converts uuencode->mime, etc.)
    if (d->frozen) {
        d->frozenBody = d->body;
    }

    // Clean up old sub-Contents and parse them again.
    qDeleteAll(d->multipartContents);
    d->multipartContents.clear();
    d->clearBodyMessage();
    Headers::ContentType *ct = contentType();
    if (ct->isEmpty()) { //Set default content-type as defined in https://tools.ietf.org/html/rfc2045#page-10 (5.2.  Content-Type Defaults)
        ct->setMimeType("text/plain");
        ct->setCharset("us-ascii");
    }
    if (ct->isText()) {
        // This content is either text, or of unknown type.

        if (d->parseUuencoded(this)) {
            // This is actually uuencoded content generated by broken software.
        } else if (d->parseYenc(this)) {
            // This is actually yenc content generated by broken software.
        } else {
            // This is just plain text.
        }
    } else if (ct->isMultipart()) {
        // This content claims to be MIME multipart.

        if (d->parseMultipart(this)) {
            // This is actual MIME multipart content.
        } else {
            // Parsing failed; treat this content as "text/plain".
            ct->setMimeType("text/plain");
            ct->setCharset("US-ASCII");
        }
    } else {
        // This content is something else, like an encapsulated message or a binary attachment
        // or something like that
        if (bodyIsMessage()) {
            d->bodyAsMessage = Message::Ptr(new Message);
            d->bodyAsMessage->setContent(d->body);
            d->bodyAsMessage->setFrozen(d->frozen);
            d->bodyAsMessage->parse();
            d->bodyAsMessage->d_ptr->parent = this;

            // Clear the body, as it is now represented by d->bodyAsMessage. This is the same behavior
            // as with multipart contents, since parseMultipart() clears the body as well
            d->body.clear();
        }
    }
}

bool Content::isFrozen() const
{
    return d_ptr->frozen;
}

void Content::setFrozen(bool frozen)
{
    d_ptr->frozen = frozen;
}

void Content::assemble()
{
    Q_D(Content);
    if (d->frozen) {
        return;
    }

    d->head = assembleHeaders();
    const auto contentsList = contents();
    for (Content *c : contentsList) {
        c->assemble();
    }
}

QByteArray Content::assembleHeaders()
{
    Q_D(Content);
    QByteArray newHead;
    for (const Headers::Base *h : std::as_const(d->headers)) {
        if (!h->isEmpty()) {
            newHead += foldHeader(h->as7BitString()) + '\n';
        }
    }

    return newHead;
}

void Content::clear()
{
    Q_D(Content);
    qDeleteAll(d->headers);
    d->headers.clear();
    clearContents();
    d->head.clear();
    d->body.clear();
}

void Content::clearContents(bool del)
{
    Q_D(Content);
    if (del) {
        qDeleteAll(d->multipartContents);
    }
    d->multipartContents.clear();
    d->clearBodyMessage();
}

QByteArray Content::encodedContent(bool useCrLf) const
{
    QByteArray encodedContentData = head();           // return value; initialize with the head data
    const QByteArray encodedBodyData = encodedBody();

    /* Make sure that head and body have at least two newlines as separator, otherwise add one.
     * If we have enough newlines as separator, then we should not change the number of newlines
     * to not break digital signatures
     */
    if (!encodedContentData.endsWith("\n\n") &&
        !encodedBodyData.startsWith("\n\n") &&
        !(encodedContentData.endsWith("\n") && encodedBodyData.startsWith("\n"))){
        encodedContentData += '\n';
    }
    encodedContentData += encodedBodyData;

    if (useCrLf) {
        return LFtoCRLF(encodedContentData);
    } else {
        return encodedContentData;
    }
}

QByteArray Content::encodedBody() const
{
    Q_D(const Content);
    QByteArray e;
    // Body.
    if (d->frozen) {
        // This Content is frozen.
        if (d->frozenBody.isEmpty()) {
            // This Content has never been parsed.
            e += d->body;
        } else {
            // Use the body as it was before parsing.
            e += d->frozenBody;
        }
    } else if (bodyIsMessage() && d->bodyAsMessage) {
        // This is an encapsulated message
        // No encoding needed, as the ContentTransferEncoding can only be 7bit
        // for encapsulated messages
        e += d->bodyAsMessage->encodedContent();
    } else if (!d->body.isEmpty()) {
        // This is a single-part Content.
        const auto enc = contentTransferEncoding();

        if (enc && d->needToEncode(this)) {
            if (enc->encoding() == Headers::CEquPr) {
                e += KCodecs::quotedPrintableEncode(d->body, false);
            } else {
                QByteArray encoded;
                KCodecs::base64Encode(d->body, encoded, true);
                e += encoded;
                e += '\n';
            }
        } else {
            e += d->body;
        }
    }

    if (!d->frozen && !d->multipartContents.isEmpty()) {
        // This is a multipart Content.
        const auto ct = contentType();
        QByteArray boundary = "\n--" + (ct ? ct->boundary() : QByteArray());

        if (!d->preamble.isEmpty()) {
            e += d->preamble;
        }

        //add all (encoded) contents separated by boundaries
        for (Content *c : std::as_const(d->multipartContents)) {
            e += boundary + '\n';
            e += c->encodedContent(false);    // don't convert LFs here, we do that later!!!!!
        }
        //finally append the closing boundary
        e += boundary + "--\n";

        if (!d->epilogue.isEmpty()) {
            e += d->epilogue;
        }
    }
    return e;
}

QByteArray Content::decodedContent() const
{
    QByteArray ret;
    const Headers::ContentTransferEncoding *ec = contentTransferEncoding();
    bool removeTrailingNewline = false;

    if (d_ptr->body.isEmpty()) {
        return ret;
    }

    if (!ec || d_ptr->m_decoded) {
        ret = d_ptr->body;
        //Laurent Fix bug #311267
        //removeTrailingNewline = true;
    } else {
        switch (ec->encoding()) {
        case Headers::CEbase64 : {
            KCodecs::Codec *codec = KCodecs::Codec::codecForName("base64");
            Q_ASSERT(codec);
            ret.resize(codec->maxDecodedSizeFor(d_ptr->body.size()));
            QScopedPointer<KCodecs::Decoder> decoder(codec->makeDecoder());
            QByteArray::const_iterator inputIt = d_ptr->body.constBegin();
            QByteArray::iterator resultIt = ret.begin();
            decoder->decode(inputIt, d_ptr->body.constEnd(), resultIt, ret.constEnd());
            ret.truncate(resultIt - ret.begin());
            break;
        }
        case Headers::CEquPr :
            ret = KCodecs::quotedPrintableDecode(d_ptr->body);
            removeTrailingNewline = true;
            break;
        case Headers::CEuuenc :
            KCodecs::uudecode(d_ptr->body, ret);
            break;
        case Headers::CEbinary :
            ret = d_ptr->body;
            removeTrailingNewline = false;
            break;
        default :
            ret = d_ptr->body;
            removeTrailingNewline = true;
        }
    }

    if (removeTrailingNewline && (ret.size() > 0) && (ret[ret.size() - 1] == '\n')) {
        ret.resize(ret.size() - 1);
    }

    return ret;
}

QString Content::decodedText(bool trimText, bool removeTrailingNewlines) const
{
    if (!d_ptr->decodeText(this)) {   //this is not a text content !!
      return {};
    }

    QStringDecoder codec;
    if (const auto ct = contentType(); ct) {
        codec = QStringDecoder(ct->charset().constData());
    }
    if (!codec.isValid()) {   // no suitable codec found => try local settings and hope for the best ;-)
        codec = QStringDecoder(QStringDecoder::System);
    }

    QString s = codec.decode(d_ptr->body);

    if (trimText || removeTrailingNewlines) {
        qsizetype i;
        for (i = s.length() - 1; i >= 0; --i) {
            if (trimText) {
                if (!s[i].isSpace()) {
                    break;
                }
            } else {
                if (s[i] != QLatin1Char('\n')) {
                    break;
                }
            }
        }
        s.truncate(i + 1);
    } else {
        if (s.endsWith(QLatin1Char('\n'))) {
            s.chop(1);   // remove trailing new-line
        }
    }

    return s;
}

void Content::fromUnicodeString(const QString &s)
{
    QStringEncoder codec(contentType()->charset().constData());

    if (!codec.isValid()) {   // no suitable codec found => try local settings and hope for the best ;-)
        codec = QStringEncoder(QStringEncoder::System);
        QByteArray chset = codec.name();
        contentType()->setCharset(chset);
    }

    d_ptr->body = codec.encode(s);
    d_ptr->m_decoded = true;   //text is always decoded
}

Content *Content::textContent()
{
    // we start from a non-const this we know the result will be safely const_castable as well
    return const_cast<Content*>(static_cast<const Content*>(this)->textContent());
}

const Content *Content::textContent() const
{
    //return the first content with mimetype=text/*
    // see ContentType::isText, that's also true for an empty header
    if (const auto ct = contentType(); !ct || ct->isText()) {
        return this;
    }

    const auto contents = d_ptr->contents();
    for (const Content *c : contents) {
        if (auto ret = c->textContent()) {
            return ret;
        }
    }

    return nullptr;
}

QList<Content *> Content::attachments() const {
    QList<Content *> result;

    auto ct = contentType();
    if (ct && ct->isMultipart() &&
        !ct->isSubtype("related") /* && !ct->isSubtype("alternative")*/) {
        const QList<Content *> contentsList = contents();
        result.reserve(contentsList.count());
        for (Content *child : contentsList) {
            if (isAttachment(child)) {
                result.push_back(child);
            } else {
                result += child->attachments();
            }
      }
    }

    return result;
}

QList<Content *> Content::contents() const { return d_ptr->contents(); }

void Content::replaceContent(Content *oldContent, Content *newContent)
{
    Q_D( Content );
    if ( d->multipartContents.isEmpty() || !d->multipartContents.contains( oldContent ) ) {
      return;
    }

    d->multipartContents.removeAll( oldContent );
    delete oldContent;
    d->multipartContents.append( newContent );
    if( newContent->parent() != this ) {
      // If the content was part of something else, this will remove it from there.
      newContent->setParent( this );
    }
}


void Content::appendContent(Content *c)
{
    // This method makes no sense for encapsulated messages
    Q_ASSERT(!bodyIsMessage());

    Q_D(Content);
    d->multipartContents.append(c);

    if (c->parent() != this) {
        // If the content was part of something else, this will remove it from there.
        c->setParent(this);
    }
}

void Content::prependContent(Content *c)
{
    // This method makes no sense for encapsulated messages
    Q_ASSERT(!bodyIsMessage());

    Q_D(Content);
    d->multipartContents.prepend(c);

    if (c->parent() != this) {
        // If the content was part of something else, this will remove it from there.
        c->setParent(this);
    }
}

Content *Content::takeContent(Content *c)
{
    // This method makes no sense for encapsulated messages.
    // Should be covered by the above assert already, though.
    Q_ASSERT(!bodyIsMessage());

    Q_D(Content);
    if (d->multipartContents.isEmpty() || !d->multipartContents.contains(c)) {
        return nullptr;
    }

    d->multipartContents.removeAll(c);
    c->d_ptr->parent = nullptr;
    return c;
}

void Content::changeEncoding(Headers::contentEncoding e)
{
    // This method makes no sense for encapsulated messages, they are always 7bit
    // encoded.
    Q_ASSERT(!bodyIsMessage());

    Headers::ContentTransferEncoding *enc = contentTransferEncoding();
    if (enc->encoding() == e) {
        // Nothing to do.
        return;
    }

    if (d_ptr->decodeText(this)) {
        // This is textual content.  Textual content is stored decoded.
        Q_ASSERT(d_ptr->m_decoded);
        enc->setEncoding(e);
    } else {
        // This is non-textual content.  Re-encode it.
        if (e == Headers::CEbase64) {
            KCodecs::base64Encode(decodedContent(), d_ptr->body, true);
            enc->setEncoding(e);
            d_ptr->m_decoded = false;
        } else {
            // It only makes sense to convert binary stuff to base64.
            Q_ASSERT(false);
        }
    }
}

QList<Headers::Base *> Content::headers() const { return d_ptr->headers; }

Headers::Base *Content::headerByType(QByteArrayView type) const
{
    for (Headers::Base *h : std::as_const(d_ptr->headers)) {
        if (h->is(type)) {
            return h; // Found.
        }
    }

    return nullptr; // Not found.
}

QList<Headers::Base *> Content::headersByType(QByteArrayView type) const
{
    QList<Headers::Base *> result;

    for (Headers::Base *h : std::as_const(d_ptr->headers)) {
        if (h->is(type)) {
            result << h;
        }
    }

    return result;
}

void Content::setHeader(Headers::Base *h)
{
    Q_ASSERT(h);
    removeHeader(h->type());
    appendHeader(h);
}

void Content::appendHeader(Headers::Base *h)
{
    Q_D(Content);
    d->headers.append(h);
}

bool Content::removeHeader(QByteArrayView type)
{
    Q_D(Content);
    const auto endIt = d->headers.end();
    for (auto it = d->headers.begin(); it != endIt; ++it) {
        if ((*it)->is(type)) {
            delete(*it);
            d->headers.erase(it);
            return true;
        }
    }

    return false;
}

bool Content::hasHeader(QByteArrayView type) const
{
    return headerByType(type) != nullptr;
}

qsizetype Content::size() const
{
    const auto ret = d_ptr->body.size();

    if (const auto cte = contentTransferEncoding(); cte && cte->encoding() == Headers::CEbase64) {
        KCodecs::Codec *codec = KCodecs::Codec::codecForName("base64");
        return codec->maxEncodedSizeFor(ret);
    }

    // Not handling quoted-printable here since that requires actually
    // converting the content, and that is O(size_of_content).
    // For quoted-printable, this is only an approximate size.

    return ret;
}

qsizetype Content::storageSize() const
{
    const Q_D(Content);
    auto s = d->head.size();

    if (d->contents().isEmpty()) {
        s += d->body.size();
    } else {

        // FIXME: This should take into account the boundary headers that are added in
        //        encodedContent!
        const auto contents = d->contents();
        for (Content *c : contents) {
            s += c->storageSize();
        }
    }

    return s;
}

bool ContentPrivate::needToEncode(const Content *q) const
{
    const auto cte = q->contentTransferEncoding();
    return m_decoded && cte && (cte->encoding() == Headers::CEquPr || cte->encoding() == Headers::CEbase64);
}

bool ContentPrivate::decodeText(const Content *q)
{
    const Headers::ContentTransferEncoding *enc = q->contentTransferEncoding();

    if (const auto ct = q->contentType(); ct && !ct->isText()) {
        return false; //non textual data cannot be decoded here => use decodedContent() instead
    }
    if (m_decoded) {
        return true; //nothing to do
    }

    if (enc) {
        switch (enc->encoding()) {
        case Headers::CEbase64 :
            body = KCodecs::base64Decode(body);
            break;
        case Headers::CEquPr :
            body = KCodecs::quotedPrintableDecode(body);
            break;
        case Headers::CEuuenc :
            body = KCodecs::uudecode(body);
            break;
        case Headers::CEbinary :
            // nothing to decode
        default :
            break;
        }
    }
    if (!body.endsWith('\n')) {
        body.append('\n');
    }
    m_decoded = true;
    return true;
}

Content *KMime::Content::content(const ContentIndex &index) const
{
    if (!index.isValid()) {
        return const_cast<KMime::Content *>(this);
    }
    ContentIndex idx = index;
    unsigned int i = idx.pop() - 1; // one-based -> zero-based index
    if (i < static_cast<unsigned int>(d_ptr->contents().size())) {
        return d_ptr->contents().at(i)->content(idx);
    } else {
        return nullptr;
    }
}

ContentIndex KMime::Content::indexForContent(Content *content) const
{
    const auto i = d_ptr->contents().indexOf(content);
    if (i >= 0) {
        ContentIndex ci;
        ci.push(i + 1);   // zero-based -> one-based index
        return ci;
    }
    // not found, we need to search recursively
    for (int i = 0; i < d_ptr->contents().size(); ++i) {
        ContentIndex ci = d_ptr->contents().at(i)->indexForContent(content);
        if (ci.isValid()) {
            // found it
            ci.push(i + 1);   // zero-based -> one-based index
            return ci;
        }
    }
    return {}; // not found
}

bool Content::isTopLevel() const
{
    return d_ptr->parent == nullptr;
}

void Content::setParent(Content *parent)
{
    // Make sure the Content is only in the contents list of one parent object
    Content *oldParent = d_ptr->parent;
    if (oldParent) {
        if (!oldParent->contents().isEmpty() && oldParent->contents().contains(this)) {
            oldParent->takeContent(this);
        }
    }

    d_ptr->parent = parent;
    if (parent) {
        if (!parent->contents().isEmpty() && !parent->contents().contains(this)) {
            parent->appendContent(this);
        }
    }
}

Content *Content::parent()
{
    return d_ptr->parent;
}

const Content *Content::parent() const
{
    return d_ptr->parent;
}

Content *Content::topLevel()
{
    auto c = this;
    while (c->parent()) {
        c = c->parent();
    }
    return c;
}

const Content *Content::topLevel() const
{
    auto c = this;
    while (c->parent()) {
        c = c->parent();
    }
    return c;
}

ContentIndex Content::index() const
{
    auto top = topLevel();
    if (top) {
        return top->indexForContent(const_cast<Content *>(this));
    }

    return indexForContent(const_cast<Content *>(this));
}

Message::Ptr Content::bodyAsMessage() const
{
    if (bodyIsMessage() && d_ptr->bodyAsMessage) {
        return d_ptr->bodyAsMessage;
    } else {
      return {};
    }
}

bool Content::bodyIsMessage() const
{
    if (const auto ct = contentType(); ct) {
        return ct->isMimeType("message/rfc822");
    }
    return false;
}

// @cond PRIVATE
#define kmime_mk_header_accessor( type, method ) \
    Headers::type *Content::method( bool create ) { \
        return header<Headers::type>( create ); \
    } \
    const Headers::type *Content::method() const { \
        return header<Headers::type>(); \
    }

kmime_mk_header_accessor(ContentType, contentType)
kmime_mk_header_accessor(ContentTransferEncoding, contentTransferEncoding)
kmime_mk_header_accessor(ContentDisposition, contentDisposition)
kmime_mk_header_accessor(ContentDescription, contentDescription)
kmime_mk_header_accessor(ContentLocation, contentLocation)
kmime_mk_header_accessor(ContentID, contentID)

#undef kmime_mk_header_accessor
// @endcond

void ContentPrivate::clearBodyMessage()
{
    bodyAsMessage.reset();
}

QList<Content *> ContentPrivate::contents() const {
    Q_ASSERT(multipartContents.isEmpty() || !bodyAsMessage);
    if (bodyAsMessage) {
      return QList<Content *>() << bodyAsMessage.data();
    } else {
        return multipartContents;
    }
}

bool ContentPrivate::parseUuencoded(Content *q)
{
    Parser::UUEncoded uup(body, head);
    if (!uup.parse()) {
        return false; // Parsing failed.
    }

    Headers::ContentType *ct = q->contentType();
    ct->clear();

    if (uup.isPartial()) {
        // This seems to be only a part of the message, so we treat it as "message/partial".
        ct->setMimeType("message/partial");
        //ct->setId( uniqueString() ); not needed yet
        ct->setPartialParams(uup.partialCount(), uup.partialNumber());
        q->contentTransferEncoding()->setEncoding(Headers::CE7Bit);
    } else {
        // This is a complete message, so treat it as "multipart/mixed".
        const auto prevBody = body;
        body.clear();
        ct->setMimeType("multipart/mixed");
        ct->setBoundary(multiPartBoundary());
        auto cte = q->contentTransferEncoding();
        cte->setEncoding(Headers::CE7Bit);
        m_decoded = true;

        // Add the plain text part first.
        Q_ASSERT(multipartContents.isEmpty());
        {
            auto c = new Content(q);
            c->contentType()->setMimeType("text/plain");
            c->contentTransferEncoding()->setEncoding(Headers::CE7Bit);
            c->setBody(uup.textPart());
            multipartContents.append(c);
        }

        // Now add each of the binary parts as sub-Contents.
        for (int i = 0; i < uup.binaryParts().count(); ++i) {
            auto c = new Content(q);
            c->contentType()->setMimeType(uup.mimeTypes().at(i));
            c->contentType()->setName(QLatin1StringView(uup.filenames().at(i)));
            c->contentTransferEncoding()->setEncoding(Headers::CEuuenc);
            c->contentDisposition()->setDisposition(Headers::CDattachment);
            c->contentDisposition()->setFilename(
                QLatin1StringView(uup.filenames().at(i)));
            // uup.binaryParts().at(i) does no longer have the uuencode header, which makes KCodecs fail since 5c66308c4786ef7fbf77b0e306e73f7d4ac3431b
            c->setEncodedBody(prevBody);
            c->changeEncoding(Headers::CEbase64);   // Convert to base64.
            multipartContents.append(c);
        }
    }

    return true; // Parsing successful.
}

bool ContentPrivate::parseYenc(Content *q)
{
    Parser::YENCEncoded yenc(body);
    if (!yenc.parse()) {
        return false; // Parsing failed.
    }

    Headers::ContentType *ct = q->contentType();
    ct->clear();

    if (yenc.isPartial()) {
        // Assume there is exactly one decoded part.  Treat this as "message/partial".
        ct->setMimeType("message/partial");
        //ct->setId( uniqueString() ); not needed yet
        ct->setPartialParams(yenc.partialCount(), yenc.partialNumber());
        q->contentTransferEncoding()->setEncoding(Headers::CEbinary);
        q->changeEncoding(Headers::CEbase64);   // Convert to base64.
    } else {
        // This is a complete message, so treat it as "multipart/mixed".
        body.clear();
        ct->setMimeType("multipart/mixed");
        ct->setBoundary(multiPartBoundary());
        auto cte = q->contentTransferEncoding();
        cte->setEncoding(Headers::CE7Bit);
        m_decoded = true;

        // Add the plain text part first.
        Q_ASSERT(multipartContents.isEmpty());
        {
            auto c = new Content(q);
            c->contentType()->setMimeType("text/plain");
            c->contentTransferEncoding()->setEncoding(Headers::CE7Bit);
            c->setBody(yenc.textPart());
            multipartContents.append(c);
        }

        // Now add each of the binary parts as sub-Contents.
        for (int i = 0; i < yenc.binaryParts().count(); i++) {
            auto c = new Content(q);
            c->contentType()->setMimeType(yenc.mimeTypes().at(i));
            c->contentType()->setName(QLatin1StringView(yenc.filenames().at(i)));
            c->contentTransferEncoding()->setEncoding(Headers::CEbinary);
            c->contentDisposition()->setDisposition(Headers::CDattachment);
            c->contentDisposition()->setFilename(
                QLatin1StringView(yenc.filenames().at(i)));
            c->setBody(yenc.binaryParts().at(i));     // Yenc bodies are binary.
            c->changeEncoding(Headers::CEbase64);   // Convert to base64.
            multipartContents.append(c);
        }
    }

    return true; // Parsing successful.
}

bool ContentPrivate::parseMultipart(Content *q)
{
    const Headers::ContentType *ct = q->contentType();
    const QByteArray boundary = ct->boundary();
    if (boundary.isEmpty()) {
        return false; // Parsing failed; invalid multipart content.
    }
    Parser::MultiPart mpp(body, boundary);
    if (!mpp.parse()) {
        return false; // Parsing failed.
    }

    preamble = mpp.preamble();
    epilogue = mpp.epilouge();

    // Create a sub-Content for every part.
    Q_ASSERT(multipartContents.isEmpty());
    body.clear();
    const auto parts = mpp.parts();
    for (const QByteArray &part : parts) {
        auto c = new Content(q);
        c->setContent(part);
        c->setFrozen(frozen);
        c->parse();
        multipartContents.append(c);
    }

    return true; // Parsing successful.
}

} // namespace KMime
