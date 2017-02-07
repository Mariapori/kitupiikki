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

#include <QDir>

#include <QPixmap>
#include <QIcon>

#include <QFile>
#include <QTextStream>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMapIterator>

#include <QApplication>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <QProgressDialog>

#include "uusikirjanpito.h"

#include "introsivu.h"
#include "nimisivu.h"
#include "tilikarttasivu.h"
#include "tilikausisivu.h"
#include "sijaintisivu.h"
#include "loppusivu.h"

#include "db/asetusmodel.h"
#include "db/tilimodel.h"
#include "db/tili.h"
#include "db/tilikausimodel.h"

#include <QDebug>

UusiKirjanpito::UusiKirjanpito() :
    QWizard()
{
    setWindowIcon(QIcon(":/pic/Possu64.png"));
    setWindowTitle("Uuden kirjanpidon luominen");
    setPixmap( WatermarkPixmap, QPixmap(":/pic/Possu64.png") );
    addPage( new IntroSivu());
    addPage( new NimiSivu());
    addPage( new TilikarttaSivu);
    addPage( new TilikausiSivu );
    addPage( new SijaintiSivu );
    addPage( new LoppuSivu );
}


QString UusiKirjanpito::aloitaUusiKirjanpito()
{
    UusiKirjanpito velho;

    velho.exec();

    if( velho.alustaKirjanpito())
        // Palautetaan uuden kirjanpidon hakemistopolku
        return velho.field("sijainti").toString() + "/" + velho.field("hakemisto").toString();
    else
        // Epäonnistui, tyhjä merkkijono
        return QString();
}

QMap<QString, QStringList> UusiKirjanpito::lueKtkTiedosto(const QString &polku)
{
    // ktk-tiedosto koostuu osista, jotka merkitään [otsikko] ja
    // niiden väleissä olevista tiedoista. Rivi voidaan
    // kommentoida #-merkillä


    QMap<QString, QStringList> tiedot;

    QFile tiedosto(polku);
    if( tiedosto.open(QIODevice::ReadOnly))
    {
        QTextStream in(&tiedosto);
        in.setCodec("utf8");
        QString nykyavain;
        QStringList nykytieto;

        while( !in.atEnd())
        {
            QString rivi = in.readLine();
            if( rivi.startsWith('[') && rivi.endsWith(']'))
            {
                // Tallennetaan nykyinen
                if( !nykyavain.isEmpty())
                    tiedot[nykyavain] = nykytieto;
                // Aloitetaan uusi
                nykyavain = rivi.mid(1, rivi.length() - 2);
                nykytieto.clear();
            }
            else if( !rivi.startsWith('#') && !nykyavain.isEmpty())
                nykytieto.append(rivi);
        }
        // Tiedoston lopussa päätetään viimeinen tieto
        if( !nykyavain.isEmpty())
            tiedot[nykyavain] = nykytieto;
    }

    return tiedot;
}

bool UusiKirjanpito::alustaKirjanpito()
{
    // Ladataan karttatiedosto
    QMap<QString,QStringList> kartta = lueKtkTiedosto( field("tilikartta").toString() );

    // Näyttää QProgressDialogin jotta käyttäjä ei hermostu
    // Koska hitain osuus on tilien kirjoittaminen, päivätetään dialogia aina tilejä kirjoitettaessa
    // ja sitä varten lasketaan "prosessilukuun" tilien ja otsikkojen lukumäärä.

    int prosessiluku = 9 + kartta.value("otsikot").count() + kartta.value("tilit").count();
    QProgressDialog progDlg(tr("Luodaan uutta kirjanpitoa"), QString(), 1, prosessiluku);
    progDlg.setMinimumDuration(0);
    progDlg.setValue(1);
    progDlg.setWindowModality(Qt::WindowModal);
    qApp->processEvents();


    QString hakemistopolku = field("sijainti").toString() + "/" + field("hakemisto").toString();
    QDir hakemisto;

    // Luodaan hakemisto
    if( !hakemisto.mkdir(hakemistopolku) || !hakemisto.cd(hakemistopolku))
        return false;

    // Luodaan alihakemistot
    hakemisto.mkdir("tositteet");
    hakemisto.mkdir("arkisto");

    progDlg.setValue( progDlg.value() + 1 );

    // Luodaan tietokanta
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","luonti");
    db.setDatabaseName(hakemistopolku + "/kitupiikki.sqlite");
    if( !db.open())
    {
        return false;
    }
    QSqlQuery query(db);

    progDlg.setValue( progDlg.value() + 1 );

    // Luodaan tietokanta
    // Tietokannan luontikäskyt ovat resurssitiedostossa luo.sql
    QFile sqltiedosto(":/sql/luo.sql");
    sqltiedosto.open(QIODevice::ReadOnly);
    QTextStream in(&sqltiedosto);
    QString sqluonti = in.readAll();
    sqluonti.replace("\n","");
    QStringList sqlista = sqluonti.split(";");

    foreach (QString kysely,sqlista)
    {
        query.exec(kysely);
        qApp->processEvents();
    }

    progDlg.setValue( progDlg.value() + 1 );


    AsetusModel asetukset(db);

    // Kirjataan tietokannan perustietoja

    asetukset.aseta("nimi", field("nimi").toString());
    asetukset.aseta("ytunnus", field("ytunnus").toString());
    asetukset.aseta("harjoitus", field("harjoitus").toBool());

    asetukset.aseta("luotu", QDate::currentDate());
    asetukset.aseta("versio", qApp->applicationVersion());

    progDlg.setValue( progDlg.value() + 1 );

    // Siirretään asetustauluun tilikartan tiedot
    // jotka alkavat *-merkillä
    QMapIterator<QString,QStringList> i(kartta);
    while( i.hasNext())
    {
        i.next();
        if( i.key().startsWith("*"))
        {
            asetukset.aseta( i.key(), i.value());
        }
    }

    progDlg.setValue( progDlg.value() + 1 );

    // Tilien ja otsikkojen kirjoittaminen
    TiliModel tilit(db);

    QStringList tililista = kartta.value("tilit");
    foreach ( QString tili, tililista)
    {
        // Tilitietueet ovat muotoa tyyppi;numero;nimi
        QStringList splitti = tili.split(";");
        if( splitti.count() > 2)            
        {
            // Kolmannessa mahdollisessa sarakkeessa otsikkotaso, 0 jos kirjaustili
            int otsikkotaso = 0;
            if( splitti.count() > 3)
                otsikkotaso = splitti[3].toInt();

            Tili tili;
            tili.asetaTyyppi( splitti[0] );
            tili.asetaNumero( splitti[1].toInt() );
            tili.asetaNimi( splitti[2]);
            tili.asetaOtsikkotaso( otsikkotaso );
            tilit.lisaaTili(tili);
        }
        progDlg.setValue( progDlg.value() + 1 );
    }

    qDebug() << tilit.rowCount( QModelIndex() ) << " tiliä talletettu ";

    tilit.tallenna();



    // Tilikausien kirjoittaminen
    // Nykyinen tilikausi

    TilikausiModel tilikaudet(db);
    tilikaudet.lisaaTilikausi( Tilikausi( field("alkaa").toDate(), field("paattyy").toDate() ));

    if( field("onekakausi").toBool())
    {
        // Ensimmäinen tilikausi, tilinavausta ei tarvita
        asetukset.aseta("tilinavaus",0);
    }
    else
    {
        // Edellinen tilikausi.
        tilikaudet.lisaaTilikausi( Tilikausi(field("edalkoi").toDate(), field("edpaattyi").toDate()  ) );

        asetukset.aseta("tilinavaus", 2);
        asetukset.aseta("tilinavauspvm", field("edpaattyi").toDate());
        asetukset.aseta("tilitpaatetty", field("edpaattyi").toDate());
    }
    tilikaudet.tallenna();

    progDlg.setValue( progDlg.value() + 1 );

    // Kirjoitetaan nollatosite tilien avaamiseen
    if( !field("onekakausi").toBool())
    {
        query.prepare("INSERT INTO TOSITE(id,pvm,otsikko,laji) "
                      "VALUES (0,?,\"Tilinavaus\",0)");
        query.addBindValue( field("edpaattyi").toDate());
        query.exec();
    }
    // Prosessi valmis
    progDlg.setValue( prosessiluku );

    db.close();

    return true;

}
