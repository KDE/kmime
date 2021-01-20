
#include "kmime_headers.h"
#include "kmime_header_parsing.h"

#include <QFile>
#include <QByteArray>
#include <QMap>
#include <iostream>
#include <cstdlib>
#include <cassert>

#include <getopt.h>

using namespace KMime::HeaderParsing;
using namespace std;

static const char *tokenTypes[] = {
    "encoded-word",
    "atom",
    "token",
    "quoted-string",
    "domain-literal",
    "comment",
    "phrase",
    "dot-atom",
    "domain",
    "obs-route",
    "addr-spec",
    "angle-addr",
    "mailbox",
    "group",
    "address",
    "address-list",
    "parameter-list",
    "time",
    "date-time"
};
static const int tokenTypesLen = sizeof tokenTypes / sizeof * tokenTypes;

void usage(const char *msg = nullptr)
{
    if (msg && *msg) {
        cerr << msg << endl;
    }
    cerr <<
         "usage: test_kmime_header_parsing "
         "(--token <tokentype>|--headerfield <fieldtype>|--header)\n"
         "\n"
         "  --token <tokentype>       interpret input as <tokentype> and output\n"
         "  (-t)                      in parsed form. Currently defined values of\n"
         "                            <tokentype> are:" << endl;
    for (int i = 0 ; i < tokenTypesLen ; ++i) {
        cerr << "                               " << tokenTypes[i]
             << endl;
    }
    cerr << "\n"
         "  --headerfield <fieldtype> interpret input as header field <fieldtype>\n"
         "  (-f)                      and output in parsed form.\n"
         "\n"
         "  --header                  parse an RFC2822 header. Iterates over all\n"
         "  (-h)                      header fields and outputs them in parsed form."
         << endl;
    exit(1);
}

ostream &operator<<(ostream &stream, const QString &str)
{
    return stream << str.toUtf8().data();
}

int main(int argc, char *argv[])
{
    if (argc == 1 || argc > 3) {
        usage();
    }
    //
    // process options:
    //
    enum { None, Token, HeaderField, Header } action = None;
    const char *argument = nullptr;
    bool withCRLF = false;
    while (true) {
        int option_index = 0;
        static const struct option long_options[] = {
            // actions:
            { "token", 1, nullptr, 't' },
            { "headerfield", 1, nullptr, 'f' },
            { "header", 0, nullptr, 'h' },
            { "crlf", 0, nullptr, 'c' },
            { nullptr, 0, nullptr, 0 }
        };

        int c = getopt_long(argc, argv, "cf:ht:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'c': // --crlf
            withCRLF = true;
            break;
        case 't': // --token <tokentype>
            action = Token;
            argument = optarg;
            break;
        case 'f': // --headerfield <headertype>
            usage("--headerfield is not yet implemented!");
            break;
        case 'h': // --header
            usage("--header is not yet implemented!");
            break;
        default:
            usage("unknown option encountered!");
        }
    }

    if (optind < argc) {
        usage("non-option argument encountered!");
    }

    assert(action == Token);
    Q_UNUSED(action) // avoid warning in release mode

    int index;
    for (index = 0 ; index < tokenTypesLen ; ++index) {
        if (!qstricmp(tokenTypes[index], argument)) {
            break;
        }
    }

    if (index >= tokenTypesLen) {
        usage("unknown token type");
    }

    //QT5 KComponentData componentData( "test_kmime_header_parsing" );

    QFile stdIn;
    stdIn.open(stdin, QIODevice::ReadOnly);
    const QByteArray indata = stdIn.readAll();
    stdIn.close();
    QByteArray::ConstIterator iit = indata.begin();
    const QByteArray::ConstIterator iend = indata.end();

    switch (index) {
    case 0: {
        // encoded-word
        QString result;
        QByteArray language, charset;
        // must have checked for initial '=' already:
        bool ok = indata.size() >= 1 && *iit++ == '=' &&
                  parseEncodedWord(iit, iend, result, language, charset);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result << endl
             << "language:\n" << language.data() << endl;
    }
    break;
    case 1: {
        // atom
        cout << "with 8bit: " << endl;
        QByteArray result;
        bool ok = parseAtom(iit, iend, result, true);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result.constData()
             << endl;

        cout << "without 8bit: " << endl;
#ifdef COMPILE_FAIL
        ok = parseAtom(indata.begin(), iend, result, false);
#else
        iit = indata.begin();
        ok = parseAtom(iit, iend, result, false);
#endif

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result.constData()
             << endl;
    }
    break;
    case 2: {
        // token
        cout << "with 8bit: " << endl;
        QByteArray result;
        bool ok = parseToken(iit, iend, result, ParseTokenAllow8Bit);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result.constData()
             << endl;

        cout << "without 8bit: " << endl;
#ifdef COMPILE_FAIL
        ok = parseToken(indata.begin(), iend, result, ParseTokenNoFlag);
#else
        iit = indata.begin();
        ok = parseToken(iit, iend, result, ParseTokenNoFlag);
#endif

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result.constData()
             << endl;
    }
    break;
    case 3: {
        // quoted-string
        QString result;
        // must have checked for initial '"' already:
        bool ok = *iit++ == '"' &&
                  parseGenericQuotedString(iit, iend, result, withCRLF, '"', '"');

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result
             << endl;
    }
    break;
    case 4: {
        // domain-literal
        QString result;
        // must have checked for initial '[' already:
        bool ok = *iit++ == '[' &&
                  parseGenericQuotedString(iit, iend, result, withCRLF, '[', ']');

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result
             << endl;
    }
    break;
    case 5: {
        // comment
        QString result;
        // must have checked for initial '(' already:
        bool ok = *iit++ == '(' &&
                  parseComment(iit, iend, result, withCRLF, true);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result
             << endl;
    }
    break;
    case 6: {
        // phrase
        QString result;
        bool ok = parsePhrase(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result
             << endl;
    }
    break;
    case 7: {
        // dot-atom
        QByteArray result;
        bool ok = parseDotAtom(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result.constData()
             << endl;
    }
    break;
    case 8: {
        // domain
        QString result;
        bool ok = parseDomain(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result:\n" << result
             << endl;
    }
    break;
    case 9: {
        // obs-route
        QStringList result;
        bool ok = parseObsRoute(iit, iend, result, withCRLF, true /*save*/);

        cout << (ok ? "OK" : "BAD") << endl
             << "result: " << result.count() << " domains:"
             << endl;
        for (QStringList::ConstIterator it = result.constBegin() ;
                it != result.constEnd() ; ++it) {
            cout << (*it) << endl;
        }
    }
    break;
    case 10: {
        // addr-spec
        KMime::Types::AddrSpec result;
        bool ok = parseAddrSpec(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.localPart:\n" << result.localPart << endl
             << "result.domain:\n" << result.domain
             << endl;
    }
    break;
    case 11: {
        // angle-addr
        KMime::Types::AddrSpec result;
        bool ok = parseAngleAddr(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.localPart:\n" << result.localPart << endl
             << "result.domain:\n" << result.domain
             << endl;
    }
    break;
    case 12: {
        // mailbox
        KMime::Types::Mailbox result;
        bool ok = parseMailbox(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.displayName:\n" << result.name() << endl
             << "result.addrSpec.localPart:\n" << result.addrSpec().localPart << endl
             << "result.addrSpec.domain:\n" << result.addrSpec().domain
             << endl;
    }
    break;
    case 13: {
        // group
        KMime::Types::Address result;
        bool ok = parseGroup(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.displayName:\n" << result.displayName
             << endl;
        int i = 0;
        for (const auto &it : qAsConst(result.mailboxList)) {
            cout << "result.mailboxList[" << i << "].displayName:\n"
                 << (it).name() << endl
                 << "result.mailboxList[" << i << "].addrSpec.localPart:\n"
                 << (it).addrSpec().localPart << endl
                 << "result.mailboxList[" << i << "].addrSpec.domain:\n"
                 << (it).addrSpec().domain << endl;
            ++i;
        }
    }
    break;
    case 14: {
        // address
        KMime::Types::Address result;
        bool ok = parseAddress(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.displayName:\n"
             << endl;
        int i = 0;
        const auto mailboxList = result.mailboxList;
        for (const auto &it : mailboxList) {
            cout << "result.mailboxList[" << i << "].displayName:\n"
                 << (it).name() << endl
                 << "result.mailboxList[" << i << "].addrSpec.localPart:\n"
                 << (it).addrSpec().localPart << endl
                 << "result.mailboxList[" << i << "].addrSpec.domain:\n"
                 << (it).addrSpec().domain
                 << endl;
            ++i;
        }
    }
    break;
    case 15: {
        // address-list
        KMime::Types::AddressList result;
        bool ok = parseAddressList(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl;
        int j = 0;
        for (const auto &jt : qAsConst(result)) {
            cout << "result[" << j << "].displayName:\n"
                 << (jt).displayName
                 << endl;
            int i = 0;
            const auto mailboxList = (jt).mailboxList;
            for (const auto &it : mailboxList) {
                cout << "result[" << j << "].mailboxList[" << i << "].displayName:\n"
                     << (it).name() << endl
                     << "result[" << j << "].mailboxList[" << i << "].addrSpec.localPart:\n"
                     << (it).addrSpec().localPart << endl
                     << "result[" << j << "].mailboxList[" << i << "].addrSpec.domain:\n"
                     << (it).addrSpec().domain
                     << endl;
                ++i;
            }
            ++j;
        }
    }
    break;
    case 16: {
        // parameter-list
        QMap<QString, QString> result;
        bool ok = parseParameterList(iit, iend, result, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result: " << result.count() << " parameters:"
             << endl;
        int i = 0;
        for (QMap<QString, QString>::Iterator it = result.begin() ;
                it != result.end() ; ++it, ++i) {
            cout << "result[" << i << "].key() (attribute):\n"
                 << it.key() << endl
                 << "result[" << i << "].data() (value):\n"
                 << it.value()
                 << endl;
        }
    }
    break;
    case 17: {
        // time
        int hour, mins, secs;
        long int secsEastOfGMT;
        bool timeZoneKnown = true;

        bool ok = parseTime(iit, iend, hour, mins, secs,
                            secsEastOfGMT, timeZoneKnown, withCRLF);

        cout << (ok ? "OK" : "BAD") << endl
             << "result.hour: " << hour << endl
             << "result.mins: " << mins << endl
             << "result.secs: " << secs << endl
             << "result.secsEastOfGMT: " << secsEastOfGMT << endl
             << "result.timeZoneKnown: " << timeZoneKnown
             << endl;
    }
    break;
    case 18: {
        // date-time
        QDateTime result;
        bool ok =  parseDateTime(iit, iend, result, withCRLF);
        time_t timet = result.toSecsSinceEpoch();

        cout << (ok ? "OK" : "BAD") << endl
             << "result.time (in local timezone): " << ctime(&timet)
             << "result.secsEastOfGMT: " << result.offsetFromUtc()
             << " (" << result.offsetFromUtc() / 60 << "mins)"
             << endl;
    }
    break;
    default:
        assert(0);
    }
}
