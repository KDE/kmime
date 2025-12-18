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

TODO: possible glossary terms:
 content
   encoding, transfer type, disposition, description
 header
 body
 attachment
 charset
 article
 string representation
 broken-down object representation
*/

#pragma once

#include "kmime_export.h"
#include "contentindex.h"
#include "util.h"
#include "headers.h"

#include <QByteArray>
#include <QList>
#include <QMetaType>
#include <QSharedPointer>

#include <span>

namespace KMime
{

/*!
 * Controls whether to create a header that does not exist.
 *
 * \value DontCreate Do not create header
 * \value Create Create header
 *
 * \since 26.04
 */
enum CreatePolicy {
    DontCreate,
    Create,
};

///@cond internal
namespace Internal {

template <typename T>
class OwningConstPtrSpan : public std::span<const T *const> {
public:
    explicit inline OwningConstPtrSpan(const QList<T*> &list)
      : std::span<const T *const>(static_cast<const T* const*>(list.constData()), list.size())
      , m_list(list)
    {}
private:
    QList<T*> m_list;
};
}
///@endcond

class ContentPrivate;
class Message;

/**
  @brief
  A class that encapsulates @ref MIME encoded Content.

  A Content object holds two representations of a content:
  - the string representation: This is the content encoded as a string ready
    for transport.  Accessible through the encodedContent() method.
  - the broken-down representation: This is the tree of objects (headers,
    sub-Contents and (if present) the encapsulated message) that this Content is made of.
    Accessible through methods like header(), contents() and bodyAsMessage().

  The parse() function updates the broken-down representation of the Content
  from its string representation.  Calling it is necessary to access the
  headers, sub-Contents or the encapsulated message of this Content.

  The assemble() function updates the string representation of the Content
  from its broken-down representation.  Calling it is necessary for
  encodedContent() to reflect any changes made to the broken-down representation of the Content.

  There are two basic types of a Content:
  - A leaf Content: This is a content that is neither a multipart content nor an encapsulated
                    message. Because of this, it will not have any children, it has no sub-contents
                    and is therefore a leaf content.
                    Only leaf contents have a body that is not empty, i.e. functions that operate
                    on the body, such as body(), size() and decodedBody(), will work only on
                    leaf contents.
  - A non-leaf Content: This is a content that itself doesn't have any body, but that does have
                        sub-contents.
                        This is the case for contents that are of mimetype multipart/ or of mimetype
                        message/rfc822. In case of a multipart content, contents() will return the
                        multipart child contents. In case of an encapsulated message, the message
                        can be accessed with bodyAsMessage(), and contents() will have one entry
                        that is the message as well.
                        On a non-leaf content, body() will have an empty return value and other
                        functions working on the body will not work.
                        A call to parse() is required before the child multipart contents or the
                        encapsulated message is created.
*/
class KMIME_EXPORT Content
{
public:
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    /**
      Describes a list of Content objects.
    */
  KMIME_DEPRECATED typedef QList<KMime::Content *> List;
#endif
  /**
    Creates an empty Content object with a specified parent.
    @param parent the parent Content object
    @since 4.3
  */
  explicit Content(Content *parent = nullptr);

  /**
    Destroys this Content object.
  */
  virtual ~Content();

  /**
    Returns true if this Content object is not empty.
  */
  [[nodiscard]] bool hasContent() const;

  /**
    Sets the Content to the given raw data, containing the Content head and
    body separated by two linefeeds.

    This method operates on the string representation of the Content. Call
    parse() if you want to access individual headers, sub-Contents or the
    encapsulated message.

    @note The passed data must not contain any CRLF sequences, only LF.
          Use CRLFtoLF for conversion before passing in the data.

    @param s is a QByteArray containing the raw Content data.
  */
  void setContent(const QByteArray &s);

  /**
   * Parses the Content.
   *
   * This means the broken-down object representation of the Content is
   * updated from the string representation of the Content.
   *
   * Call this if you want to access or change headers, sub-Contents or the
   * encapsulated message.
   *
   * @note Calling parse() twice will not work for multipart contents or for
   * contents of which the body is an encapsulated message. The reason is that
   * the first parse() will delete the body, so there is no body to work on for
   * the second call of parse().
   *
   * @note Calling this will reset the message returned by bodyAsMessage(), as
   *       the message is re-parsed as well.
   *       Also, all old sub-contents will be deleted, so any old Content
   * pointer will become invalid.
   */
  void parse();

  /**
    Returns whether this Content is frozen.
    A frozen content is immutable, i.e. calling assemble() will never modify
    its head or body, and encodedContent() will return the same data before
    and after parsing.

    @since 4.4.
    @see setFrozen().
  */
  [[nodiscard]] bool isFrozen() const;

  /**
    Freezes this Content if @p frozen is true; otherwise unfreezes it.
    @param frozen freeze content if @c true, otherwise unfreeze
    @since 4.4
    @see isFrozen().
  */
  void setFrozen(bool frozen = true);

  /**
    Generates the MIME content.
    This means the string representation of this Content is updated from the
    broken-down object representation.
    Call this if you have made changes to the content, and want
    encodedContent() to reflect those changes.

    @note assemble() has no effect if the Content isFrozen().  You may want
    to freeze, for instance, signed sub-Contents, to make sure they are kept
    unmodified.

    @note If this content is an encapsulated message, i.e. bodyIsMessage()
    returns true, then calling assemble() will also assemble the message
    returned by bodyAsMessage().

    @warning assemble() may change the order of the headers, and other
    details such as where folding occurs.  This may break things like
    signature verification, so you should *ONLY* call assemble() when you
    have actually modified the content.
  */
  void assemble();

  /**
    Clears the content, deleting all headers and sub-Contents.
  */
  void clear();

  /**
    Removes all sub-Contents from this content. sub-Contents will be deleted.
    Calling clearContents() does NOT make this Content single-part.

    @since 4.4
  */
  void clearContents();

  /**
    Returns the Content header raw data.

    @see setHead().
  */
  [[nodiscard]] QByteArray head() const;

  /**
    Sets the Content header raw data.

    This method operates on the string representation of the Content. Call
    parse() if you want to access individual headers.

    @param head is a QByteArray containing the header data.

    @see head().
  */
  void setHead(const QByteArray &head);

  /**
   * Returns all headers.
   * @since 5.7
   */
  [[nodiscard]] QList<Headers::Base *> headers();
  [[nodiscard]] inline auto headers() const -> auto
  {
      return Internal::OwningConstPtrSpan<Headers::Base>(const_cast<Content*>(this)->headers());
  }

  /**
    Returns the first header of type @p type, if it exists.  Otherwise returns
    0. Note that the returned header may be empty.
    @param type the header type to find
    @since 4.2
  */
  [[nodiscard]] Headers::Base *headerByType(QByteArrayView type) const;

  /**
    Returns the first header of type T, if it exists.
    If the header does not exist and \p create is \c Create, creates an empty header
    and returns it. Otherwise returns \c nullptr.
    Note that the returned header may be empty.
    @param create Whether to create the header if it does not exist.
    @since 4.4 (took a bool before 26.04)
  */
  template <typename T> T *header(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  template <typename T> [[deprecated("use the CreatePolicy overload instead")]] inline T *header(bool create) {
      return header<T>(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the first header of type @tparam T.

    Can be @c nullptr if such a header doesn't exist.
    @since 24.08
  */
  template <typename T> [[nodiscard]] const std::remove_cv_t<T> *header() const;

  /**
    Returns all @p type headers in the Content.
    Take care that this result is not cached, so could be slow.
    @param type the header type to find
    @since 4.2
  */
  [[nodiscard]] QList<Headers::Base *> headersByType(QByteArrayView type) const;

  /**
    Sets the specified header to this Content.
    Any previous header of the same type is removed.
    If you need multiple headers of the same type, use appendHeader() or
    prependHeader().

    @param h The header to set.
    @see appendHeader()
    @see removeHeader()
    @since 4.4 (took a raw pointer before 26.04)
  */
  void setHeader(std::unique_ptr<Headers::Base> &&h);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the unique_ptr overload instead")]] void setHeader(Headers::Base *h);
#endif

  /**
    Appends the specified header to the headers of this Content.
    @param h The header to append.
    @since 4.4 (took a raw pointer before 26.04)
  */
  void appendHeader(std::unique_ptr<Headers::Base> &&h);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the unique_ptr overload instead")]] void appendHeader(Headers::Base *h);
#endif

  /**
    Searches for the first header of type @p type, and deletes it, removing
    it from this Content.
    @param type The type of the header to look for.
    @return true if a header was found and removed.
  */
  bool removeHeader(QByteArrayView type);

  /**
    Searches for the first header of type @p T, and deletes it, removing
    it from this Content.
    @tparam T The type of the header to look for.
    @return true if a header was found and removed.
  */
  template <typename T> bool removeHeader();

  /**
    @return true if this Content has a header of type @p type.
    @param type The type of the header to look for.
  */
  // TODO probably provide hasHeader<T>() too.
  [[nodiscard]] bool hasHeader(QByteArrayView type) const;

  /**
    Returns the Content-Type header.

    @param create Whether to create the header if it doesn't exist yet.
  */
  Headers::ContentType *contentType(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentType *contentType(bool create) {
      return contentType(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-Type header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentType *contentType() const;

  /**
    Returns the Content-Transfer-Encoding header.

    @param create Whether to create the header if it doesn't exist yet.
  */
  Headers::ContentTransferEncoding *contentTransferEncoding(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentTransferEncoding *contentTransferEncoding(bool create) {
      return contentTransferEncoding(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-Transfer-Encoding header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentTransferEncoding *contentTransferEncoding() const;

  /**
    Returns the Content-Disposition header.

    @param create Whether to create the header if it doesn't exist yet.
  */
  Headers::ContentDisposition *contentDisposition(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentDisposition *contentDisposition(bool create) {
      return contentDisposition(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-Disposition header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentDisposition *contentDisposition() const;

  /**
    Returns the Content-Description header.

    @param create Whether to create the header if it doesn't exist yet.
  */
  Headers::ContentDescription *contentDescription(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentDescription *contentDescription(bool create) {
      return contentDescription(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-Description header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentDescription *contentDescription() const;

  /**
    Returns the Content-Location header.

    @param create Whether to create the header if it doesn't exist yet.
    @since 4.2
  */
  Headers::ContentLocation *contentLocation(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentLocation *contentLocation(bool create) {
      return contentLocation(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-Location header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentLocation *contentLocation() const;

  /**
    Returns the Content-ID header.
    @param create Whether to create the header if it doesn't exist yet.
    @since 4.4
  */
  Headers::ContentID *contentID(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the CreatePolicy overload instead")]] inline Headers::ContentID *contentID(bool create) {
      return contentID(create ? Create : DontCreate);
  }
#endif
  /**
    Returns the Content-ID header.

    Can be @c nullptr if the header doesn't exist.
    @since 24.08
  */
  [[nodiscard]] const Headers::ContentID *contentID() const;

  /**
    Returns the size of the Content body after encoding.
    (If the encoding is quoted-printable, this is only an approximate size.)
    This will return 0 for multipart contents or for encapsulated messages.
  */
  [[nodiscard]] qsizetype size() const;

  /**
    Returns the size of this Content and all sub-Contents.
  */
  [[nodiscard]] qsizetype storageSize() const;

  /**
    Returns the Content body raw data.

    Note that this will be empty for multipart contents or for encapsulated
    messages, after parse() has been called.

    @see setBody().
  */
  [[nodiscard]] QByteArray body() const;

  /**
    Sets the Content decoded body raw data.

    This method operates on the string representation of the Content. Call
    parse() if you want to access individual sub-Contents or the encapsulated
    message.

    @param body is a QByteArray containing the body data.

    @note @p body is assumed to be decoded as far as the content transfer encoding
    is concerned.

    @see setEncodedBody()
  */
  void setBody(const QByteArray &body);

  /**
    Sets the Content body raw data encoded according to the content transfer encoding.

    This method operates on the string representation of the Content. Call
    parse() if you want to access individual sub-Contents or the encapsulated
    message.

    @param body is a QByteArray containing the body data.

    @note @p body is assumed to be encoded as far as the content transfer encoding
    is concerned.

    @since 24.08

    @see setBody()
  */
  void setEncodedBody(const QByteArray &body);

  /**
    Returns the MIME preamble.

    @return a QByteArray containing the MIME preamble.

    @since 4.9
   */
  [[nodiscard]] QByteArray preamble() const;

  /**
    Sets the MIME preamble.

    @param preamble a QByteArray containing what will be used as the
    MIME preamble.

    @since 4.9
   */

  void setPreamble(const QByteArray &preamble);

  /**
    Returns the MIME preamble.

    @return a QByteArray containing the MIME epilogue.

    @since 4.9
   */
  [[nodiscard]] QByteArray epilogue() const;

  /**
    Sets the MIME preamble.

    @param epilogue a QByteArray containing what will be used as the
    MIME epilogue.

    @since 4.9
   */
  void setEpilogue(const QByteArray &epilogue);

  /**
    Returns a QByteArray containing the encoded Content, including the
    Content header and all sub-Contents.

    If you make changes to the broken-down representation of the message, be
    sure to first call assemble() before calling encodedContent(), otherwise
    the result will not be up-to-date.

    If this content is an encapsulated message, i.e. bodyIsMessage() returns
    true, then encodedContent() will use the message returned by bodyAsMessage()
    as the body of the result, calling encodedContent() on the message.

    \a newline whether to use CRLF for linefeeds, or LF (default is LF).
  */
  [[nodiscard]] QByteArray encodedContent(NewlineType newline = NewlineType::LF) const;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the NewlineType overload instead")]]
  [[nodiscard]] QByteArray encodedContent(bool useCrLf) const;
#endif

  /**
   * Like encodedContent(), with the difference that only the body will be
   * returned, i.e. the headers are excluded.
   *
   * @since 4.6
   */
  [[nodiscard]] QByteArray encodedBody() const;

  /**
   * Returns the decoded Content body.
   *
   * Note that this will be empty for multipart contents or for encapsulated
   * messages, after parse() has been called.
   * @since 25.12 (previously decodedContent())
   */
  [[nodiscard]] QByteArray decodedBody() const;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("Use decodedBody() instead")]]
  [[nodiscard]] QByteArray decodedContent() const;
#endif

  /** Options for Content::decodedText().
   *  @since 24.12
   */
  enum DecodedTextTrimOption {
    NoTrim, ///< do not trim text content.
    TrimNewlines, ///< trim trailing newlines
    TrimSpaces, ///< trim any trailing whitespaces
  };

  /**
    Returns the decoded text. Additional to decodedBody(), this also
    applies charset decoding. If this is not a text Content, decodedText()
    returns an empty QString.

    @param trimOption Control how to trim trailing white spaces.
    The last trailing new line of the decoded text is always removed.

    @since 24.12
  */
  [[nodiscard]] QString decodedText(DecodedTextTrimOption trimOption = NoTrim) const;

  /**
    Returns the decoded text. Additional to decodedBody(), this also
    applies charset decoding. If this is not a text Content, decodedText()
    returns an empty QString.

    @param trimText If true, then the decoded text will have all trailing
    whitespace removed.
    @param removeTrailingNewlines If true, then the decoded text will have
    all consecutive trailing newlines removed.

    The last trailing new line of the decoded text is always removed.

    @deprecated since 24.12, use decodedText(DecodedTextTrimOption) instead.
  */
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use decodedText(DecodedTextTrimOption) instead")]]
  [[nodiscard]] inline QString decodedText(bool trimText, bool removeTrailingNewlines = false) const
  {
    return decodedText(trimText ? TrimSpaces : removeTrailingNewlines ? TrimNewlines : NoTrim);
  }
#endif

  /**
    Sets the Content body to the given string using charset of the content type.

    If the charset can not be found, the system charset is taken and the content
    type header is changed to that charset. The charset of the content type
    header should be set to a charset that can encode the given string before
    calling this method.

    This method does not set the content transfer encoding automatically, it
    needs to be set to a suitable value that can encode the given string before
    calling this method.

    This method only makes sense for single-part contents, do not try to pass a
    multipart body or an encapsulated message here, that wouldn't work.

    @param s Unicode-encoded string.
  */
  void fromUnicodeString(const QString &s);

  /**
    Returns the first Content with mimetype text/.
  */
  [[nodiscard]] Content *textContent();
  /**
    Returns the first Content with MIME type text/.
    Const overload of the above, the returned Content cannot be modified.
    @since 24.08
  */
  [[nodiscard]] const Content *textContent() const;

  /**
   * Returns all attachments below this node, recursively.
   * This does not include crypto parts, nodes of alternative or related
   * multipart nodes, or the primary body part (see textContent()).
   * @see KMime::isAttachment(), KMime::hasAttachment()
   */
  [[nodiscard]] QList<Content *> attachments();
  [[nodiscard]] inline auto attachments() const -> auto {
      return Internal::OwningConstPtrSpan<Content>(const_cast<Content*>(this)->attachments());
  }

  /**
   * For multipart contents, this will return a list of all multipart child
   * contents. For contents that are of mimetype message/rfc822, this will
   * return a list with one entry, and that entry is the encapsulated message,
   * as it would be returned by bodyAsMessage().
   */
  [[nodiscard]] QList<Content *> contents();
  [[nodiscard]] inline auto contents() const -> auto {
      return Internal::OwningConstPtrSpan<Content>(const_cast<Content*>(this)->contents());
  }

  /**
    Appends a new sub-Content. If the sub-Content is already part of another
    Content object, it is removed from there and its parent is updated.

    @param content The new sub-Content.
    @see prependContent()
    @see takeContent()
    @since 6.0 (took a raw pointer until 26.04)
  */
  void appendContent(std::unique_ptr<KMime::Content> &&content);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the unique_ptr overload instead")]] void appendContent(Content *content);
#endif

  /**
    Prepends a new sub-Content. If the sub-Content is already part of another
    Content object, it is removed from there and its parent is updated.

    @param content The new sub-Content.
    @see appendContent()
    @see takeContent()
    @since 6.0 (took a raw pointer until 26.04)
  */
  void prependContent(std::unique_ptr<KMime::Content> &&content);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use the unique_ptr overload instead")]] void prependContent(Content *content);
#endif

  /**
    Removes the given sub-Content and, if that actually was a sub-content
    returns that.

    @param content The Content to remove. It is not deleted, ownership goes
    back to the caller.

    @see appendContent()
    @see prependContent()
    @see clearContents()
    @since 6.0 (returned a raw pointer until 26.04)
  */
  std::unique_ptr<Content> takeContent(Content *content);

  /**
    Changes the encoding of this Content to @p e.  If the Content is binary,
    this actually re-encodes the data to use the new encoding.

    @param e The new encoding to use.
  */
  void changeEncoding(Headers::contentEncoding e);

  /**
    Returns the Content specified by the given index.
    If the index does not point to a Content, 0 is returned. If the index
    is invalid (empty), this Content is returned.

    @param index The Content index.
  */
  [[nodiscard]] Content *content(const ContentIndex &index) const;

  /**
    Returns the ContentIndex for the given Content, or an invalid index
    if the Content is not found within the hierarchy.
    @param content the Content object to search.
  */
  [[nodiscard]] ContentIndex indexForContent(const Content *content) const;

  /**
    Returns true if this is the top-level node in the MIME tree. The top-level
    node is always a Message or NewsArticle. However, a node can be a Message
    without being a top-level node when it is an encapsulated message.
  */
  [[nodiscard]] bool isTopLevel() const;

  /**
   * Sets a new parent to the Content and add to its contents list. If it
   * already had a parent, it is removed from the old parents contents list.
   * @param parent the new parent
   * @since 4.3
   */
  void setParent(Content *parent);

  /**
   * Returns the parent content object, or 0 if the content doesn't have a
   * parent.
   * @since 4.3
   */
  [[nodiscard]] Content *parent();
  [[nodiscard]] const Content *parent() const;

  /**
   * Returns the toplevel content object, 0 if there is no such object.
   * @since 4.3
   */
  [[nodiscard]] Content *topLevel();
  [[nodiscard]] const Content *topLevel() const;


  /**
   * Returns the index of this Content based on the topLevel() object.
   * @since 4.3
   */
  [[nodiscard]] ContentIndex index() const;

  /**
   * @return true if this content is an encapsulated message, i.e. if it has the
   * mimetype message/rfc822.
   *
   * @since 4.5
   */
  // AK_REVIEW: move to MessageViewer/ObjectTreeParser
  [[nodiscard]] bool bodyIsMessage() const;

  /**
   * If this content is an encapsulated message, in which case bodyIsMessage()
   * will return true, the message represented by the body of this content will
   * be returned. The returned message is already fully parsed. Calling this
   * method is the aquivalent of calling contents().first() and casting the
   * result to a KMime::Message*. bodyAsMessage() has the advantage that it will
   * return a shared pointer that will not be destroyed when the container
   * message is destroyed or re-parsed.
   *
   * The message that is returned here is created when calling parse(), so make
   * sure to call parse() first. Since each parse() creates a new message
   * object, a different message object will be returned each time you call
   * parse().
   *
   * If you make changes to the returned message, you need to call assemble() on
   * this content or on the message if you want that encodedContent() reflects
   * these changes. This also means that calling assemble() on this content will
   * assemble the returned message.
   *
   * @since 4.5
   */
  [[nodiscard]] std::shared_ptr<Message> bodyAsMessage();
  [[nodiscard]] inline std::shared_ptr<const Message> bodyAsMessage() const
  {
      return const_cast<Content*>(this)->bodyAsMessage();
  }

protected:
    /**
      Reimplement this method if you need to assemble additional headers in a
      derived class. Don't forget to call the implementation of the base class.
      @return The raw, assembled headers.
    */
    virtual QByteArray assembleHeaders();

    //@cond PRIVATE
    ContentPrivate *d_ptr;
    //@endcond

private:
    Q_DECLARE_PRIVATE(Content)
    Q_DISABLE_COPY(Content)
};

template <typename T> T *Content::header(CreatePolicy create)
{

    if (auto h = headerByType(T::staticType()); h) {
        // Make sure the header is actually of the right type.
        Q_ASSERT(dynamic_cast<T *>(h));
        return static_cast<T *>(h);
    }
    if (create == Create) {
        auto hptr = std::make_unique<T>();
        auto h = hptr.get();
        appendHeader(std::move(hptr)); // we already know the header doesn't exist yet
        return h;
    }
    return nullptr;
}

template <typename T> const std::remove_cv_t<T> *Content::header() const
{
    Headers::Base *h = headerByType(T::staticType());
    if (h) {
        // Make sure the header is actually of the right type.
        Q_ASSERT(dynamic_cast<T *>(h));
    }
    return static_cast<T *>(h);
}

template <typename T> bool Content::removeHeader()
{
    return removeHeader(T::staticType());
}

} // namespace KMime

Q_DECLARE_METATYPE(KMime::Content*)

