# KMime #

[TOC]

# Introduction # {#introduction}

KMime is a library for handling mail messages and newsgroup articles. Both mail messages and
newsgroup articles are based on the same standard called MIME, which stands for
**Multipurpose Internet Mail Extensions**. In this document, the term *message* is used to
refer to both mail messages and newsgroup articles.

KMime deals solely with the in-memory representation of messages. Topics such as transport or storage
of messages are handled by other libraries, for example by [the mailtransport library](https://api.kde.org/kdepim/kmailtransport/html/index.html)
or by [the KIMAP library](https://api.kde.org/kdepim/kimap/html/index.html).
Similarly, this library does not deal with displaying messages or advanced composing, for those there
are the messageviewer and messagecomposer
components in the KDE PIM [messagelib](https://api.kde.org/kdepim/messagelib/html/index.html) module.

KMime's main function is to parse, modify and assemble messages in-memory. In a
[later section](@ref string-broken-down), *parsing* and *assembling* are actually explained.
KMime provides high-level classes that make these tasks easy.

MIME is defined by various RFCs, see the [RFC section](@ref rfcs) for a list of them.

# Structure of this document # {#structure}

This document will first give an [introduction to the MIME specification](@ref mime-intro), as it is
essential to understand the basics of the structure of MIME messages for using this library.
The introduction here is aimed at users of the library. It gives a broad overview with examples and
omits some details. Developers who wish to modify KMime should read the
[corresponding RFCs](@ref rfcs) as well, but this is not necessary for library users.

After the introduction to the MIME format, the two ways of representing a message in memory are
discussed, the [string representation and the broken down representation](@ref string-broken-down).

This is followed by a section giving an 
[overview of the most important KMime classes](@ref classes-overview).

The last sections give a list of [relevant RFCs](@ref rfcs) and provide links for
[further reading](@ref links).

# Structure of MIME messages # {#mime-intro}

## A brief history of the MIME standard ## {#history}

The MIME standard is quite new (1993), email and usenet existed way before the MIME standard came into
existence. Because of this, the MIME standard has to keep backwards compatibility. The email
standard before MIME lacked many capabilities, like encodings other than ASCII, or attachments. These
and other things were later added by MIME. The standard for messages before MIME is defined in
[RFC 5233](https://tools.ietf.org/html/rfc5322). In [RFC 2045](https://tools.ietf.org/html/rfc2045)
to [RFC 2049](https://tools.ietf.org/html/rfc2049), several backward-compatible extensions
to the basic message format are defined, adding support for attachments, different encodings and many
others.

Actually, there is an even older standard, defined in [RFC 733](https://tools.ietf.org/html/rfc733)
(*Standard for the format of ARPA network text messages*, introduced in 1977).
This standard is now obsoleted by RFC 5322, but backwards compatibility is in some cases supported, as
there are still messages in this format around.

Since pre-MIME messages had no way to handle attachments, attachments were sometimes added to the message
text in an [uuencoded](https://en.wikipedia.org/wiki/Uuencoding) form. Although this is also
obsolete, reading uuencoded attachments is still supported by KMime.

After MIME was introduced, people realized that there was no way to have the filename of attachments
encoded in anything other than ASCII. Thus, [RFC 2231](https://tools.ietf.org/html/rfc2231)
was introduced to allow arbitrary encodings for parameter values, such as the attachment filename.

## MIME by examples ## {#examples}

In the following sections, MIME message examples are shown, examined and explained, starting with
a simple message and proceeding to more interesting examples.
You can get additional examples by simply viewing the source of your own messages in your mail client,
or by having a look at the examples in the [various RFCs](@ref rfcs).

### A simple message ### {#simple-email}

    Subject: First Mail
    From: John Doe <john.doe@domain.com>
    Date: Sun, 21 Feb 2010 19:16:11 +0100
    MIME-Version: 1.0
    
    Hello World!

The above example features a very simple message. The two main parts of this message are the **header**
and the **body**, which are separated by an empty line. The body contains the actual message content,
and the header contains metadata about the message itself. The header consists of several **header fields**,
each of them in their own line. Header fields are made up from the **header field name**, followed by a colon, followed
by the **header field body**.

The **MIME-Version** header field is mandatory for MIME messages. **Subject**,
**From** and **Date** are important header fields; they are usually displayed in the message list of a
mail client. The `Subject` header field can be anything, it does not have a special structure. It is a
so-called **unstructured** header field. In contrast, the `From` and the `Date` header fields have
to follow a special structure, they must be formed in a way that machines can parse. They are **structured**
header fields. For example, a mail client needs to understand
the `Date` header field so that it can sort the messages by date in the message list.
The exact details of how the header field bodies of structured header fields should be
formed are specified in an RFC.

In this example, the `From` header contains a single email address. More precisely, a single email address is called
a **mailbox**, which is made up of the **display name** (John Doe) and the **address specification** (john.doe@domain.com),
which is enclosed in angle brackets. The `addr-spec` consists of the user name, the **local part**,
and the **domain** name.

Many header fields can contain multiple email addresses, for example the `To` field for messages with
multiple recipients can have a comma-separated list of mailboxes.
A list of mailboxes, together with a display name for the list, forms a **group**, and multiple groups can form an
**address list**. This is however rarely used, you'll most often see a simple list of plain mailboxes.

There are many more possible header fields than shown in this example, and the header can even contain
arbitrary header fields, which usually are prefixed with `X-`, like `X-Face`.

### Encodings and charsets ### {#encodings}

    From: John Doe <john.doe@domain.com>
    Date: Mon, 22 Feb 2010 00:42:45 +0100
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="iso-8859-1"
    Content-Transfer-Encoding: quoted-printable
    
    Gr=FCezi Welt!

The above shows a message that is using a different **charset** than the standard **US-ASCII** charset. The
message body contains the string "Grüezi Welt!", which is **encoded** in a special way.

The **content-type** of this message is **text/plain**, which means that the message is simple text. Later,
other content types will be introduced, such as **text/html**. If there is no `Content-Type` header
field, it is assumed that the content-type is `text/plain`.

Before MIME was introduced, all messages were limited to the US-ASCII charset. Only the
lower 127 values of the bytes were allowed to be used, the so-called **7-bit** range. Writing a message in
another charset or using letters from the upper 127 byte values was not allowed.

#### Charset Encoding ####

When talking about charsets, it is important to understand how strings of text are converted to
byte arrays, and the other way around. A message is nothing else than a big array of bytes.
The bytes that form the body of the message somehow need to be interpreted as a text string. Interpreting
a byte array as a text string is called **decoding** the text. Converting a text string to a byte array is called
**encoding** the text. A **codec** (**co**der-**dec**oder) is a utility that can encode and decode text.
In Qt, the class for text strings is QString, and the class for byte arrays is QByteArray. The base class
of all codecs is QTextCodec.

With the US-ASCII charset, encoding and decoding text is easy, one just has to look at an [ASCII table](https://en.wikipedia.org/wiki/ASCII_table)
to be able to convert text strings to byte arrays and byte arrays to text strings. For
example, the letter 'A' is represented by a single byte with the value of 65. When encountering a byte
with the value 84, we can look that up in the table and see that it represents the letter 'T'.
With the US-ASCII charset, each letter is represented by exactly one byte, which is very convenient.
Even better, all letters commonly used in English text have byte values below 127, so the 7-bit limit
of messages is no problem for text encoded with the US-ASCII charset.
Another example: The string "Hello World!" is represented by the following byte array:

    48 65 6C 6C 6F 20 57 6F 72 6C 64

Note that the byte values are written in hexadecimal form here, not in decimal as earlier.

Now, what if we want to write a message that contains German umlauts or Chinese letters? Those
are not in the ASCII table, therefore a different charset has to be used. There is a wealth of charsets
to choose from. Not all charsets can handle all letters, for example the
[ISO-8859-1](https://en.wikipedia.org/wiki/ISO-8859-1#ISO-8859-1) charset can handle
German umlauts, but cannot handle Chinese or Arabic letters. The [Unicode standard](https://en.wikipedia.org/wiki/Unicode)
is an attempt to introduce charsets that can handle all known letters in the
world, in all languages. Unicode actually has several charsets, for example [UTF-8](https://en.wikipedia.org/wiki/UTF-8)
and [UTF-16](https://en.wikipedia.org/wiki/UTF-16). In an ideal world, everyone would be using
Unicode charsets, but for historic and legacy reasons, other charsets are still much in use.

Charsets other than US-ASCII don't generally have as nice properties: A single letter can be represented
by multiple bytes, and generally the byte values are not in the 7-bit range. Pay attention to the UTF-8
charset: At first glance, it looks exactly like the US-ASCII charset, common latin letters like A - Z
are encoded with the same byte values as with US-ASCII. However, letters other than A - Z are suddenly
encoded with two or even more bytes. In general, one letter can be encoded in an abitrary number of bytes, depending
on the charset. One can **not** rely on the `1 letter == 1 byte` assumption.

Now, what should be done when the text string "Grüezi Welt!" should be sent in the body of a message?
The first step is to choose a charset that can represent all of its letters. This already excludes US-ASCII.
Once a charset is chosen, the text string is encoded into a byte array.
"Grüezi Welt!" encoded with the ISO-8859-1 charset produces the following byte array:

    47 72 FC 65 7A 69 20 57 65 6C 74 21

The letter 'ü' here is encoded using a single byte with the value `FC`.
The same string encoded with UTF-8 looks slightly different:

    47 72 C3 BC 65 7A 69 20 57 65 6C 74 21

Here, the letter 'ü' is encoded with two bytes, `C3 BC`. Still, one can see the similarity
between the two charsets for the other letters.

You can try this out yourself: Open your favorite text editor and enter some text with non-latin
letters. Then save the file and view it in a hex editor to see how the text was converted to a
byte array. Make sure to try out setting different charsets in your text editor.

At this point, the text string is successfully converted to a byte array, using e.g. the ISO-8859-1
charset. To indicate which charset was used, a **Content-Type** header field has to be added, with the correct
**charset** parameter. In our example above, that was done. If the charset parameter of the `Content-Type`,
or even the complete `Content-Type` header field is left out, the receiver can not know how to interpret
the byte array! In these cases, the byte array is usually decoded incorrectly, and the text strings contain
wrong letters or lots of question marks. There is even a special term for such wrongly decoded text,
[Mojibake](https://en.wikipedia.org/wiki/Mojibake). It is important to always know what charset
your byte array is encoded with, otherwise an attempt at decoding the byte array into a text string will fail and produce
Mojibake. **There is no such thing as plain text!** If there is no `Content-Type` header field in
a message, the message body should be interpreted as US-ASCII.

To learn more about charsets and encodings, read 
[The Absolute Minimum Every Software Developer Absolutely, Positively Must Know About Unicode and Character Sets (No Excuses!)](https://www.joelonsoftware.com/articles/Unicode.html)
and [A tutorial on character code issues](https://www.cs.tut.fi/~jkorpela/chars.html). Especially
the first article should really be read, as the name indicates.

#### Content Transfer Encoding ####

Now, we can't use the byte array that was just created in a message. The string encoded with ISO-8859-1
has the byte value `FC` for the letter 'ü', which is decimal value 252. However, as said earlier,
messages are only valid when all bytes are in the 7-bit range, i.e. have byte value below 127.
So what should we do for byte values that are greater than 127, how can they be added to messages? The solution
for this is to use a **content transfer encoding** (CTE). A content transfer encoding takes a byte
array as input and transforms it. The output is another byte array, but one which only uses byte values
in the 7-bit range. One such content transfer encoding is **quoted-printable** (QTP), which is used in the
above example. Quoted-printable is easy to understand: When encountering a byte that has a value greater
than 127, it is simply replaced by a '=', followed by the hexadecimal code of the byte value, represented
as letters and digits encoded with ASCII. This means
that a byte with the value 252 is replaced with the ASCII string `=FC`, since `FC`
is the hexadecimal value of 252. The ASCII string `=FC` itself is now three bytes big,
`3D 46 43`. Therefore, the quoted-printable encoding replaces each byte outside of the 7-bit
range with 3 new bytes. Decoding quoted-printable encoding is also easy: Each time a byte with the value
`3D`, which is the letter '=' in ASCII, is encountered, the next two following bytes are interpreted
as the hex value of the resulting byte. The quoted-printable encoding was invented to make reading the
byte array easy for humans.

The quoted-printable encoding is not a good choice when the input byte array contains lots of bytes
outside the 7-bit range, as the resulting byte array will be three times as big in the worst case,
which is a waste of space. Therefore another content transfer encoding was introduced, **Base64**.
The details of the base64 encoding are too much to write about here; refer to the
[Wikipedia article](https://en.wikipedia.org/wiki/Base64) or the [RFC](https://tools.ietf.org/html/rfc2045#section-6.8)
for details. As an example, the ISO-8859-1 encoded text string "Grüezi Welt!" is, after encoding it with base64,
represented by the following ASCII string: `R3L8ZXppIFdlbHQh`.
To express the same in byte arrays: The byte array `47 72 FC 65 7A 69 20 57 65 6C 74 21`
is, after encoding it with base64,
represented by the byte array `52 33 4C 38 5A 58 70 70 49 46 64 6C 62 48 51 68`.

There are two other content transfer encodings besides quoted printable and base64: **7-bit** and
**8-bit**. 7-bit is just a marker to indicate that no content transfer encoding is used. This is the
case when the byte array is already completely in the 7-bit range, for example when writing English
text using the US-ASCII charset. 8-bit is also a marker to indicate that no content transfer encoding
was used. This time, not because it was not necessary, but because of a special exception, byte values
outside of the 7-bit range are allowed. For example, some SMTP servers support the
[8BITMIME](https://tools.ietf.org/html/rfc1652) extension, which indicates that they accept
bytes outside of the 7-bit range. In this case, one can simply use the byte arrays as-is, without using
any content transfer encoding. Creating messages with 8-bit content transfer encoding is currently not
supported by KMime. The advantage of 8-bit is that there is no overhead in size, unlike with
base64 or even quoted-printable.

When using one of the 4 contents transfer encodings, i.e. quoted-printable, base64, 7-bit or 8-bit, this
has to be indicated in the header field **Content-Transfer-Encoding**. If the header field is left out,
it is assumed that the content transfer encoding is 7-bit. The example above uses quoted-printable.

    From: John Doe <john.doe@domain.com>
    Date: Mon, 22 Feb 2010 00:42:45 +0100
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="iso-8859-1"
    Content-Transfer-Encoding: base64
    
    R3L8ZXppIFdlbHQh

The same example, this time encoded with the base64 content transfer encoding.

    From: John Doe <john.doe@domain.com>
    Date: Mon, 22 Feb 2010 00:42:45 +0100
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="utf-8"
    Content-Transfer-Encoding: base64
    
    R3LDvGV6aSBXZWx0IQ==

Again the same example, this time using UTF-8 as the charset.

    From: John Doe <john.doe@domain.com>
    Date: Mon, 22 Feb 2010 00:42:45 +0100
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="utf-8"
    Content-Transfer-Encoding: quoted-printable

    Gr=C3=BCezi Welt!

The example with a combination of UTF-8 and quoted-printable CTE. As said somewhere above, with the
UTF-8 encoding, the letter 'ü' is represented by the two bytes `C3 BC`.

    From: John Doe <john.doe@domain.com>
    Date: Mon, 22 Feb 2010 00:42:45 +0100
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="utf-8"
    Content-Transfer-Encoding: 7-bit

    Hello World

A different example, showing 7-bit content transfer encoding. Although the UTF-8 charset has lots
of letters that are represented by bytes outside of the 7-bit range, the string "Hello World" can
be fully represented in the 7-bit range here, even with UTF-8.

In the [further reading](@ref links) section, you will find links to web applications that demonstrate
encodings and charsets.

#### Conclusion ####

When adding a text string to the body of a message, it needs to be encoded twice: First, the encoding of the charset
needs to be applied, which transforms the text string into a byte array. Afterwards, the content transfer
encoding has to be applied, which transforms the byte array from the first step into a byte array that
only has bytes in the 7-bit range.

When decoding, the same has to be done, in reverse: One first has decode the byte array with the content transfer encoding, to get a byte
array that has all 256 possible byte values. Afterwards, the resulting byte array needs to be decoded
with the correct charset, to transform it into a text string. For those two decoding steps, one has to
look at the `Content-Type` and the `Content-Transfer-Encoding` header fields to find the correct
charset and CTE for decoding.

It is important to always keep the charset and the content transfer encoding in mind. Byte arrays and
strings are not to be confused. Byte arrays that are encoded with a CTE are not to be confused with
byte arrays that are **not** encoded with a CTE.

This section showed how to use different charsets in the *body* of a message. The next section will
show what to do when another charset is needed in one of the *header* field bodies.

### Encoding in Header Fields ### {#header-encoding}

In the last section, we discussed how to use different charsets in the body of a message. But what if
a different charset needs to be added to one of the header fields? For example one might want to write
a mail to a mailbox with the display name "András Manţia" and with the subject "Grüezi!".

The header fields are limited to characters in the 7-bit range, and are interpreted as US-ASCII.
That means the header field names, such as "From: ", are all encoded in US-ASCII. The header field
bodies, such as the "1.0" of `MIME-Version`, are also encoded with US-ASCII. This is mandated by
[the RFC](https://tools.ietf.org/html/rfc5322#section-2).

The `Content-Type` and the `Content-Transfer-Encoding` header fields only apply to the message body,
they have no meaning for other header fields.

This means that any letter in a different charset has to be encoded in some way to satisfy the RFC.
Letters with a different charset are only allowed in some of the header field bodies; the header field
names always have to be in US-ASCII.

    From: Thomas McGuire <thomas@domain.com>
    Subject: =?iso-8859-1?q?Gr=FCezi!?=
    Date: Mon, 22 Feb 2010 14:34:01 +0100
    MIME-Version: 1.0
    To: =?utf-8?q?Andr=C3=A1s?= =?utf-8?q?_Man=C5=A3ia?= <andras@domain.com>
    Content-Type: Text/Plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    bla bla bla

The above example shows how text that is encoded with a different charset than US-ASCII is handled
in the message header. This can be seen in the bodies of the `Subject` header field and the `To` header field.
In this example, the body of the message is unimportant, it is just "bla bla bla" in US-ASCII.
The way the header field bodies are encoded is sometimes referred to as a **RFC2047 string** or as an **encoded word**, which has
its origin in the [RFC](https://tools.ietf.org/html/rfc2047) where this encoding scheme is defined.
RFC2047 strings are only allowed in some of the header fields, like `Subject`, and in the display name
of mailboxes in header fields like `From` and `To`. In other header fields, such as `Date` and
`MIME-Version`, they are not allowed, but they wouldn't make much sense there anyway, since those are
structured header fields with a clearly defined structure.

RFC2047 strings start with "=?" and end with "?=". Between those markers, they consists of three parts:
* The charset, such as "iso-8859-1"
* The encoding, which is "q" or "b"
* The encoded text

These three parts are separated with a '?'. Encoding the third part, the text, is very similar to how
text strings in the message body are encoded: First, the text string is encoded to a byte array using
the charset encoding. Afterwards, the second encoding is used on the result, to ensure that all resulting
bytes are within the 7-bit range.

The *second encoding* here is almost identical to the content transfer encoding. There are two
possible encodings, **b** and **q**. The `b` encoding is the same as the base64 encoding of the content
transfer encoding. The `q` encoding is very similar to the quoted-printable encoding of the content
transfer encoding, but with some little differences that are described in
[the RFC](https://tools.ietf.org/html/rfc2047#section-4.2).

Let's examine the subject of the message, `=?iso-8859-1?q?Gr=FCezi!?=`, in detail:

The first part of the RFC2027 string is the charset, so it is ISO-8859-1 in this case. The second part
is the encoding, which is the `q` encoding here. The last part is the encoded text, which is
`Gr=FCezi!`. As with the quoted-printable encoding, "=FC" is the encoding for the byte with
the value `FC`, which in the ISO-8859-1 charset is the letter 'ü'. The complete decoded
text is therefore "Grüezi!".

Each RFC2047 string in the header can use a different charset: In this example, the `Subject` uses ISO-8859-1,
`To` uses UTF-8 and the message body uses US-ASCII.

In the `To` header field, two RFC2047 strings are used. A single, bigger, RFC2047 string for the whole
display name could also have been used. In this case, the second RFC2047 string starts with an underscore,
which is decoded as a space in the `q` encoding. The space between the two RFC2047 strings is ignored,
it is just used to separate the two encoded words.

There are some restriction on RFC2047 strings: They are not allowed to be longer than 75 characters,
which means two or more encoded words have to be used for long text strings. Also, there are some
restrictions on where RFC2047 strings are allowed; most importantly, the address specification must
not be encoded, to be backwards compatible. For further details, refer to the RFC.

### Messages with attachments ### {#multipart-mixed}

Until now, we only looked at messages that had a single text part as the message body. In this section,
we'll examine messages with attachments.

    From: frank@domain.com
    To: greg@domain.com
    Subject: Nice Photo
    Date: Sun, 28 Feb 2010 19:57:00 +0100
    MIME-Version: 1.0
    Content-Type: Multipart/Mixed;
      boundary="Boundary-00=_8xriL5W6LSj00Ly"
    
    --Boundary-00=_8xriL5W6LSj00Ly
    Content-Type: Text/Plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Hi Greg,
    
    attached you'll find a nice photo.
    
    --Boundary-00=_8xriL5W6LSj00Ly
    Content-Type: image/jpeg;
      name="test.jpeg"
    Content-Transfer-Encoding: base64
    Content-Disposition: attachment;
      filename="test.jpeg"
    
    /9j/4AAQSkZJRgABAQAAAQABAAD/4Q3XRXhpZgAASUkqAAgAAAAHAAsAAgAPAAAAYgAAAAABBAAB
    [SNIP 800 lines]
    ze5CdSH2Z8yTatHSV2veW0rKzeq30//Z
    
    --Boundary-00=_8xriL5W6LSj00Ly--

*Note: Since the image in this message would be really big, most of it is omitted / snipped here.*

The above example consists of two parts: A normal text part and an image attachment. Messages that
consist of multiple parts are called **multipart** messages. The top-level content-type therefore is
**multipart/mixed**. `Mixed` simply means that the following parts have no relation to each other,
it is just a random mixture of parts. Later, we will look at other types, such as `multipart/alternative`
or `multipart/related`. A **part** is sometimes also called **node**, **content** or **MIME part**.

Each MIME part of the message is separated by a **boundary**, and that boundary
is specified in the top-level content-type header as a parameter. In the message body, the boundary
is prefixed with `"--"`, and the last boundary is suffixed with `"--"`, so that the end of the message can
be detected. When creating a message, care must be taken that the boundary appears nowhere else in the
message, for example in the text part, as the parser would get confused by this.

A MIME part begins right after the boundary. It consists of a **MIME header** and a **MIME body**, which
are separated by an empty line. The MIME header should not be confused with the message header: The
message header contains metadata about the whole message, like subject and date. The MIME header only
contains metadata about the specific MIME part, like the content type of the MIME part. MIME header
field names always start with `"Content-"`.
The example above shows the three most important MIME header fields. Usually those are the only ones
used. The top-level header of a message actually mixes the message metadata and the MIME metadata into one header: In this
example, the header contains the `Date` header field, which is an ordinary header field, and it contains
the `Content-Type` header field, which is a MIME header field.

MIME parts can be nested, and therefore form a tree. The above example has the following tree:

    multipart/mixed
    |- text/plain
    \- image/jpeg

The `text/plain` node is therefore a `child` of the `multipart/mixed` node. The `multipart/mixed` node
is a `parent` of the other two nodes. The `image/jpeg` node is a **sibling** of the `text/plain` node.
`Multipart` nodes are the only nodes that have children, other nodes are **leaf** nodes.
The body of a multipart node consists of all complete child nodes (MIME header and MIME body), separated
by the boundary.

Each MIME part can have a different content transfer encoding. In the above example, the text part has
a `7bit` CTE, while the image part has a `base64` CTE. The multipart/mixed node does not specify
a CTE, multipart nodes always have `7bit` as the CTE. This is because the body of multipart nodes can
only consist of bytes in the 7 bit range: The boundary is 7 bit, the MIME headers are 7 bit, and the
MIME bodies are already encoded with the CTE of the child MIME part, and are therefore also 7 bit. This means
no CTE for multipart nodes is necessary.

The MIME part for the image does not specify a charset parameter in the content type header field. This
is because the body of that MIME part will not be interpreted as a text string, therefore the byte array
does not need to be decoded to a string. Instead, the byte array is interpreted as an image, by an image
renderer. The message viewer application passes the MIME part body as a byte array to the image renderer.
The content type consists of a **media type** and a **subtype**. For example, the content type
`"text/html"` has the media type "text" and the subtype "html". Only nodes that have the media type "text"
need to specify a charset, as those nodes are the only nodes of which the body is interpreted as a text string.

The only header field not yet encountered in previous sections is the **Content-Disposition** header field,
which is defined in a [separate RFC](https://tools.ietf.org/html/rfc2183). It describes how
the message viewer application should display the MIME part. In the case of the image part, is should
be presented as an attachment. The **filename** parameter tells the message viewer application which filename
should be used by default when the user saves the attachment to disk.

The content type header field for the image MIME part has a **name** parameter, which is similar to the
`filename` parameter of the `Content-Disposition` header field. The difference is that `name` refers
to the name of the complete MIME part, whereas `filename` refers to the name of the attachment. The
`name` parameter of the `Content-Type` header field in this case is superfluous and only exists for
backwards compatibility, and can be ignored;
the `filename` parameter of the `Content-Disposition` header field should be preferred when it is present.

    From: Thomas McGuire <thomas@domain.com>
    To: sebastian@domain.com
    Subject: Help with SPARQL
    Date: Sun, 28 Feb 2010 21:57:51 +0100
    MIME-Version: 1.0
    Content-Type: Multipart/Mixed;
      boundary="Boundary-00=_PjtiLU2PvHpvp/R"
    
    --Boundary-00=_PjtiLU2PvHpvp/R
    Content-Type: Text/Plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Hi Sebastian,
    
    I have a problem with a SPARQL query, can you help me debug this? Attached is
    the query and a screenshot showing the result.
    
    --Boundary-00=_PjtiLU2PvHpvp/R
    Content-Type: text/plain;
      charset="UTF-8";
      name="query.txt"
    Content-Transfer-Encoding: 7bit
    Content-Disposition: attachment;
      filename="query.txt"
    
    prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>
    
    SELECT ?person
    WHERE {
     ?person a nco:PersonContact .
     ?person nco:birthDate ?birthDate .
    }"
    --Boundary-00=_PjtiLU2PvHpvp/R
    Content-Type: image/png;
      name="screenshot.png"
    Content-Transfer-Encoding: base64
    Content-Disposition: attachment;
      filename="screenshot.png"
    
    AAAAyAAAAAEBBAABAAAAyAAAAA0BAgATAAAAcQAAABIBAwABAAAAAQAAADEBAgAPAAAAhAAAAGmH
    [SNIP]
    YXJlLmpwZWcAZGlnaUthbS0w
    
    --Boundary-00=_PjtiLU2PvHpvp/R--

The above example message consists of three MIME parts: The main text part and two attachments.
One attachment has the media type `text`, therefore a charset parameter is necessary to correctly
display it. The MIME tree looks like this:

    multipart/mixed
    |- text/plain
    |- text/plain
    \- image/jpeg

### HTML Messages ### {#multipart-alternative}

    From: Thomas McGuire <thomas@domain.com>
    Subject: HTML test
    Date: Thu, 4 Mar 2010 13:59:18 +0100
    MIME-Version: 1.0
    Content-Type: multipart/alternative;
      boundary="Boundary-01=_m66jLd2/vZrH5oe"
    Content-Transfer-Encoding: 7bit
    
    --Boundary-01=_m66jLd2/vZrH5oe
    Content-Type: text/plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Hello World
    
    --Boundary-01=_m66jLd2/vZrH5oe
    Content-Type: text/html;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
    <html>
      <head></head>
      <body>
        Hello <b>World</b>
      </body>
    </html>
    --Boundary-01=_m66jLd2/vZrH5oe--

The above example is a simple HTML message. It consists of a plain text and a HTML part, which are
in a **multipart/alternative** container. The message has the following structure:

    multipart/alternative
    |- text/plain
    \- text/html

The HTML part and the plain text part have the identical content, except that the HTML part contains
additional markup, in this case for displaying the word `World` in bold. Since those parts are in a
multipart/alternative container, the message viewer application can freely choose which part it displays.
Some users might prefer reading the message in HTML format, some might prefer reading the message
in plain text format.

Of course, a HTML message could also consist only of a single `text/html`, without the multipart/alternative
container and therefore without an alternative plain text part. However, people preferring the plain
text version wouldn't like this, especially if their mail client has no HTML engine and they would see
the HTML source including all tags only. Therefore, HTML messages should always include an alternative plain text part.

HTML messages can of course also contain attachments. In this case, the message contains both a
multipart/alternative and a multipart/mixed node, for example with the following structure, for a HTML
message that has an image attachment:

    multipart/mixed
    |- multipart/alternative
    |  |- text/plain
    |  \- text/html
    \- image/png

The message itself would look like this:

    From: Thomas McGuire <thomas@domain.com>
    Subject: HTML message with an attachment
    Date: Thu, 4 Mar 2010 15:20:26 +0100
    MIME-Version: 1.0
    Content-Type: Multipart/Mixed;
      boundary="Boundary-00=_qG8jLwWCwkUfJV1"
    
    --Boundary-00=_qG8jLwWCwkUfJV1
    Content-Type: multipart/alternative;
      boundary="Boundary-01=_qG8jLfs1FRmlOhl"
    Content-Transfer-Encoding: 7bit
    
    --Boundary-01=_qG8jLfs1FRmlOhl
    Content-Type: text/plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Hello World
    
    --Boundary-01=_qG8jLfs1FRmlOhl
    Content-Type: text/html;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
    <html>
      <head></head>
      <body>
        Hello <b>World</b>
      </body>
    </html>
    --Boundary-01=_qG8jLfs1FRmlOhl--
    
    --Boundary-00=_qG8jLwWCwkUfJV1
    Content-Type: image/png;
      name="test.png"
    Content-Transfer-Encoding: base64
    Content-Disposition: attachment;
      filename="test.png"
    
    iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAA8SAAAPEgEhm/IzAAAC
    [SNIP]
    eFkXsFgBMG4fJhYlx+iyB3cLpNZwYr/iP7teTwNYa7DZAAAAAElFTkSuQmCC
    
    --Boundary-00=_qG8jLwWCwkUfJV1--

### HTML Messages with Inline Images ### {#multipart-related}

HTML has support for showing images, with the `img` tag. Such an image is shown at the place where
the `img` tag occurs, which is called an **inline image**. Note that inline images are different
from images that are just normal attachments: Normal attachments are always shown at the beginning or
at the end of the message, while inline images are shown in-place. In HTML, the `img` tag points to an
image file that is either a file on disk or a URL of an image on the Internet. To make inline images
work with MIME messages, a different mechanism is needed, since the image is not a file on disk or on
the Internet, but a MIME part somewhere in the same message. As specified in
[RFC 2557](https://tools.ietf.org/html/rfc2557), the way this can be done is by referring
to a **Content-ID** in the `img` tag, and marking the MIME part that is the image with that content
ID as well.

An example will probably be more clear than this explanation:

    From: Thomas McGuire <thomas@domain.com>
    Subject: Inine Image Test
    Date: Thu, 4 Mar 2010 16:54:53 +0100
    MIME-Version: 1.0
    Content-Type: multipart/related;
      boundary="Boundary-02=_Nf9jLpJ2aGp5RQK"
    Content-Transfer-Encoding: 7bit
    
    --Boundary-02=_Nf9jLpJ2aGp5RQK
    Content-Type: multipart/alternative;
      boundary="Boundary-01=_Nf9jLZ6aPhm3WrN"
    Content-Transfer-Encoding: 7bit
    Content-Disposition: inline
    
    --Boundary-01=_Nf9jLZ6aPhm3WrN
    Content-Type: text/plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Text before image
    
    Text after image
    
    --Boundary-01=_Nf9jLZ6aPhm3WrN
    Content-Type: text/html;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
    <html>
      <head></head>
      <body>
        Text before image<br>
        <img src="cid:547730348@KDE" /><br>
        Text after image
      </body>
    </html>
    --Boundary-01=_Nf9jLZ6aPhm3WrN--
    
    --Boundary-02=_Nf9jLpJ2aGp5RQK
    Content-Type: image/png;
      name="test.png"
    Content-Transfer-Encoding: base64
    Content-Id: <547730348@KDE>
    
    iVBORw0KGgoAAAANSUhEUgAAAMgAAADICAIAAAAiOjnJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAg
    [SNIP]
    AABJRU5ErkJggg==
    --Boundary-02=_Nf9jLpJ2aGp5RQK--

The first thing you'll notice in this example probably is that it has a **multipart/related** node with
the following structure:

    multipart/related
    |- multipart/alternative
    |  |- text/plain
    |  \- text/html
    \- image/png

When the HTML part has inline image, the HTML part and its image part both have to be children of a
multipart/related container, like in this example.
In this case, the `img` tag has the source `cid:547730348@KDE`, which is a placeholder that refers
to the Content-Id header of another part. The image part contains exactly that value in its `Content-Id`
header, and therefore a message viewer application can connect both.

The plain text part cannot have inline images, therefore its text might seem a bit confusing.

HTML messages with inline images can of course also have attachments, in which the message structure
becomes a mix of multipart/related, multipart/alternative and multipart/mixed. The following example
shows the structure of a message with two inline images and one `.tar.gz` attachment:

    multipart/mixed
    |- multipart/related
    |  |- multipart/alternative
    |  |  |- text/plain
    |  |  \- text/html
    |  |- image/png
    |  \- image/png
    \- application/x-compressed-tar

The structure of MIME messages can get arbitrarily complex, the above is just one relatively simple example.
The nesting of multipart nodes can get much deeper, there is no restriction on nesting levels.

### Encapsulated messages ### {#encapsulated}

Encapsulated messages are messages which are attachments to another message. The most common example
is a forwarded mail, like in this example:

    From: Frank <frank@domain.com>
    To: Bob <bob@domain.com>
    Subject: Fwd: Blub
    MIME-Version: 1.0
    Content-Type: Multipart/Mixed;
      boundary="Boundary-00=_sX+jLVPkV1bLFdZ"
    
    --Boundary-00=_sX+jLVPkV1bLFdZ
    Content-Type: text/plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Hi Bob,
    
    hereby I forward you an interesting message from Greg.
    
    --Boundary-00=_sX+jLVPkV1bLFdZ
    Content-Type: message/rfc822;
      name="forwarded message"
    Content-Transfer-Encoding: 7bit
    Content-Description: Forwarded Message
    Content-Disposition: inline
    
    From: Greg <greg@domain.com>
    To: Frank <frank@domain.com>
    Subject: Blub
    MIME-Version: 1.0
    Content-Type: Text/Plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Bla Bla Bla
    
    --Boundary-00=_sX+jLVPkV1bLFdZ--


    multipart/mixed
    |- text/plain
    \- message/rfc822
    \- text/plain

The attached message is treated like any other attachment, and therefore the top-level content type
is multipart/mixed.
The most interesting part is the `message/rfc822` MIME part. As usual, it has some MIME headers, like
`Content-Type` or `Content-Disposition`, followed by the MIME body. The MIME body in this case is
the attached message. Since it is a message, it consists of a header and a body itself.
Therefore, the `message/rfc822` MIME part appears to have two headers; in reality, it is the normal
MIME header and the message header of the encapsulated message. The message header and the message body
are both in the MIME body of the `message/rfc822` MIME part.

### Signed and Encrypted Messages ### {#crypto}

MIME messages can be cryptographically signed and/or encrypted. The format for those messages is
defined in [RFC 1847](https://tools.ietf.org/html/rfc1847), which specifies two new
multipart subtypes, **multipart/signed** and **multipart/encrypted**. The crypto format of these new
security multiparts is defined in additional RFCs; the most common formats are
[OpenPGP](https://tools.ietf.org/html/rfc3156) and [S/MIME](https://tools.ietf.org/html/rfc2633).
Both formats use the principle of [public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography).
OpenPGP uses **keys**, and S/MIME uses **certificates**. For easier text flow, only the term `key` will be used
for both keys and certificates in the text below.

Security multiparts only sign or encrypt a specific MIME part. The consequence is that the message headers
can not be signed or encrypted. Also this means that it is possible to sign or encrypt only some of
the MIME parts of a message, while leaving other MIME parts unsigned or unencrypted. Furthermore, it
is possible to sign or encrypt different MIME parts with different crypto formats. As you can see,
security multiparts are very flexible.

Security multiparts are not supported by KMime. However, it is possible for applications to use KMime
when providing support for crypto messages. For example, the messageviewer
component in KDE PIM's [messagelib](https://api.kde.org/kdepim/messagelib/html/index.html) supports signed and encrypted MIME parts, and the
messagecomposer library can create
such messages.

Signed MIME parts are signed with the private key of the sender, and everybody who has the
public key of the sender can verify the signature. Encrypted MIME parts are encrypted with the public
key of the receiver, and only the receiver, who is the sole person possessing the private key, can decrypt
it. Sending an encrypted message to multiple recipients therefore means that the message has to be sent
multiple times, once for each receiver, as each message needs to be encrypted with a different key.

#### Signed MIME parts ####

A multipart/signed MIME part has exactly two children: The first child is the content that is signed,
and the second child is the signature.

    From: Thomas McGuire <thomas@domain.com>
    Subject: My Subject
    Date: Mon, 15 Mar 2010 12:20:16 +0100
    MIME-Version: 1.0
    Content-Type: multipart/signed;
      boundary="nextPart2567247.O5e8xBmMpa";
      protocol="application/pgp-signature";
      micalg=pgp-sha1
    Content-Transfer-Encoding: 7bit
    
    --nextPart2567247.O5e8xBmMpa
    Content-Type: Text/Plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    
    Simple message
    
    --nextPart2567247.O5e8xBmMpa
    Content-Type: application/pgp-signature; name=signature.asc
    Content-Description: This is a digitally signed message part.
    
    -----BEGIN PGP SIGNATURE-----
    Version: GnuPG v2.0.14 (GNU/Linux)
    
    iEYEABECAAYFAkueF/UACgkQKglv3sO8a1MdTACgnBEP6ZUal931Vwu7PyiXT1bn
    Zr0Anj4bAI9JhHEDiwA/iwrWGfSC+Nlz
    =d2ol
    -----END PGP SIGNATURE-----
    --nextPart2567247.O5e8xBmMpa--


    multipart/signed
    |- text/plain
    \- application/pgp-signature

The example here uses the OpenPGP format to sign a simply plain text message. Here, the text/plain
MIME part is signed, and the application/pgp-signature MIME part contains the signature data, which in
this case is ASCII-armored.

As said above, it is possible to sign only some MIME parts. A message which has a image/jpeg attachment
that is signed, but a main text part is not signed, has the following MIME structure:

    multipart/mixed
    |- text/plain
    \- multipart/signed
    |- image/jpeg
    \- application/pgp-signature

It is possible to sign multipart parts as well. Consider the above example that has a plain text part
and an image attachment. Those two parts can be signed together, with the following structure:

    multipart/signed
    |- multipart/mixed
    |  |- text/plain
    |  \- image/jpeg
    \- application/pgp-signature

Signed messages in the S/MIME format use a different content type for the signature data, like here:

    multipart/signed
    |- text/plain
    \- application/x-pkcs7-signature

#### Encrypted MIME parts ####

Multipart/encrypted MIME parts also have exactly two children: The first child contains metadata about
the encrypted data, such as a version number. The second child then contains the actual encrypted data.

    From: someone@domain.com
    To: Thomas McGuire <thomas@domain.com>
    Subject: Encrypted message
    Date: Mon, 15 Mar 2010 12:50:16 +0100
    MIME-Version: 1.0
    Content-Type: multipart/encrypted;
      boundary="nextPart2726747.j47xUGTWKg";
      protocol="application/pgp-encrypted"
    Content-Transfer-Encoding: 7bit
    
    --nextPart2726747.j47xUGTWKg
    Content-Type: application/pgp-encrypted
    Content-Disposition: attachment
    
    Version: 1
    --nextPart2726747.j47xUGTWKg
    Content-Type: application/octet-stream
    Content-Disposition: inline; filename="msg.asc"
    
    -----BEGIN PGP MESSAGE-----
    Version: GnuPG v2.0.14 (GNU/Linux)
    
    hQIOA8p5rdC5CBNfEAf+NZVzVq48C1r5opOOiWV96+FUzIWuMQ6u8fzFgI7YVyCn
    [SNIP]
    =reNr
    --nextPart2726747.j47xUGTWKg--
    -----END PGP MESSAGE-----


    multipart/encrypted
    |- application/pgp-encrypted
    \- application/octet-stream

The encrypted data is contained in the `application/octet-stream` MIME part. Without decrypting
the data, it is unknown what the original content type of the encrypted MIME data is! The encrypted
data could be a simple text/plain MIME part, an image attachment, or a multipart part. The encrypted
data contains both the MIME header and the MIME body of the original MIME part, as the header is needed
to know the content type of the data. The data could as well be of content type multipart/signed, in
which case the message would be both signed and encrypted.

#### Inline Crypto Formats ####

Although using the security multiparts `multipart/signed` and `multipart/encrypted` is the recommended
standard, there are other possibilities to sign or encrypt a message. The most common methods are
**Inline OpenPGP** and **S/MIME Opaque**.

For inline OpenPGP messages, the crypto data is contained inlined in the actual MIME part. For example,
a message with a signed text/plain part might look like this:

    From: someone@domain.com
    To: someoneelse@domain.com
    Subject: Inline OpenPGP test
    MIME-Version: 1.0
    Content-Type: text/plain;
      charset="us-ascii"
    Content-Transfer-Encoding: 7bit
    Content-Disposition: inline
    
    -----BEGIN PGP SIGNED MESSAGE-----
    Hash: SHA1
    
    Inline OpenPGP signed example.
    -----BEGIN PGP SIGNATURE-----
    Version: GnuPG v2.0.14 (GNU/Linux)
    
    iEYEARECAAYFAkueJ2EACgkQKglv3sO8a1MS3QCfcsYnJG7uYQxzxz6J5cPF7lHz
    WIoAn3PjVPlWibu02dfdFObwd2eJ1jAW
    =p3uO
    -----END PGP SIGNATURE-----

Encrypted inline OpenPGP works in a similar way. Opaque S/MIME messages are also similar: For signed
MIME parts, both the signature and the signed data are contained in a single MIME part with a content
type of `application/pkcs7-mime`.

As security multiparts are preferred over inline OpenPGP and over opaque S/MIME, I won't go into more
detail here.

### Miscellaneous Points about Messages ### {#misc}

#### Line Breaks ####

Each line in a MIME message has to end with a **CRLF**, which is a carriage return followed by a
newline, which is the escape sequence `\\r\\n`. CR and LF may not appear in other places in
a MIME message. Special care needs to be taken with encoded line breaks in binary data, and with
distinguishing soft and hard line breaks when converting between different content transfer encodings.
For more details, have a look at the RFCs.

While the official format is to have a CRLF at the end of each line, KMime only expects a single LF
for its in-memory storage. Therefore, when loading a message from disk or from a server into KMime, the CRLFs need
to be converted to LFs first, for example with KMime::CRLFtoLF(). The opposite needs to be done when
storing a KMime message somewhere.

Lines should not be longer than 78 characters and may not be longer than 998 characters.

#### Header Folding and CFWS ####

Header fields can span multiple lines, which was already shown in some of the examples above where
the parameters of the header field value were in the next line. The header field is said to be
**folded** in this case. In general, header fields can be folded whenever whitespace (**WS**) occurs.

Header field values can contain **comments**; these comments are semantically invisible and have no
meaning. Comments are surrounded by parentheses.

    Date: Thu, 13
          Feb 1969 23:32 -0330 (Newfoundland Time)

This example shows a folded header that also has a comment (*Newfoundland Time*). The date header is a structured header
field, and therefore it has to obey to a defined syntax; however, adding comments and whitespace is
allowed almost anywhere, and they are ignored when parsing the message. Comments and whitespace where
folding is allowed is sometimes referred to as **CFWS**. Any occurrence of CFWS is semantically regarded
as a single space.

# The two in-memory representations of messages # {#string-broken-down }

There are two representations of messages in memory. The first is called **string representation**
and the other one is called **broken-down representation**.

String representation is somewhat misnamed,
a better term would be "byte array representation". The string representation is just a big array of
bytes in memory, and those bytes make up the encoded mail. The string representation is what is stored
on disk or what is received from an IMAP server, for example.

With the broken-down representation, the mail is *broken down* into smaller structures. For example,
instead of having a single byte array for all headers, the broken-down structure has a list of individual headers,
and each header in that list is again broken down into a structure. While the string representation
is just an array of 7 bit characters that might be encoded, the broken-down representations contain the
decoded text strings.

As an example, consider the byte array

    "Hugo Maier" <hugo.maier@mailer.domain>

Although this is just a bunch of 7 bit characters, a human immediately recognizes the broken-down structure and
sees that the display name is "Hugo Maier" and that the localpart of the email address is "hugo.maier".
To illustrate, the broken-down structure could be stored in a structure like this:

    struct Mailbox
    {
        QString displayName;
        QByteArray addressSpec;
    };

The address spec actually could be broken down further into a localpart and a domain.
The process of converting the string representation to a broken-down representation is called **parsing**, and
the reverse is called **assembling**. Parsing a message is necessary when wanting to access or modify the broken-down
structure. For example, when sending a mail,
the address spec of a mailbox needs to be passed to the SMTP server, which means that the recipient headers need to
be parsed in order to access that information. Another example is the message list in an mail application, where the
broken-down structure of a mail is needed
to display information like subject, sender and date in the list.
On the other hand, assembling a message is for example done in the composer of a mail application, where the mail information
is available in a broken-down form in the composer window, and is then assembled into a final MIME message that is then sent with SMTP.

Parsing is often quite tricky. You should always use the methods from KMime instead of writing parsing
routines yourself. Even the simple mailbox example above is in practice difficult to parse, as many things like comments
and escaped characters need to be taken into consideration.
The same is true for assembling: In the above case, one could be tempted to assemble the mailbox by simply
writing code like this:

    QByteArray stringRepresentation = '"' + displayName + "\" <" + addressSpec + ">";

However, just like with parsing, you shouldn't be doing assembling yourself. In the above case, for example,
the display name might contain non-ASCII characters, and RFC2047 encoding would need to be applied. So use
KMime for assembling in all cases.

When parsing a message and assembling it afterwards, the result might not be the same as the original byte
array. For example, comments in header fields are ignored during parsing and not stored in the broken-down
structure, therefore the assembled message will also not contain comments.

Messages in memory are usually stored in a broken-down structure so that it is easy to to access and
manipulate the message. On disk and on servers, messages are stored in string representation.

# Overview of KMime classes # {#classes-overview}

KMime has basically two sets of classes: Classes for headers and classes for MIME
parts. A MIME part is represented by `KMime::Content`. A Content can be parsed from a string representation
and also be assembled from the broken-down representation again. If parsed, it has a list of sub-contents (in case of multipart contents) and a
list of headers. If the Content is not parsed, it stores the headers and the body in a byte array, which can be accessed
with head() and body().
There is also a class `KMime::Message`, which basically is a thin wrapper around Content for the top-level
MIME part. Message also contains convenience methods to access the message headers.

For headers, there is a class hierarchy, with `KMime::Headers::Base` as the base class, and
`KMime::Headers::Generics::Structured` and `KMime::Headers::Generics::Unstructured` in the next levels. Unstructured is
for headers that don't have a defined structure, like Subject, whereas Structured headers have a
specific structure, like Date. The header classes have methods to parse headers, like `from7BitString()`,
and to assemble them, like `as7BitString()`. Once a header is parsed, the classes provide access to the
broken-down structures; for example the `Date` header has a method `dateTime()`.
The parsing in `from7BitString()` is usually handled by a protected `parse()` function, which in turn call
parsing functions for different types, like `parseAddressList()` or `parseAddrSpec()` from the `KMime::HeaderParsing`
namespace.

When modifying messages, the message is first parsed into a broken-down representation. This broken-down
representation can then be accessed and modified with the appropriate functions. After changing the broken-down
structure, it needs to be assembled again to get the modified string representation.

KMime also comes with some codes for handling base64 and quoted-printable encoding, with `KMime::Codec`
as the base class.

# RFCs # {#rfcs}

* [RFC 5322](https://tools.ietf.org/html/rfc5322): Internet Message Format
* [RFC 5536](https://tools.ietf.org/html/rfc5536): Netnews Article Format
* [RFC 2045](https://tools.ietf.org/html/rfc2045): Multipurpose Internet Mail Extensions (MIME), Part 1: Format of Internet Message Bodies
* [RFC 2046](https://tools.ietf.org/html/rfc2046): Multipurpose Internet Mail Extensions (MIME), Part 2: Media Types
* [RFC 2047](https://tools.ietf.org/html/rfc2047): Multipurpose Internet Mail Extensions (MIME), Part 3: Message Header Extensions for Non-ASCII Text
* [RFC 2048](https://tools.ietf.org/html/rfc2048): Multipurpose Internet Mail Extensions (MIME), Part 4: Registration Procedures
* [RFC 2049](https://tools.ietf.org/html/rfc2049): Multipurpose Internet Mail Extensions (MIME), Part 5: Conformance Criteria and Examples
* [RFC 2231](https://tools.ietf.org/html/rfc2231): MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages, and Continuations
* [RFC 2183](https://tools.ietf.org/html/rfc2183): Communicating Presentation Information in Internet Message: The Content-Disposition Header Field
* [RFC 2557](https://tools.ietf.org/html/rfc2557): MIME Encapsulation of Aggregate Documents, such as HTML (MHTML)
* [RFC 1847](https://tools.ietf.org/html/rfc1847): Security Multiparts for MIME: Multipart/Signed and Multipart/Encrypted
* [RFC 3851](https://tools.ietf.org/html/rfc3851): S/MIME Version 3 Message Specification
* [RFC 3156](https://tools.ietf.org/html/rfc3156): MIME Security with OpenPGP
* [RFC 2298](https://tools.ietf.org/html/rfc2298): An Extensible Message Format for Message Disposition Notifications
* [RFC 2646](https://tools.ietf.org/html/rfc2646): The Text/Plain Format Parameter (not supported by KMime)

# Further Reading # {#section}

* [Wikipedia article on MIME](https://en.wikipedia.org/wiki/MIME)
* [The Absolute Minimum Every Software Developer Absolutely, Positively Must Know About Unicode and Character Sets (No Excuses!)](https://www.joelonsoftware.com/articles/Unicode.html)
* [A tutorial on character code issues](https://www.cs.tut.fi/~jkorpela/chars.html)
* [Online Base64 encoder and decoder](https://www.motobit.com/util/base64-decoder-encoder.asp)
* [Online quoted-printable encoder](https://www.motobit.com/util/quoted-printable-encoder.asp)
* [Onlinw quota reached](https://www.motobit.com/util/quoted-printable-decoder.asp)
* [Online charset converter](https://www.motobit.com/util/charset-codepage-conversion.asp)
* [Wikipedia article on public-key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography)
