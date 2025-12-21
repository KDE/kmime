/*  -*- c++ -*-
    kmime_mdn.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/*!
  @file
  This file is part of the API for handling \ MIME data and
  provides functions for supporting Message Disposition Notifications (MDNs),
  also known as email return receipts.

  \brief
  Provides support for Message Disposition Notifications.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor MDN @b MDN:
  see \ Message_Disposition_Notification

  @glossary @anchor Message_Disposition_Notification
  @b Message @b Disposition @b Notification:
  Return receipts for email are called message disposition notifications.
  Their format and usage is outlined in \ RFC2298.

  @glossary @anchor RFC2298 @anchor rfc2298 @b RFC @b 2298:
  RFC that defines the <a href="https://tools.ietf.org/html/rfc2298">
  An Extensible Message Format for Message Disposition Notifications</a>.
*/

#pragma once

#include "kmime_export.h"
#include <QList>
#include <QString>

class QByteArray;

namespace KMime
{

namespace MDN
{

/*!
  The following disposition-types are defined:

  @li Displayed    The message has been displayed by the UA to someone
  reading the recipient's mailbox.  There is no guarantee that the
  content has been read or understood.

  @li Dispatched   The message has been sent somewhere in some manner
  (e.g., printed, faxed, forwarded) without necessarily having been previously
  displayed to the user.  The user may or may not see the message later.

  @li Processed    The message has been processed in some manner (i.e., by
  some sort of rules or server) without being displayed to the user.  The user
  may or may not see the message later, or there may not even be a human user
  associated with the mailbox.

  @li Deleted      The message has been deleted.  The recipient may or may not
  have seen the message.  The recipient might "undelete" the message at a
  later time and read the message.

  @li Denied       The recipient does not wish the sender to be informed of the
  message's disposition.  A UA may also silently ignore message disposition
  requests in this situation.

  @li Failed       A failure occurred that prevented the proper generation
  of an MDN.  More information about the cause of the failure may be contained
  in a Failure field.  The "failed" disposition type is not to be used for
  the situation in which there is is some problem in processing the message
  other than interpreting the request for an MDN.  The "processed" or other
  disposition type with appropriate disposition modifiers is to be used in
  such situations.

  IOW:
  \a Displayed when - well -displayed
  \a Dispatched when forwarding unseen ( == new )
  \a Processed (maybe) when piping unseen, but probably never used
  \a Deleted when deleting unseen
  \a Denied on user command
  \a Failed on Disposition-Notification-Options containing
  unknown required options. ( == @em any required options )
  \a Failed needs a description in the \a special parameter.
*/
enum DispositionType {
    Displayed, Read = Displayed,
    Deleted,
    Dispatched, Forwarded = Dispatched,
    Processed,
    Denied,
    Failed
};

/*!
  The following disposition modifiers are defined:

  @li Error               An error of some sort occurred that prevented
  successful processing of the message.  Further information is contained
  in an Error field.

  @li Warning             The message was successfully processed but some
  sort of exceptional condition occurred.  Further information is contained
  in a Warning field.

  @li Superseded          The message has been automatically rendered obsolete
  by another message received.  The recipient may still access and read the
  message later.

  @li Expired             The message has reached its expiration date and has
  been automatically removed from the recipient's mailbox.

  @li MailboxTerminated   The recipient's mailbox has been terminated and all
  message in it automatically removed.
*/
enum DispositionModifier {
    Error,
    Warning,
    Superseded,
    Expired,
    MailboxTerminated
};

/*!
  The following disposition modes are defined:

  @li ManualAction    The disposition described by the disposition type
  was a result of an explicit instruction by the user rather than some sort of
  automatically performed action.

  @li AutomaticAction The disposition described by the disposition type was
  a result of an automatic action, rather than an explicit instruction by the
  user for this message.

  IOW:
  \a ManualAction for user-driven actions,
  \a AutomaticAction for filtering.
*/
enum ActionMode {
    ManualAction,
    AutomaticAction
};

/*!
  @li SentManually      The user explicitly gave permission for this
  particular MDN to be sent.

  @li SentAutomatically The MDN was sent because the MUA had previously
  been configured to do so automatically.

  IOW:
  \a SentManually for when we have asked the user
  \a SentAutomatically when we use the default specified by the user
*/
enum SendingMode {
    SentManually,
    SentAutomatically
};

/*!
  Generates the content of the message/disposition-notification body part.
*/
[[nodiscard]] KMIME_EXPORT QByteArray dispositionNotificationBodyContent(
    const QString &finalRecipient, const QByteArray &originalRecipient,
    const QByteArray &originalMsgID, DispositionType disposition,
    ActionMode actionMode, SendingMode sendingMode,
    const QList<DispositionModifier> &dispositionModifers =
        QList<DispositionModifier>(),
    const QString &special = QString());

[[nodiscard]] KMIME_EXPORT QString descriptionFor(
    DispositionType d,
    const QList<DispositionModifier> &m = QList<DispositionModifier>());

} // namespace MDN

} // namespace KMime

