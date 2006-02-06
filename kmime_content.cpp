/*
    kmime_content.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#include "kmime_content.h"
#include "kmime_parsers.h"

#include <kcharsets.h>
#include <kcodecs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qtextcodec.h>
#include <QTextStream>
#include <QByteArray>

using namespace KMime;

namespace KMime {

Content::Content()
 : f_orceDefaultCS( false )
{
  d_efaultCS = cachedCharset("ISO-8859-1");
}


Content::Content( const QByteArray &h, const QByteArray &b )
 : f_orceDefaultCS( false )
{
  d_efaultCS = cachedCharset("ISO-8859-1");
  h_ead = h;
  b_ody = b;
}


Content::~Content()
{
  qDeleteAll( c_ontents );
  c_ontents.clear();
  qDeleteAll( h_eaders );
  h_eaders.clear();
}


void Content::setContent( const QList<QByteArray> & l )
{
  //qDebug("Content::setContent( const QList<QByteArray> & l ) : start");
  h_ead.clear();
  b_ody.clear();

  //usage of textstreams is much faster than simply appending the strings
  QTextStream hts(&h_ead, QIODevice::WriteOnly),
              bts(&b_ody, QIODevice::WriteOnly);
  hts.setCodec( "ISO 8859-1" );
  bts.setCodec( "ISO 8859-1" );

  bool isHead=true;
  foreach ( QByteArray line, l ) {
    if ( isHead && line.isEmpty() ) {
      isHead=false;
      continue;
    }
    if(isHead)
      hts << line << "\n";
    else
      bts << line << "\n";
  }

  //terminate strings
  hts << '\0';
  bts << '\0';

  //qDebug("Content::setContent( const QList<QByteArray> & l ) : finished");
}


void Content::setContent( const QByteArray &s )
{
  int pos = s.indexOf( "\n\n", 0 );
  if(pos>-1) {
    h_ead=s.left(++pos);  //header *must* end with "\n" !!
    b_ody=s.mid(pos+1, s.length()-pos-1);
  }
  else
    h_ead=s;
}


//parse the message, split multiple parts
void Content::parse()
{
  //qDebug("void Content::parse() : start");
  qDeleteAll( h_eaders );
  h_eaders.clear();

  // check this part has already been partioned into subparts.
  // if this is the case, we will not try to reparse the body
  // of this part.
  if ( b_ody.size() == 0 && !c_ontents.isEmpty() ) {
    // reparse all sub parts
    foreach ( Content *c, c_ontents )
      c->parse();
    return;
  }

  qDeleteAll( c_ontents );
  c_ontents.clear();

  Headers::ContentType *ct=contentType();
  QByteArray tmp;
  Content *c;
  Headers::contentCategory cat;

  // just "text" as mimetype is suspicious, perhaps this article was
  // generated by broken software, better check for uuencoded binaries
  if (ct->mimeType()=="text")
    ct->setMimeType("invalid/invalid");

  if(ct->isText())
    return; //nothing to do

  if(ct->isMultipart()) {   //this is a multipart message
    tmp=ct->boundary(); //get boundary-parameter

    if(!tmp.isEmpty()) {
      Parser::MultiPart mpp(b_ody, tmp);
      if(mpp.parse()) { //at least one part found

        if(ct->isSubtype("alternative")) //examine category for the sub-parts
          cat=Headers::CCalternativePart;
        else
          cat=Headers::CCmixedPart;  //default to "mixed"

        QList<QByteArray> parts=mpp.parts();
        QList<QByteArray>::Iterator it;
        for(it=parts.begin(); it!=parts.end(); ++it) { //create a new Content for every part
          c=new Content();
          c->setContent(*it);
          c->parse();
          c->contentType()->setCategory(cat); //set category of the sub-part
          c_ontents.append( c );
          //qDebug("part:\n%s\n\n%s", c->h_ead.data(), c->b_ody.left(100).data());
        }

        //the whole content is now split into single parts, so it's safe delete the message-body
        b_ody.clear();
      }
      else { //sh*t, the parsing failed so we have to treat the message as "text/plain" instead
        ct->setMimeType("text/plain");
        ct->setCharset("US-ASCII");
      }
    }
  }
  else if (ct->mimeType()=="invalid/invalid") { //non-mime body => check for uuencoded content
    Parser::UUEncoded uup(b_ody, rawHeader("Subject"));

    if(uup.parse()) { // yep, it is uuencoded

      if(uup.isPartial()) {  // this seems to be only a part of the message so we treat it as "message/partial"
        ct->setMimeType("message/partial");
        //ct->setId(uniqueString()); not needed yet
        ct->setPartialParams(uup.partialCount(), uup.partialNumber());
        contentTransferEncoding()->setCte(Headers::CE7Bit);
      }
      else { //it's a complete message => treat as "multipart/mixed"
        //the whole content is now split into single parts, so it's safe to delete the message-body
        b_ody.clear();

        //binary parts
        for ( int i = 0; i < uup.binaryParts().count(); ++i ) {
          c=new Content();
          //generate content with mime-compliant headers
          tmp="Content-Type: ";
          tmp += uup.mimeTypes().at(i);
          tmp += "; name=\"";
          tmp += uup.filenames().at(i);
          tmp += "\"\nContent-Transfer-Encoding: x-uuencode\nContent-Disposition: attachment; filename=\"";
          tmp += uup.filenames().at(i);
          tmp += "\"\n\n";
          tmp += uup.binaryParts().at(i);
          c->setContent(tmp);
          addContent(c);
        }

        if ( !c_ontents.isEmpty() && c_ontents.first() ) { //readd the plain text before the uuencoded part
          c_ontents.first()->setContent("Content-Type: text/plain\nContent-Transfer-Encoding: 7Bit\n\n"+uup.textPart());
          c_ontents.first()->contentType()->setMimeType("text/plain");
        }
      }
    } else {
      Parser::YENCEncoded yenc(b_ody);

      if ( yenc.parse()) {
        /* If it is partial, just assume there is exactly one decoded part,
         * and make this that part */
        if (yenc.isPartial()) {
          ct->setMimeType("message/partial");
          //ct->setId(uniqueString()); not needed yet
          ct->setPartialParams(yenc.partialCount(), yenc.partialNumber());
          contentTransferEncoding()->setCte(Headers::CEbinary);
        }
        else { //it's a complete message => treat as "multipart/mixed"
          //the whole content is now split into single parts, so it's safe to delete the message-body
          b_ody.clear();

          //binary parts
          for (int i=0;i<yenc.binaryParts().count();i++) {
            c=new Content();
            //generate content with mime-compliant headers
            tmp="Content-Type: ";
            tmp += yenc.mimeTypes().at(i);
            tmp += "; name=\"";
            tmp += yenc.filenames().at(i);
            tmp += "\"\nContent-Transfer-Encoding: binary\nContent-Disposition: attachment; filename=\"";
            tmp += yenc.filenames().at(i);
            tmp += "\"\n\n";
            c->setContent(tmp);

            // the bodies of yenc message parts are binary data, not null-terminated strings:
            c->setBody(yenc.binaryParts()[i]);

            addContent(c);
          }

          if( !c_ontents.isEmpty() && c_ontents.first() ) { //readd the plain text before the uuencoded part
            c_ontents.first()->setContent("Content-Type: text/plain\nContent-Transfer-Encoding: 7Bit\n\n"+yenc.textPart());
            c_ontents.first()->contentType()->setMimeType("text/plain");
          }
        }
      }
      else { //no, this doesn't look like uuencoded stuff => we treat it as "text/plain"
        ct->setMimeType("text/plain");
      }
    }
  }

  //qDebug("void Content::parse() : finished");
}


void Content::assemble()
{
  QByteArray newHead="";

  //Content-Type
  newHead+=contentType()->as7BitString()+'\n';

  //Content-Transfer-Encoding
  newHead+=contentTransferEncoding()->as7BitString()+'\n';

  //Content-Description
  Headers::Base *h=contentDescription(false);
  if(h)
    newHead+=h->as7BitString()+"\n";

  //Content-Disposition
  h=contentDisposition(false);
  if(h)
    newHead+=h->as7BitString()+"\n";

  h_ead=newHead;
}


void Content::clear()
{
  qDeleteAll( h_eaders );
  h_eaders.clear();
  qDeleteAll( c_ontents );
  c_ontents.clear();
  h_ead.clear();
  b_ody.clear();
}


QByteArray Content::encodedContent(bool useCrLf)
{
  QByteArray e;

  // hack to convert articles with uuencoded or yencoded binaries into
  // proper mime-compliant articles
  if ( !c_ontents.isEmpty() ) {
    bool convertNonMimeBinaries=false;

    // reencode non-mime binaries...
    foreach ( Content *c, c_ontents ) {
      if ((c->contentTransferEncoding(true)->cte()==Headers::CEuuenc) ||
          (c->contentTransferEncoding(true)->cte()==Headers::CEbinary)) {
        convertNonMimeBinaries=true;
        c->b_ody = KCodecs::base64Encode(c->decodedContent(), true);
        c->b_ody.append("\n");
        c->contentTransferEncoding(true)->setCte(Headers::CEbase64);
        c->contentTransferEncoding(true)->setDecoded(false);
        c->removeHeader("Content-Description");
        c->assemble();
      }
    }

    // add proper mime headers...
    if (convertNonMimeBinaries) {
      int beg = 0, end = 0;
      beg = h_ead.indexOf( "MIME-Version: " );
      if ( beg >= 0 ) end = h_ead.indexOf( '\n', beg );
      if ( beg >= 0 && end > beg ) h_ead.remove( beg, end - beg );
      beg = h_ead.indexOf( "Content-Type: " );
      if ( beg >= 0 ) end = h_ead.indexOf( '\n', beg );
      if ( beg >= 0 && end > beg ) h_ead.remove( beg, end - beg );
      beg = h_ead.indexOf( "Content-Transfer-Encoding: " );
      if ( beg >= 0 ) end = h_ead.indexOf( '\n', beg );
      if ( beg >= 0 && end > beg ) h_ead.remove( beg, end - beg );

      h_ead+="MIME-Version: 1.0\n";
      h_ead+=contentType(true)->as7BitString()+"\n";
      h_ead+=contentTransferEncoding(true)->as7BitString()+"\n";
    }
  }

  //head
  e=h_ead;
  e+='\n';

  //body
  if(!b_ody.isEmpty()) { //this message contains only one part
    Headers::CTEncoding *enc=contentTransferEncoding();

    if(enc->needToEncode()) {
      if(enc->cte()==Headers::CEquPr) {
        e+=KCodecs::quotedPrintableEncode(b_ody, false);
      } else {
        e+=KCodecs::base64Encode(b_ody, true);
        e+="\n";
      }
    }
    else
      e+=b_ody;
  }
  else if( !c_ontents.isEmpty() ) { //this is a multipart message
    Headers::ContentType *ct=contentType();
    QByteArray boundary="--"+ct->boundary();

    //add all (encoded) contents separated by boundaries
    foreach ( Content *c, c_ontents ) {
      e+=boundary+"\n";
      e+=c->encodedContent(false);  // don't convert LFs here, we do that later!!!!!
    }
    //finally append the closing boundary
    e+=boundary+"--\n";
  };

  if(useCrLf)
    return LFtoCRLF(e);
  else
    return e;
}


QByteArray Content::decodedContent()
{
  QByteArray temp, ret;
  Headers::CTEncoding *ec=contentTransferEncoding();
  bool removeTrailingNewline=false;
  int size = b_ody.length();

  if (size==0)
    return ret;

  temp.resize(size);
  memcpy(temp.data(), b_ody.data(), size);

  if(ec->decoded()) {
    ret = temp;
    removeTrailingNewline=true;
  } else {
    switch(ec->cte()) {
      case Headers::CEbase64 :
        KCodecs::base64Decode(temp, ret);
      break;
      case Headers::CEquPr :
        ret = KCodecs::quotedPrintableDecode(b_ody);
        ret.resize(ret.size()-1);  // remove null-char
        removeTrailingNewline=true;
      break;
      case Headers::CEuuenc :
        KCodecs::uudecode(temp, ret);
      break;
      case Headers::CEbinary :
        ret = temp;
        removeTrailingNewline=false;
      default :
        ret = temp;
        removeTrailingNewline=true;
    }
  }

  if (removeTrailingNewline && (ret.size()>0) && (ret[ret.size()-1] == '\n'))
    ret.resize(ret.size()-1);

  return ret;
}


void Content::decodedText(QString &s, bool trimText,
			  bool removeTrailingNewlines)
{
  if(!decodeText()) //this is not a text content !!
    return;

  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  s=codec->toUnicode(b_ody.data(), b_ody.length());

  if (trimText && removeTrailingNewlines) {
    int i;
    for (i=s.length()-1; i>=0; i--)
      if (!s[i].isSpace())
        break;
    s.truncate(i+1);
  } else {
    if (s.right(1)=="\n")
      s.truncate(s.length()-1);    // remove trailing new-line
  }
}


void Content::decodedText(QStringList &l, bool trimText,
			  bool removeTrailingNewlines)
{
  if(!decodeText()) //this is not a text content !!
    return;

  QString unicode;
  bool ok=true;

  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  unicode=codec->toUnicode(b_ody.data(), b_ody.length());

  if (trimText && removeTrailingNewlines) {
    int i;
    for (i=unicode.length()-1; i>=0; i--)
      if (!unicode[i].isSpace())
        break;
    unicode.truncate(i+1);
  } else {
    if (unicode.right(1)=="\n")
      unicode.truncate(unicode.length()-1);    // remove trailing new-line
  }

  l = unicode.split( '\n' ); //split the string at linebreaks
}


void Content::fromUnicodeString(const QString &s)
{
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  if(!ok) { // no suitable codec found => try local settings and hope the best ;-)
    codec=KGlobal::locale()->codecForEncoding();
    QByteArray chset = KGlobal::locale()->encoding();
    contentType()->setCharset(chset);
  }

  b_ody=codec->fromUnicode(s);
  contentTransferEncoding()->setDecoded(true); //text is always decoded
}


Content* Content::textContent()
{
  Content *ret=0;

  //return the first content with mimetype=text/*
  if(contentType()->isText())
    ret=this;
  else
    foreach ( Content *c, c_ontents )
      if( (ret=c->textContent())!=0 )
        break;

  return ret;
}


void Content::attachments( Content::List &dst, bool incAlternatives )
{
  if ( c_ontents.isEmpty() )
    dst.append(this);
  else {
    foreach ( Content *c, c_ontents ) {
      if( !incAlternatives && c->contentType()->category()==Headers::CCalternativePart)
        continue;
      else
        c->attachments(dst, incAlternatives);
    }
  }

  if(type()!=ATmimeContent) { // this is the toplevel article
    Content *text=textContent();
    if(text)
      dst.removeAll( text );
  }
}


void Content::addContent(Content *c, bool prepend)
{
  if ( c_ontents.isEmpty() ) { // this message is not multipart yet

    // first we convert the body to a content
    Content *main=new Content();

    //the Mime-Headers are needed, so we move them to the new content
    for ( Headers::Base::List::iterator it = h_eaders.begin();
            it != h_eaders.end(); ) {
      if ( (*it)->isMimeHeader() ) {
        // append to new content
        main->h_eaders.append( *it );
        // and remove from this content
        h_eaders.erase( it );
      }
      else
        ++it;
    }

    //"main" is now part of a multipart/mixed message
    main->contentType()->setCategory(Headers::CCmixedPart);

    //the head of "main" is empty, so we assemble it
    main->assemble();

    //now we can copy the body and append the new content;
    main->b_ody = b_ody;
    c_ontents.append( main );
    b_ody.clear(); //not longer needed


    //finally we have to convert this article to "multipart/mixed"
    Headers::ContentType *ct=contentType();
    ct->setMimeType("multipart/mixed");
    ct->setBoundary(multiPartBoundary());
    ct->setCategory(Headers::CCcontainer);
    contentTransferEncoding()->clear();  // 7Bit, decoded

  }
  //here we actually add the content
  if(prepend)
    c_ontents.insert( 0, c );
  else
    c_ontents.append( c );
}


void Content::removeContent(Content *c, bool del)
{
  if( c_ontents.isEmpty() ) // what the ..
    return;

  c_ontents.removeAll( c );
  if(del)
    delete c;

  //only one content left => turn this message in a single-part
  if ( c_ontents.count() == 1 ) {
    Content *main = c_ontents.first();

    //first we have to move the mime-headers
    for ( Headers::Base::List::iterator it = main->h_eaders.begin();
          it != main->h_eaders.end(); ) {
      if ( (*it)->isMimeHeader() ) {
        kDebug(5003) << "Content::removeContent(Content *c, bool del) : mime-header moved: "
                      << (*it)->as7BitString() << endl;
        // first remove the old header
        removeHeader( (*it)->type() );
        // then append to new content
        h_eaders.append( *it );
        // and finally remove from this content
        main->h_eaders.erase( it );
      }
      else
        ++it;
    }

    //now we can copy the body
    b_ody = main->b_ody;

    //finally we can delete the content list
    qDeleteAll( c_ontents );
    c_ontents.clear();
  }
}


void Content::changeEncoding(Headers::contentEncoding e)
{
  Headers::CTEncoding *enc=contentTransferEncoding();
  if(enc->cte()==e) //nothing to do
    return;

  if(decodeText())
    enc->setCte(e); // text is not encoded until it's sent or saved so we just set the new encoding
  else { // this content contains non textual data, that has to be re-encoded

    if(e!=Headers::CEbase64) {
      //kWarning(5003) << "Content::changeEncoding() : non textual data and encoding != base64 - this should not happen\n => forcing base64" << endl;
      e=Headers::CEbase64;
    }

    if(enc->cte()!=e) { // ok, we reencode the content using base64
      b_ody = KCodecs::base64Encode(decodedContent(), true);
      b_ody.append("\n");
      enc->setCte(e); //set encoding
      enc->setDecoded(false);
    }
  }
}


void Content::toStream(QTextStream &ts, bool scrambleFromLines)
{
  QByteArray ret=encodedContent(false);

  if (scrambleFromLines)
    // FIXME Why are only From lines with a preceding empty line considered?
    //       And, of course, all lines starting with >*From have to be escaped
    //       because otherwise the transformation is not revertable.
    ret.replace( "\n\nFrom ", "\n\n>From ");

  ts << ret;
}


Headers::Generic*  Content::getNextHeader(QByteArray &head)
{
  int pos1=-1, pos2=0, len=head.length()-1;
  bool folded(false);
  Headers::Generic *header=0;

  pos1 = head.indexOf(": ");

  if (pos1>-1) {    //there is another header
    pos2=pos1+=2; //skip the name

    if (head[pos2]!='\n') {  // check if the header is not empty
      while(1) {
        pos2=head.indexOf('\n', pos2+1);
        if(pos2==-1 || pos2==len || ( head[pos2+1]!=' ' && head[pos2+1]!='\t') ) //break if we reach the end of the string, honor folded lines
          break;
        else
          folded = true;
      }
    }

    if(pos2<0) pos2=len+1; //take the rest of the string

    if (!folded)
      header = new Headers::Generic(head.left(pos1-2), this, head.mid(pos1, pos2-pos1));
    else {
      QByteArray hdrValue = head.mid( pos1, pos2 - pos1 );
      header = new Headers::Generic( head.left( pos1 - 2 ), this, unfoldHeader( hdrValue ) );
    }

    head.remove(0,pos2+1);
  }
  else {
    head = "";
  }

  return header;
}


Headers::Base* Content::getHeaderByType(const char *type)
{
  if(!type)
    return 0;

  //first we check if the requested header is already cached
  foreach ( Headers::Base *h, h_eaders )
    if ( h->is( type ) )
      return h; //found

  //now we look for it in the article head
  Headers::Base *h = 0;
  QByteArray raw=rawHeader(type);
  if(!raw.isEmpty()) { //ok, we found it
    //choose a suitable header class
    if(strcasecmp("Message-Id", type)==0)
      h=new Headers::MessageID(this, raw);
    else if(strcasecmp("Subject", type)==0)
      h=new Headers::Subject(this, raw);
    else if(strcasecmp("Date", type)==0)
      h=new Headers::Date(this, raw);
    else if(strcasecmp("From", type)==0)
      h=new Headers::From(this, raw);
    else if(strcasecmp("Organization", type)==0)
      h=new Headers::Organization(this, raw);
    else if(strcasecmp("Reply-To", type)==0)
      h=new Headers::ReplyTo(this, raw);
    else if(strcasecmp("Mail-Copies-To", type)==0)
      h=new Headers::MailCopiesTo(this, raw);
    else if(strcasecmp("To", type)==0)
      h=new Headers::To(this, raw);
    else if(strcasecmp("CC", type)==0)
      h=new Headers::CC(this, raw);
    else if(strcasecmp("BCC", type)==0)
      h=new Headers::BCC(this, raw);
    else if(strcasecmp("Newsgroups", type)==0)
      h=new Headers::Newsgroups(this, raw);
    else if(strcasecmp("Followup-To", type)==0)
      h=new Headers::FollowUpTo(this, raw);
    else if(strcasecmp("References", type)==0)
      h=new Headers::References(this, raw);
    else if(strcasecmp("Lines", type)==0)
      h=new Headers::Lines(this, raw);
    else if(strcasecmp("Content-Type", type)==0)
      h=new Headers::ContentType(this, raw);
    else if(strcasecmp("Content-Transfer-Encoding", type)==0)
      h=new Headers::CTEncoding(this, raw);
    else if(strcasecmp("Content-Disposition", type)==0)
      h=new Headers::CDisposition(this, raw);
    else if(strcasecmp("Content-Description", type)==0)
      h=new Headers::CDescription(this, raw);
    else
      h=new Headers::Generic(type, this, raw);

    h_eaders.append( h );  //add to cache
    return h;
  }
  else
    return 0; //header not found
}


void Content::setHeader(Headers::Base *h)
{
  if(!h) return;
  removeHeader(h->type());
  h_eaders.append( h );
}


bool Content::removeHeader(const char *type)
{
  for ( Headers::Base::List::iterator it = h_eaders.begin();
        it != h_eaders.end(); ++it )
    if ( (*it)->is(type) ) {
      delete (*it);
      h_eaders.erase( it );
      return true;
    }

  return false;
}


int Content::size()
{
  int ret=b_ody.length();

  if(contentTransferEncoding()->cte()==Headers::CEbase64)
    return (ret*3/4); //base64 => 6 bit per byte

  return ret;
}


int Content::storageSize()
{
  int s=h_ead.size();

  if ( c_ontents.isEmpty() )
    s+=b_ody.size();
  else {
    foreach ( Content *c, c_ontents )
      s+=c->storageSize();
  }

  return s;
}


int Content::lineCount()
{
  int ret=0;
  if(type()==ATmimeContent)
    ret+=h_ead.count('\n');
  ret+=b_ody.count('\n');

  foreach ( Content *c, c_ontents )
    ret+=c->lineCount();

  return ret;
}


QByteArray Content::rawHeader(const char *name)
{
  return KMime::extractHeader(h_ead, name);
}


bool Content::decodeText()
{
  Headers::CTEncoding *enc=contentTransferEncoding();

  if(!contentType()->isText())
    return false; //non textual data cannot be decoded here => use decodedContent() instead
  if(enc->decoded())
    return true; //nothing to do

  switch(enc->cte()) {
    case Headers::CEbase64 :
      b_ody=KCodecs::base64Decode(b_ody);
      b_ody.append("\n");
    break;
    case Headers::CEquPr :
      b_ody=KCodecs::quotedPrintableDecode(b_ody);
    break;
    case Headers::CEuuenc :
      b_ody=KCodecs::uudecode(b_ody);
      b_ody.append("\n");
    break;
    case Headers::CEbinary :
      // nothing to decode
      b_ody.append("\n");
    default :
    break;
  }

  enc->setDecoded(true);
  return true;
}


void Content::setDefaultCharset( const QByteArray &cs )
{
  d_efaultCS = KMime::cachedCharset(cs);

  foreach ( Content *c, c_ontents )
    c->setDefaultCharset(cs);

  // reparse the part and its sub-parts in order
  // to clear cached header values
  parse();
}


void Content::setForceDefaultCS(bool b)
{
  f_orceDefaultCS=b;

  foreach ( Content *c, c_ontents )
    c->setForceDefaultCS(b);

  // reparse the part and its sub-parts in order
  // to clear cached header values
  parse();
}


} // namespace KMime
