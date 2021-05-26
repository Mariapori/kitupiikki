/*
   Copyright (C) 2017 Arto Hyvättinen

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDesktopServices>
#include <QFile>
#include <QPainter>

#include <QPdfWriter>

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

#include <QDebug>
#include <QApplication>
#include <QMessageBox>

#include "tilinpaatostulostaja.h"
#include "db/kirjanpito.h"

#include "raportti/raportoija.h"



#include <cmath>

TilinpaatosTulostaja::TilinpaatosTulostaja(Tilikausi tilikausi, const QString& teksti, const QStringList &raportit, const QString& kieli, QObject *parent)
    : QObject(parent), tilikausi_(tilikausi), teksti_(teksti), raportit_(raportit), kieli_(kieli)
{

}

TilinpaatosTulostaja::~TilinpaatosTulostaja()
{

}

void TilinpaatosTulostaja::nayta()
{
    tallenna_ = false;
    tilaaRaportit();
}

void TilinpaatosTulostaja::tallenna()
{
    tallenna_ = true;
    tilaaRaportit();
}


void TilinpaatosTulostaja::tulosta(QPagedPaintDevice *writer) const
{
    writer->setPageSize( QPdfWriter::A4);

    writer->setPageMargins( QMarginsF(25,10,10,10), QPageLayout::Millimeter );
    QPainter painter( writer );

    tulostaKansilehti( &painter, tulkkaa("Tilinpäätös", kieli_) , tilikausi_, kieli_);
    int sivulla = 1;

    // Raportit
    for(auto& rk : kirjoittajat_) {
        if( rk.riveja()) {
            writer->newPage();
            sivulla += rk.tulosta(writer, &painter, false, sivulla);
        }
    }


    // Liitetiedot, allekirjoitukset yms
    painter.setFont( QFont("FreeSans",10));
    int rivinkorkeus = painter.fontMetrics().height();
    RaportinKirjoittaja kirjoittaja;
    kirjoittaja.asetaOtsikko(tulkkaa("Tilinpäätös", kieli_).toUpper() );
    kirjoittaja.asetaKausiteksti( tilikausi_.kausivaliTekstina());

    QTextDocument doc;
    // Sivutetaan niin, että ylätunniste mahtuu

    QSizeF sivunkoko( painter.viewport().width()  ,  painter.viewport().height() - rivinkorkeus * 4 );

    doc.documentLayout()->setPaintDevice( painter.device() );
    doc.setPageSize( sivunkoko );
    doc.setHtml( teksti_ );

    int pages = qRound(std::ceil( doc.size().height() / sivunkoko.height()  ));
    for( int i=0; i < pages; i++)
    {
        writer->newPage();
        painter.save();
        kirjoittaja.tulostaYlatunniste( &painter, sivulla);
        painter.drawLine(0,0,qRound(sivunkoko.width()),0);
        painter.translate(0, rivinkorkeus );

        painter.translate(0, 0 - i * sivunkoko.height() );

        doc.drawContents( &painter, QRectF(0, i * sivunkoko.height(),
                                           doc.textWidth(), sivunkoko.height() ) );
        painter.restore();
        sivulla++;
    }

    painter.end();

}

QString TilinpaatosTulostaja::otsikko() const
{
    return QString("%1 %2").arg(tulkkaa("Tilinpäätös", kieli_)).arg(tilikausi_.kausivaliTekstina());
}

void TilinpaatosTulostaja::tilaaRaportit()
{
    tilattuja_ = raportit_.count();
    // Tilaa halutut raportit, jotta ne saadaan käyttöön
    for(QString raportti : raportit_)
        if(!tilaaRaportti(raportti))
            return;
}


void TilinpaatosTulostaja::tulostaKansilehti(QPainter *painter, const QString otsikko, Tilikausi kausi, const QString& kieli)
{
    tulostaKansilehti(painter, otsikko, kausi.kausivaliTekstina(), kieli);
}

void TilinpaatosTulostaja::tulostaKansilehti(QPainter *painter, const QString &otsikko, const QString &alaotsikko, const QString &kieli)
{
    painter->save();
    painter->setFont(QFont("FreeSans",24,QFont::Bold));

    int sivunleveys = painter->window().width();
    int rivinkorkeus = painter->fontMetrics().height();
    int sivunkorkeus = painter->window().height();

    if( !kp()->logo().isNull()  )
    {
        double skaala = (1.00 *  kp()->logo().width() ) / kp()->logo().height();
        double leveys = rivinkorkeus * 4 * skaala;
        double korkeus = rivinkorkeus * 4;

        if( leveys > sivunleveys * 10 / 11)
        {
            leveys = sivunleveys * 10 / 11;
            korkeus = leveys / skaala;
        }

        painter->drawImage( QRectF((sivunleveys - leveys) / 2, sivunkorkeus / 3 - rivinkorkeus * 4, leveys , korkeus),
                              kp()->logo() );
    }
    painter->drawText( QRectF(0, sivunkorkeus/3, sivunleveys, rivinkorkeus * 2), Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignHCenter, kp()->asetukset()->asetus(AsetusModel::OrganisaatioNimi));

    painter->setFont(QFont("FreeSans",24));
    painter->drawText( QRectF(0, sivunkorkeus/3 + rivinkorkeus * 4, sivunleveys, rivinkorkeus ), Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignHCenter, otsikko );
    painter->drawText( QRectF(0, sivunkorkeus/3 + rivinkorkeus * 5, sivunleveys, rivinkorkeus*4 ), Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignHCenter , alaotsikko);

    painter->setFont(QFont("FreeSans",12));
    QString omaOsoite = kp()->asetukset()->asetus(AsetusModel::Katuosoite) + "\n" +
            kp()->asetukset()->asetus(AsetusModel::Postinumero) + " " + kp()->asetukset()->asetus(AsetusModel::Kaupunki);
    painter->drawText( QRectF(0,sivunkorkeus / 8 * 7, sivunleveys / 3, sivunkorkeus / 8), Qt::TextWordWrap, omaOsoite);
    if( !kp()->asetukset()->asetus(AsetusModel::Ytunnus).isEmpty())
        painter->drawText( QRectF( sivunleveys/3*2, sivunkorkeus / 8 * 7, sivunleveys / 3, rivinkorkeus), Qt::AlignLeft, QString("%1: %2").arg(tulkkaa("Y-tunnus", kieli)).arg(kp()->asetukset()->asetus(AsetusModel::Ytunnus)));
    if( !kp()->asetukset()->asetus(AsetusModel::Kotipaikka).isEmpty())
        painter->drawText( QRectF( sivunleveys/3*2, sivunkorkeus / 8 * 7 + painter->fontMetrics().height(), sivunleveys / 3, rivinkorkeus), Qt::AlignLeft, QString("%1: %2").arg(tulkkaa("Kotipaikka", kieli)).arg(kp()->asetukset()->asetus(AsetusModel::Kotipaikka)));


    painter->restore();
}

bool TilinpaatosTulostaja::tilaaRaportti(const QString &raporttistr)
{
    QRegularExpression raporttiRe("@(?<raportti>.+?)(:(?<optiot>\\w*))?[!](?<otsikko>.+)@");
    QRegularExpressionMatch mats = raporttiRe.match(raporttistr);
    QString raporttitunnus = mats.captured("raportti");
    QString optiot = mats.captured("optiot");
    QString otsikko = mats.captured("otsikko");

    if( !kp()->asetukset()->onko(raporttitunnus)) {
        QMessageBox::critical(nullptr, tr("Virheellinen tilinpäätöskaava"),
                              tr("Tilinpäätöskaavan asetuksiin sisältyy raportti %1 jota ei ole olemassa.\n"
                                 "Tilinpäätöksen kaavaa on korjattava jotta tilinpäätöksen voi tulostaa.").arg(raporttitunnus));
        return false;
    }

    Raportoija::RaportinTyyppi tyyppi = Raportoija::VIRHEELLINEN;

    if( optiot.contains("K"))
        tyyppi = Raportoija::KOHDENNUSLASKELMA;
    else if(optiot.contains("P"))
        tyyppi = Raportoija::PROJEKTILASKELMA;
    bool erittelyt = optiot.contains("E");

    int indeksi = kirjoittajat_.size();
    kirjoittajat_.append(RaportinKirjoittaja());

    Raportoija* raportoija = new Raportoija(raporttitunnus, kieli_ , this, tyyppi);
    Tilikausi edellinen = kp()->tilikausiPaivalle( tilikausi_.alkaa().addDays(-1) );

    if( raportoija->onkoTaseraportti()) {
        raportoija->lisaaTasepaiva( tilikausi_.paattyy() );
        if( edellinen.alkaa().isValid())
            raportoija->lisaaTasepaiva( edellinen.paattyy());
    } else if( optiot.contains("B")) {
        raportoija->lisaaKausi( tilikausi_.alkaa(), tilikausi_.paattyy(),  Raportoija::BUDJETTI );
        raportoija->lisaaKausi( tilikausi_.alkaa(), tilikausi_.paattyy(),  Raportoija::TOTEUTUNUT );
        raportoija->lisaaKausi( tilikausi_.alkaa(), tilikausi_.paattyy(),  Raportoija::BUDJETTIERO );
        raportoija->lisaaKausi( tilikausi_.alkaa(), tilikausi_.paattyy(),  Raportoija::TOTEUMAPROSENTTI );
    } else {
        raportoija->lisaaKausi(tilikausi_.alkaa(), tilikausi_.paattyy());
        if( edellinen.alkaa().isValid())
            raportoija->lisaaKausi(edellinen.alkaa(), edellinen.paattyy());
    }

    connect( raportoija, &Raportoija::valmis, this, [this,indeksi,otsikko] (RaportinKirjoittaja rk) { this->raporttiSaapuu(indeksi, rk, otsikko); } );
    connect( raportoija, &Raportoija::tyhjaraportti, this, [this,indeksi,otsikko] { this->raporttiSaapuu(indeksi, RaportinKirjoittaja(), otsikko); } );
    raportoija->kirjoita(erittelyt,-1);
    return true;
}


void TilinpaatosTulostaja::raporttiSaapuu(int raportti, RaportinKirjoittaja rk, const QString& otsikko)
{
    rk.asetaOtsikko(otsikko);
    kirjoittajat_[raportti] = rk;
    tilattuja_--;
    if( !tilattuja_) {
        if( tallenna_) {
            QMap<QString,QString> meta;
            meta.insert("Filename",QString("%1 %2.pdf").arg(tulkkaa("Tilinpäätös")).arg(tilikausi_.pitkakausitunnus()));
            meta.insert("Content-type","application/pdf");

            KpKysely* kysely = kpk(QString("/liitteet/0/TP_%1").arg(tilikausi_.paattyy().toString(Qt::ISODate)), KpKysely::PUT);
            connect( kysely, &KpKysely::vastaus, this, &TilinpaatosTulostaja::tallennettu);
            kysely->lahetaTiedosto(pdf(), meta);

        } else
            esikatsele();
    }

}


