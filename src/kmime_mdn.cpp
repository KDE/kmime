/*  -*- c++ -*-
    kmime_mdn.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  provides functions for supporting Message Disposition Notifications (MDNs),
  also known as email return receipts.

  @brief
  Provides support for Message Disposition Notifications.

  @authors Marc Mutz \<mutz@kde.org\>
*/

#include "kmime_mdn.h"
#include "kmime_version.h"
#include "kmime_util.h"
#include "kmime_codecs.h"
#include "kmime_debug.h"

#include <KLocalizedString>

#include <QByteArray>

#ifdef Q_OS_WIN // gethostname
# include <winsock2.h>
#else
# include <unistd.h>
#endif

namespace KMime
{

namespace MDN
{

static const struct {
    DispositionType dispositionType;
    const char *string;
    const char *description;
} dispositionTypes[] = {
    {
        Displayed, "displayed",
        I18N_NOOP("The message sent on ${date} to ${to} with subject "
        "\"${subject}\" has been displayed. This is no guarantee that "
        "the message has been read or understood.")
    },
    {
        Deleted, "deleted",
        I18N_NOOP("The message sent on ${date} to ${to} with subject "
        "\"${subject}\" has been deleted unseen. This is no guarantee "
        "that the message will not be \"undeleted\" and nonetheless "
        "read later on.")
    },
    {
        Dispatched, "dispatched",
        I18N_NOOP("The message sent on ${date} to ${to} with subject "
        "\"${subject}\" has been dispatched. This is no guarantee "
        "that the message will not be read later on.")
    },
    {
        Processed, "processed",
        I18N_NOOP("The message sent on ${date} to ${to} with subject "
        "\"${subject}\" has been processed by some automatic means.")
    },
    {
        Denied, "denied",
        I18N_NOOP("The message sent on ${date} to ${to} with subject "
        "\"${subject}\" has been acted upon. The sender does not wish "
        "to disclose more details to you than that.")
    },
    {
        Failed, "failed",
        I18N_NOOP("Generation of a Message Disposition Notification for the "
        "message sent on ${date} to ${to} with subject \"${subject}\" "
        "failed. Reason is given in the Failure: header field below.")
    }
};

static const int numDispositionTypes =
    sizeof dispositionTypes / sizeof *dispositionTypes;

static const char *stringFor(DispositionType d)
{
    for (int i = 0 ; i < numDispositionTypes ; ++i) {
        if (dispositionTypes[i].dispositionType == d) {
            return dispositionTypes[i].string;
        }
    }
    return nullptr;
}

//
// disposition-modifier
//
static const struct {
    DispositionModifier dispositionModifier;
    const char *string;
} dispositionModifiers[] = {
    { Error, "error" },
    { Warning, "warning" },
    { Superseded, "superseded" },
    { Expired, "expired" },
    { MailboxTerminated, "mailbox-terminated" }
};

static const int numDispositionModifiers =
    sizeof dispositionModifiers / sizeof *dispositionModifiers;

static const char *stringFor(DispositionModifier m)
{
    for (int i = 0 ; i < numDispositionModifiers ; ++i) {
        if (dispositionModifiers[i].dispositionModifier == m) {
            return dispositionModifiers[i].string;
        }
    }
    return nullptr;
}

//
// action-mode (part of disposition-mode)
//

static const struct {
    ActionMode actionMode;
    const char *string;
} actionModes[] = {
    { ManualAction, "manual-action" },
    { AutomaticAction, "automatic-action" }
};

static const int numActionModes =
    sizeof actionModes / sizeof *actionModes;

static const char *stringFor(ActionMode a)
{
    for (int i = 0 ; i < numActionModes ; ++i) {
        if (actionModes[i].actionMode == a) {
            return actionModes[i].string;
        }
    }
    return nullptr;
}

//
// sending-mode (part of disposition-mode)
//

static const struct {
    SendingMode sendingMode;
    const char *string;
} sendingModes[] = {
    { SentManually, "MDN-sent-manually" },
    { SentAutomatically, "MDN-sent-automatically" }
};

static const int numSendingModes =
    sizeof sendingModes / sizeof *sendingModes;

static const char *stringFor(SendingMode s)
{
    for (int i = 0 ; i < numSendingModes ; ++i) {
        if (sendingModes[i].sendingMode == s) {
            return sendingModes[i].string;
        }
    }
    return nullptr;
}

static QByteArray dispositionField(DispositionType d, ActionMode a, SendingMode s,
                                   const QVector<DispositionModifier> &m)
{

    // mandatory parts: Disposition: foo/baz; bar
    QByteArray result = "Disposition: ";
    result += stringFor(a);
    result += '/';
    result += stringFor(s);
    result += "; ";
    result += stringFor(d);

    // optional parts: Disposition: foo/baz; bar/mod1,mod2,mod3
    bool first = true;
    for (QVector<DispositionModifier>::const_iterator mt = m.begin();
            mt != m.end() ; ++mt) {
        if (first) {
            result += '/';
            first = false;
        } else {
            result += ',';
        }
        result += stringFor(*mt);
    }
    return result + '\n';
}

static QByteArray finalRecipient(const QString &recipient)
{
    if (recipient.isEmpty()) {
        return QByteArray();
    } else {
        return "Final-Recipient: rfc822; "
               + encodeRFC2047String(recipient, "utf-8") + '\n';
    }
}

static QByteArray orginalRecipient(const QByteArray &recipient)
{
    if (recipient.isEmpty()) {
        return QByteArray();
    } else {
        return "Original-Recipient: " + recipient + '\n';
    }
}

static QByteArray originalMessageID(const QByteArray &msgid)
{
    if (msgid.isEmpty()) {
        return QByteArray();
    } else {
        return "Original-Message-ID: " + msgid + '\n';
    }
}

static QByteArray reportingUAField()
{
    char hostName[256];
    if (gethostname(hostName, 255)) {
        hostName[0] = '\0'; // gethostname failed: pretend empty string
    } else {
        hostName[255] = '\0'; // gethostname may have returned 255 chars (man page)
    }
    return QByteArray("Reporting-UA: ") + QByteArray(hostName) +
           QByteArray("; KMime " KMIME_VERSION_STRING "\n");
}

QByteArray dispositionNotificationBodyContent(const QString &r,
        const QByteArray &o,
        const QByteArray &omid,
        DispositionType d,
        ActionMode a,
        SendingMode s,
        const QVector<DispositionModifier> &m,
        const QString &special)
{
    // in Perl: chomp(special)
    QString spec;
    if (special.endsWith(QLatin1Char('\n'))) {
        spec = special.left(special.length() - 1);
    } else {
        spec = special;
    }

    // std headers:
    QByteArray result = reportingUAField();
    result += orginalRecipient(o);
    result += finalRecipient(r);
    result += originalMessageID(omid);
    result += dispositionField(d, a, s, m);

    // headers that are only present for certain disposition {types,modifiers}:
    if (d == Failed) {
        result += "Failure: " + encodeRFC2047String(spec, "utf-8") + '\n';
    } else if (m.contains(Error)) {
        result += "Error: " + encodeRFC2047String(spec, "utf-8") + '\n';
    } else if (m.contains(Warning)) {
        result += "Warning: " + encodeRFC2047String(spec, "utf-8") + '\n';
    }

    return result;
}

QString descriptionFor(DispositionType d,
                       const QVector<DispositionModifier> &)
{
    for (int i = 0 ; i < numDispositionTypes ; ++i) {
        if (dispositionTypes[i].dispositionType == d) {
            return i18n(dispositionTypes[i].description);
        }
    }
    qCWarning(KMIME_LOG) << "KMime::MDN::descriptionFor(): No such disposition type:"
               << static_cast<int>(d);
    return QString();
}

} // namespace MDN
} // namespace KMime
