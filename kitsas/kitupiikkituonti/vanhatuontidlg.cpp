/*
   Copyright (C) 2019 Arto Hyvättinen

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
#include "vanhatuontidlg.h"
#include "ui_vanhatuontidlg.h"
#include <QSettings>
#include <QFile>
#include <QListWidgetItem>

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QJsonDocument>
#include <QDate>

#include "uusikirjanpito/uusivelho.h"
#include "kieli/monikielinen.h"
#include "db/kirjanpito.h"
#include "sqlite/sqlitemodel.h"
#include "db/tositetyyppimodel.h"

#include "model/tosite.h"
#include "model/tositeviennit.h"
#include "model/tositevienti.h"
#include "model/tositerivit.h"
#include "model/lasku.h"
#include "rekisteri/asiakastoimittajadlg.h"

#include <iostream>

VanhatuontiDlg::VanhatuontiDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VanhatuontiDlg)
{
    ui->setupUi(this);
    ui->tuoNappi->setVisible(false);
    ui->jatkaNappi->setVisible(false);
    alustaTuonti();

    connect( ui->valitseTiedosto, &QPushButton::clicked, this, &VanhatuontiDlg::tuoTiedostosta);
    connect( ui->lista, &QListWidget::itemClicked, [this] (QListWidgetItem *item) { this->avaaTietokanta( item->data(Qt::UserRole).toString()) ;});
    connect( ui->jatkaNappi, &QPushButton::clicked, this, &VanhatuontiDlg::alustaSijainti);
    connect( ui->hakemistoNappi, &QPushButton::clicked, this, &VanhatuontiDlg::valitseHakemisto);
    connect( ui->tuoNappi, &QPushButton::clicked, this, &VanhatuontiDlg::tuo);
    connect( ui->ohjeNappi, &QPushButton::clicked, [] { kp()->ohje("aloitus/kitupiikki"); });
}

VanhatuontiDlg::~VanhatuontiDlg()
{
    if( kpdb_.isOpen())
        kpdb_.close();

    delete ui;
}

void VanhatuontiDlg::alustaTuonti()
{
    QSettings settings("Kitupiikki Kirjanpito", "Kitupiikki");
    QVariantMap kirjanpidot = settings.value("Tietokannat").toMap();

    // Poistetaan ne, joita ei löydy
    for(QString polku : kirjanpidot.keys()) {
        if( QFile::exists(polku))
            kirjanpidot.remove("polku");
    }

    QMapIterator<QString, QVariant> iter(kirjanpidot);
    while( iter.hasNext()) {
        iter.next();
        QString polku = iter.key();
        QVariantList lista = iter.value().toList();
        QString nimi = lista.value(0).toString();
        QByteArray logo = lista.value(1).toByteArray();

        QListWidgetItem *item = new QListWidgetItem(nimi, ui->lista);
        item->setData(Qt::UserRole, polku);
        QPixmap kuva;
        kuva.loadFromData(logo, "PNG");
        item->setIcon(QIcon(kuva));
    }
}

void VanhatuontiDlg::tuoTiedostosta()
{
    QString tiedosto = QFileDialog::getOpenFileName(this, tr("Valitse tuotava kirjanpitotiedosto"),
                                                    QDir::homePath(),"Kirjanpito (*.kitupiikki kitupiikki.sqlite)");
    if( !tiedosto.isEmpty())
        avaaTietokanta(tiedosto);
}

void VanhatuontiDlg::haeTilikartta(const QString &polku)
{
    kitsasAsetukset_ = UusiVelho::asetukset(polku);

    // Tilit oma json-tiedosto
    {
        QFile tilit(polku + "/tilit.json");
        if( tilit.open(QIODevice::ReadOnly) ) {
            QJsonDocument doc = QJsonDocument::fromJson(tilit.readAll());
            QVariant variant = doc.toVariant();
            kitsasTilit_ = variant.toList();
        }
    }

    // Tilien muuntotaulukon lukeminen
    {
        QFile muunto(polku + "/muunto.txt");
        if( muunto.open(QIODevice::ReadOnly)) {
            QString txt = QString::fromUtf8(muunto.readAll());
            for(QString rivi : txt.split('\n')) {
                QStringList osat = rivi.split(":");
                if( osat.value(1).toInt())
                    tilinMuunto_.insert( osat.value(0).toInt(), osat.value(1).toInt() );
            }
        }
    }

    // TODO: Tilien muuntotaulukko (tarvitaan, koska yritystilikartassa tilinumerointia on muutettu)
}

void VanhatuontiDlg::avaaTietokanta(const QString &tiedostonnimi)
{
    ui->pino->setCurrentIndex(VIRHE);
    qApp->processEvents();

    if( QSqlDatabase::connectionNames().contains("Tuonti"))
        kpdb_ = QSqlDatabase::database("Tuonti", false);
    else
        kpdb_ = QSqlDatabase::addDatabase("QSQLITE","Tuonti");

    kpdb_.setDatabaseName(tiedostonnimi);
    if( !kpdb_.open() ) {
        ui->virheLabel->setText(tr("Valitsemaasi tiedostoa %1 ei voi avata, tai se ei "
                                   "ole SQLITE-tietokanta.\n\n"
                                   "%2")
                                .arg(tiedostonnimi)
                                .arg(kpdb_.lastError().text()));
        return;
    }

    // Ladataan asetukset
    QSqlQuery query(kpdb_);
    query.exec("SELECT avain,arvo, muokattu FROM Asetus");
    if( query.lastError().isValid()) {
        if( query.lastError().text().contains("locked")) {
            ui->virheLabel->setText(tr("Valitsemaasi tietokantaa ei voi avata, koska "
                                    "se on toisen ohjelman käytössä."));
        } else {
            ui->virheLabel->setText(tr("Tiedoston %1 avaamisessa tapahtui virhe tai valitsemasi "
                                       "tiedosto ei ole Kitupiikin tietokanta \n\n"
                                       "%2")
                                    .arg(tiedostonnimi)
                                    .arg(query.lastError().text()));
        }
        return;
    }
    while( query.next()) {
        kitupiikkiAsetukset_.insert(query.value(0).toString(), query.value(1).toString());
        if( !query.value(2).isNull())
            muokatutAsetukset_.append(query.value(0).toString());
    }

    if( !kitupiikkiAsetukset_.contains("KpVersio"))
        ui->virheLabel->setText(tr("Valitsemasi tiedosto ei ole Kitupiikin kirjanpito."));
    else if( kitupiikkiAsetukset_.value("KpVersio").toInt() >= 20)
        ui->virheLabel->setText(tr("Valitsemasi tiedosto on jo Kitsaan kirjanpito"));
    else if( kitupiikkiAsetukset_.value("KpVersio").toInt() < 10)
        ui->virheLabel->setText(tr("Valitsemaasi tiedosto on tallennettu vanhalla Kitupiikin versiolla.\n\n"
                                   "Tiedosto on ensin avattava uudemmalla Kitupiikin versiolla, jotta se "
                                   "päivittyy uudempaan tiedostomuotoon"));
    else if( kitupiikkiAsetukset_.value("VakioTilikartta") != "tilitin.kpk"
             && kitupiikkiAsetukset_.value("VakioTilikartta") != "yhdistys-1.kpk"
             && kitupiikkiAsetukset_.value("VakioTilikartta") != "asoy.kpk")
        ui->virheLabel->setText(tr("Tuontitoiminto ei tue kirjanpitosi tilikarttaa tyyppiä %1 (%2).\n\n"
                                   "Tuontitoiminto tukee ainoastaa uusimpia elinkeinotoiminnan, "
                                   "yhdistyksen ja asunto-osakeyhtiön tilikarttoja.\n\n"
                                   "Kirjanpidot, jotka on aloitettu Kitupiikin vanhimmilla versioilla, "
                                   "voi siirtää Kitsaaseen ainoastaan luomalla uuden kirjanpidon ja "
                                   "tekemällä siihen tilinavauksen vanhan kirjanpidon pohjalta.")
                                .arg(kitupiikkiAsetukset_.value("TilikarttaNimi"))
                                .arg(kitupiikkiAsetukset_.value("VakioTilikartta")));
    else
        alustaValinnat();
}

void VanhatuontiDlg::alustaValinnat()
{
    ui->pino->setCurrentIndex(VALINNAT);
    QString kitupiikkiTilikartta = kitupiikkiAsetukset_.value("VakioTilikartta");

    if( kitupiikkiTilikartta == "yhdistys-1.kpk")
        haeTilikartta(":/tilikartat/yhdistys");
    else if( kitupiikkiTilikartta == "tilitin.kpk")
        haeTilikartta(":/tilikartat/yritys");
    else if( kitupiikkiTilikartta == "asoy.kpk")
        haeTilikartta(":/tilikartat/asoy");

    ui->nimiEdit->setText( kitupiikkiAsetukset_.value("Nimi"));

    // Laajuuksien hakeminen
    QVariantMap laajuusMap = kitsasAsetukset_.value("laajuudet").toMap();
    QMapIterator<QString,QVariant> laajuusIter(laajuusMap);
    while( laajuusIter.hasNext()) {
        laajuusIter.next();
        Monikielinen kk( laajuusIter.value() );
        QListWidgetItem *item = new QListWidgetItem(kk.teksti(), ui->laajuusLista);
        item->setData(Qt::UserRole, laajuusIter.key());
    }


    if( kitupiikkiTilikartta == "yhdistys-1.kpk") {
        ui->tilikarttaLabel->setText(tr("Yhdistys"));
        int laajuus = kitupiikkiAsetukset_.value("Muoto").left(1).toInt();
        if( laajuus > 3)
            laajuus++;  // Väliin tullut yksi laajuus
        kitsasAsetukset_.insert("laajuus", laajuus);
        kitsasAsetukset_.insert("muoto","ry");
    } else if( kitupiikkiTilikartta == "tilitin.kpk") {
        // Tieto yrityksen muodosta siirtyy muodoksi
        QString vanhamuoto = kitupiikkiAsetukset_.value("Muoto");
        QString uusimuoto = "oy";

        if( vanhamuoto == "Avoin yhtiö")
            uusimuoto = "ay";
        else if( vanhamuoto == "Elinkeinonharjoittaja")
            uusimuoto = "tmi";
        else if( vanhamuoto == "Kommandiittiyhtiö")
            uusimuoto = "ky";
        else if( vanhamuoto == "Osuuskunta")
            uusimuoto = "osk";
        // Ei enää eroa julkisen ja muun oy:n välillä
        else if( vanhamuoto == "Julkinen osakeyhtiö")
            vanhamuoto = "Osakeyhtiö";
        ui->tilikarttaLabel->setText(tr("Yritys (%1)").arg(vanhamuoto));
        kitsasAsetukset_.insert("muoto", uusimuoto);
    } else if( kitupiikkiTilikartta == "asoy.kpk") {
        ui->tilikarttaLabel->setText(tr("Asunto-osakeyhtiö"));
        kitsasAsetukset_.insert("muoto", "asoy");
        kitsasAsetukset_.insert("laajuus",2);
    }

    ui->laajuusLista->setCurrentRow( kitsasAsetukset_.value("laajuus").toInt() - 1);

    bool muokattuja = !muokatutAsetukset_.filter("Raportti/").isEmpty() ||
                      muokatutAsetukset_.contains("TilinpaatosPohja");

    bool maksamattomiaMaksuperusteisia = false;
    QSqlQuery query(kpdb_);
    query.exec("SELECT COUNT(id) FROM Vienti WHERE pvm IS NULL");
    if( query.next())
        maksamattomiaMaksuperusteisia = query.value(0).toInt();

    ui->muovaKuva->setVisible(muokattuja || maksamattomiaMaksuperusteisia);
    ui->muvaLabel->setVisible(muokattuja);
    ui->maksuperusteVaroitus->setVisible( maksamattomiaMaksuperusteisia );
    ui->jatkaNappi->setVisible(true);
}

void VanhatuontiDlg::alustaSijainti()
{
    ui->pino->setCurrentIndex(SIJAINTI);

    ui->jatkaNappi->setVisible(false);
    ui->tuoNappi->setVisible(true);
    QString vanhasijainti = kpdb_.databaseName();

    QFileInfo info(vanhasijainti);
    QString uusinimi = info.fileName();
    uusinimi.remove(".kitupiikki");

    ui->tiedostonNimi->setValidator(new QRegularExpressionValidator(QRegularExpression("([A-Za-z0-9]|-){1,64}"), this));

    ui->tiedostonNimi->setText( uusinimi );
    ui->tiedostonHakemisto->setText( info.dir().absolutePath() );
}

void VanhatuontiDlg::valitseHakemisto()
{
    QString hakemisto = QFileDialog::getExistingDirectory(nullptr, tr("Valitse tallennushakemisto"),
                                                          ui->tiedostonHakemisto->text());
    if( !hakemisto.isEmpty())
        ui->tiedostonHakemisto->setText( hakemisto );
}

void VanhatuontiDlg::tuo()
{
    QDir hakemisto(ui->tiedostonHakemisto->text());
    QString polku = hakemisto.absoluteFilePath(ui->tiedostonNimi->text() + ".kitsas");
    if( ui->tiedostonNimi->text().isEmpty() || polku.isEmpty()) {
        QMessageBox::critical(this, tr("Kirjanpidon tuonti"), tr("Tiedoston nimi ei kelpaa"));
        return;
    } else if( QFile::exists(polku)) {
        QMessageBox::critical(this, tr("Kirjanpidon tuonti"), tr("Tiedosto %1 on jo olemassa.").arg(polku));
        return;
    }

    ui->tuoNappi->setEnabled(false);
    ui->peruNappi->setEnabled(false);
    ui->pino->setCurrentIndex(ODOTA);
    qApp->processEvents();

    // Määritellään etenemismittaria
    int laskenta = 100;
    QSqlQuery sql( kpdb_);
    sql.exec("SELECT COUNT(id) FROM Tosite");
    if( sql.next())
        laskenta += sql.value(0).toInt();
    sql.exec("SELECT COUNT(id) FROM Liite");
    if( sql.next())
        laskenta += sql.value(0).toInt();
    ui->progressBar->setRange(0, laskenta);

    qlonglong vanhasumma = 0l;
    sql.exec("SELECT SUM(debetsnt) FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id WHERE tosite.pvm NOT NULL");
    if( sql.next())
        vanhasumma = sql.value(0).toLongLong();

    // Tietokannan luoneen version tieto
    ui->progressBar->setValue(2);
    kitsasAsetukset_.insert("KpVersio", SQLiteModel::TIETOKANTAVERSIO);
    kitsasAsetukset_.insert("LuotuVersiolla", qApp->applicationVersion());
    kitsasAsetukset_.insert("Nimi", ui->nimiEdit->text());  // Jotta tulee viimeisten luetteloon ;)

    lisaaIbanit();

    QVariantMap initMap;
    initMap.insert("asetukset", kitsasAsetukset_);
    initMap.insert("tilit", kitsasTilit_);
    QVariantMap map;
    map.insert("name", ui->nimiEdit->text());
    map.insert("init", initMap);

    if( !kp()->sqlite()->uusiKirjanpito(polku, map)) {
        ui->pino->setCurrentIndex(VIRHE);
        ui->virheLabel->setText(tr("Uuden tietokannan luominen epäonnistui."));
        return;
    }
    ui->progressBar->setValue(10);

    // Muuten avataan tietokanta
    if( !kp()->sqlite()->avaaTiedosto(polku)) {
        ui->pino->setCurrentIndex(VIRHE);
        ui->virheLabel->setText(tr("Uuden tietokannan luominen epäonnistui."));
        return;
    }
    qApp->processEvents();

    ui->progressBar->setValue(20);
    siirraAsetukset();
    ui->progressBar->setValue(30);
    siirraTilikaudet();
    ui->progressBar->setValue(40);
    taydennaTilit();
    ui->progressBar->setValue(50);
    siirraKohdennukset();
    ui->progressBar->setValue(60);
    siirraTuotteet();
    ui->progressBar->setValue(70);
    if( ui->asiakasCheck->isChecked())
        siirraAsiakkaat();
    ui->progressBar->setValue(80);

    // Haetaan muokatut tilit yms. jotta voidaan käsitellä tilien tuontia
    kp()->sqlite()->alusta();
    ui->progressBar->setValue(90);


    siirraTositteet();
    siirraLogo();

    // Tietokanta pitää avata vielä uudelleen, jotta päivittää tiedot
    // tietokannasta
    kp()->sqlite()->sulje();
    kp()->sqlite()->avaaTiedosto(polku);

    QSqlQuery varmistussql( kp()->sqlite()->tietokanta() );
    varmistussql.exec("SELECT SUM(debetsnt) FROM Vienti");
    varmistussql.next();
    qDebug() << " VANHA " << vanhasumma << " UUSI " << varmistussql.value(0).toLongLong();
    if( varmistussql.value(0).toLongLong() == vanhasumma)
        ui->pino->setCurrentIndex(VALMIS);
    else {
        ui->pino->setCurrentIndex(SUMMAVIRHE);
    }
    ui->peruNappi->setEnabled(true);
}

void VanhatuontiDlg::lisaaIbanit()
{
    QSqlQuery sql( kpdb_);
    sql.exec("SELECT nro, json FROM Tili WHERE tyyppi='ARP'");
    while( sql.next()) {
        int numero = sql.value("nro").toInt();
        QVariantMap jsonmap = QJsonDocument::fromJson( sql.value("json").toByteArray() ).toVariant().toMap();
        QString iban = jsonmap.value("IBAN").toString();
        if( !numero || iban.isEmpty())
            continue;
        for(int i=0; i < kitsasTilit_.count(); i++) {
            if( kitsasTilit_.value(i).toMap().value("numero").toInt() == numero) {
                QVariantMap map = kitsasTilit_.at(i).toMap();
                map.insert("iban", iban);
                kitsasTilit_[i] = map;
            }
        }
    }
}

void VanhatuontiDlg::siirraAsetukset()
{
    QVariantMap map;


    QStringList siirrettavat;
    siirrettavat << "AlvVelvollinen" << "AlvKausi" << "Harjoitus" << "Kotipaikka" << "LaskuSeuraavaId" << "LogossaNimi"
                 << "Puhelin" << "Tilinavaus" << "TilinavausPvm" << "TilitPaatetty" << "Ytunnus"
                 << "LaskuIkkuna" << "LaskuIkkunaX" << "LaskuIkkunaY" << "LaskuIkkunaLeveys" << "LaskuIkkunaKorkeus"
                 << "LaskuRF" << "MaksuAlvAlkaa" << "MaksuAlvLoppuu";

    // Joitain laskutuksen valintoja voisi myös siirtää
    // Laskutilin numero
    if( kitupiikkiAsetukset_.contains("LaskuTili")) {
        QSqlQuery sql( kpdb_);
        sql.exec(QString("SELECT json FROM Tili WHERE nro=%1 AND tyyppi='ARP'").arg(kitupiikkiAsetukset_.value("LaskuTili").toInt()) );
        if( sql.next()) {
            QVariantMap jsonmap = QJsonDocument::fromJson( sql.value("json").toByteArray() ).toVariant().toMap();
            map.insert("LaskuIbanit", jsonmap.value("IBAN"));
        }
    }


    for(auto siirrettava : siirrettavat)
        if( kitupiikkiAsetukset_.contains(siirrettava))
            map.insert(siirrettava, kitupiikkiAsetukset_.value(siirrettava));

    QStringList osoiteRivit = kitupiikkiAsetukset_.value("Osoite").split('\n');
    if( osoiteRivit.count() > 1) {
        map.insert("Katuosoite", osoiteRivit.mid(0, osoiteRivit.count()-1).join('\n') );
        QString vikarivi = osoiteRivit.last();
        int vali = vikarivi.indexOf(" ");
        map.insert("Postinumero", vikarivi.left(vali));
        map.insert("Kaupunki", vikarivi.mid(vali+1));
    }



    map.insert("AlvAlkaa", QDate::fromString(kitupiikkiAsetukset_.value("TilitPaatetty"), Qt::ISODate).addDays(1));
    map.insert("Email", kitupiikkiAsetukset_.value("Sahkoposti"));
    map.insert("LaskuTilisiirto", !kitupiikkiAsetukset_.contains("LaskuEiTilisiirto"));
    map.insert("LaskuTilisiirto", !kitupiikkiAsetukset_.contains("LaskuEiViivakoodi"));
    map.insert("LaskuQR", !kitupiikkiAsetukset_.contains("LaskuEiQR"));


    // Tositteiden numerointiperiaate
    if( !kitupiikkiAsetukset_.contains("SamaanSarjaan") && !kitupiikkiAsetukset_.contains("Samaansarjaan")) {
        map.insert("erisarjaan","ON");
        erisarja_ = true;
        QSqlQuery sql(kpdb_);
        sql.exec("SELECT id FROM Tositelaji WHERE tunnus='K'");
        if( sql.next())
            map.insert("kateissarjaan", "ON");
    }


    KpKysely *kysely = kpk("/asetukset", KpKysely::PATCH);
    kysely->kysy(map);

}

void VanhatuontiDlg::siirraTilikaudet()
{
    QSqlQuery sql(kpdb_);
    sql.exec("SELECT alkaa, loppuu, json FROM Tilikausi ORDER BY alkaa");
    while( sql.next()) {        
        KpKysely *kysely = kpk(QString("/tilikaudet/%1").arg(sql.value(0).toDate().toString(Qt::ISODate)), KpKysely::PUT);
        QVariantMap map;
        map.insert("alkaa", sql.value(0));
        map.insert("loppuu", sql.value(1));       

        QVariantMap jsonMap = QJsonDocument::fromJson( sql.value(2).toByteArray() ).toVariant().toMap();
        map.insert("arkisto", jsonMap.value("Arkistoitu"));
        map.insert("henkilosta", jsonMap.value("Henkilosto"));
        map.insert("vahvistettu", jsonMap.value("Vahvistettu"));

        kysely->kysy(map);

        tilikausipaivat_.insert(sql.value(0).toString(), sql.value(1).toString());

        if( map.contains("Budjetti")) {
            kysely = kpk( QString("/budjetti/%1").arg( sql.value(0).toString() ), KpKysely::PUT);
            kysely->kysy( map.value("Budjetti") );
        }



    }
}

void VanhatuontiDlg::taydennaTilit()
{
    QSqlQuery sql(kpdb_);
    sql.exec("SELECT nro, nimi, tyyppi, tila, json, muokattu, id FROM Tili");
    while( sql.next()) {        
        int numero = tilimuunto(sql.value(0).toInt());

        QString tyyppi = sql.value(2).toString();

        bool loytyi = false;
        if( sql.value("muokattu").isNull()) {
            for( auto& item : kitsasTilit_) {
                QVariantMap tmap = item.toMap();
                if( (!tyyppi.startsWith('H') || tyyppi == tmap.value("tyyppi").toString() )
                     && numero == tmap.value("numero").toInt() ) {
                    loytyi = true;
                    break;
                }
            }
        }
        // Jos Kitsaassa ei ole vastaavaa tiliä se lisätään.
        // Samoin muokkaukseen päädytään, jos tiliä on muokattu
        if( !loytyi ) {
            QVariantMap tmap;
            tmap.insert("numero", numero);
            tmap.insert("tyyppi", tyyppi);

            QVariantMap nimiMap;
            nimiMap.insert("fi", sql.value("nimi"));
            tmap.insert("nimi", nimiMap);

            // Nyt JSONista pitäisi hakea kaikenlaista mielenkiintoista ;)
            QVariantMap jsonmap = QJsonDocument::fromJson( sql.value("json").toByteArray() ).toVariant().toMap();
            if( jsonmap.contains("AlvLaji"))
                tmap.insert("alvlaji", jsonmap.value("AlvLaji"));
            if( jsonmap.contains("AlvProsentti"))
                tmap.insert("alvprosentti", jsonmap.value("AlvProsentti"));
            if( jsonmap.contains("Kirjausohje"))
                tmap.insert("ohje", jsonmap.value("Kirjausohje"));
            if( jsonmap.contains("Tasaerapoisto"))
                tmap.insert("tasaerapoisto", jsonmap.value("Tasaerapoisto"));
            if( jsonmap.contains("Menojaannospoisto"))
                tmap.insert("menojaannospoisto", jsonmap.value("Menojaannospoisto"));
            if( jsonmap.contains("Poistotili"))
                tmap.insert("poistotili", tilimuunto(jsonmap.value("Poistotili").toInt()));
            if( jsonmap.contains("Taseerittely"))
                tmap.insert("erittely", jsonmap.value("Taseerittely"));


            KpKysely *kysely = kpk("/tilit", KpKysely::PUT);
            kysely->kysy(tmap);
        }
    }
}

void VanhatuontiDlg::siirraKohdennukset()
{
    QSqlQuery sql(kpdb_);
    sql.exec("SELECT id, nimi, tyyppi, alkaa, loppuu, tyyppi, json FROM Kohdennus");
    while( sql.next()) {
        int id = sql.value(0).toInt();
        int tyyppi = sql.value("tyyppi").toInt();
        if( id == 0)
            continue;
        QVariantMap kmap;
        QVariantMap nimimap;
        nimimap.insert("fi", sql.value(1).toString());
        kmap.insert("nimi",nimimap);
        kmap.insert("tyyppi", tyyppi);
        kmap.insert("alkaa", sql.value("alkaa"));
        kmap.insert("loppuu", sql.value("loppuu"));
        if( tyyppi == 2 )   // Projekti
            kmap.insert("kuuluu", QVariant());

        KpKysely* kysely = kpk( QString("/kohdennukset/%1").arg(id), KpKysely::PUT);
        kysely->kysy(kmap);
    }
}


void VanhatuontiDlg::siirraTuotteet()
{
    QSqlQuery sql(kpdb_);
    sql.exec("SELECT Tuote.id AS id, nimike, yksikko, hintaSnt, alvkoodi, alvprosentti, kohdennus, Tili.nro AS tili FROM Tuote JOIN Tili ON Tuote.tili=Tili.id");
    while( sql.next()) {
        QVariantMap tmap;
        tmap.insert("nimike", sql.value("nimike"));
        tmap.insert("yksikko", sql.value("yksikko"));
        tmap.insert("ahinta", sql.value("hintaSnt").toDouble() / 100.0);
        tmap.insert("alvkoodi", sql.value("alvkoodi").toInt());
        tmap.insert("alvprosentti", sql.value("alvprosentti").toDouble());
        tmap.insert("kohdennus", sql.value("kohdennus").toInt());
        tmap.insert("tili", sql.value("tili").toInt());

        KpKysely *kysely = kpk(QString("/tuotteet/%1").arg(sql.value("id").toInt()), KpKysely::PUT);
        kysely->kysy(tmap);
    }
}

void VanhatuontiDlg::siirraAsiakkaat()
{
    QRegularExpression postiRe("(\\d{5})\\s(\\w{2,}.*)");
    QSet<QString> saajatiedosta;

    QSqlQuery sql(kpdb_);
    sql.exec("SELECT asiakas, json FROM Vienti WHERE asiakas NOT NULL ORDER BY muokattu DESC");
    while( sql.next()) {
        QString nimi = sql.value(0).toString();
        QVariantMap tiedot = QJsonDocument::fromJson(sql.value(1).toByteArray()).toVariant().toMap();
        bool saatuSaajasta = nimi.isEmpty();

        if( nimi.isEmpty() && tiedot.contains("SaajanNimi") ) {
            nimi = tiedot.value("SaajanNimi").toString();
            saajatiedosta.insert(nimi);
        }

        if( nimi.isEmpty() || (asiakasIdt_.contains(nimi) && !saajatiedosta.contains(nimi)))
            continue;       // Jokaisesta asiakkaasta huomioidaan vain viimeisimmät tiedot
        if( !saatuSaajasta) // Kuitenkin, jos saatu saajatiedosta, niin haetaan mieluummin laskulta
            saajatiedosta.remove(nimi);     // jossa niitä tietojakin löytyy

        QVariantMap asiakasMap;
        asiakasMap.insert("nimi", nimi);
        asiakasMap.insert("maa", "fi");

        if( tiedot.contains("Osoite")) {
            QString postinumero;
            QString kaupunki;
            QStringList osoitelista = tiedot.value("Osoite").toString().split('\n');
            for(int i=1; i < osoitelista.count(); i++) {
                QRegularExpressionMatch mats = postiRe.match(osoitelista.value(i));
                if( mats.hasMatch()) {
                    postinumero = mats.captured(1);
                    kaupunki = mats.captured(2);
                    osoitelista.removeAt(i);
                    break;
                }
            }
            asiakasMap.insert("osoite", osoitelista.join('\n'));
            asiakasMap.insert("postinumero", postinumero);
            asiakasMap.insert("kaupunki", kaupunki);
        }
        if( tiedot.contains("Email"))
            asiakasMap.insert("email", tiedot.value("Email"));
        if( tiedot.contains("YTunnus")) {
            asiakasMap.insert("alvtunnus", AsiakasToimittajaDlg::yToAlv(tiedot.value("YTunnus").toString()));
        }
        if( tiedot.contains("VerkkolaskuOsoite")) {
            asiakasMap.insert("ovt", tiedot.value("VerkkolaskuOsoite"));
            asiakasMap.insert("operaattori", tiedot.value("VerkkolaskuValittaja"));
        }
        KpKysely *kysely = kpk("/kumppanit", KpKysely::POST);
        connect(kysely, &KpKysely::vastaus, this, &VanhatuontiDlg::tallennaAsiakasId);
        kysely->kysy(asiakasMap);
    }

}

void VanhatuontiDlg::tallennaAsiakasId(QVariant *data)
{
    QVariantMap map = data->toMap();
    int id = map.value("id").toInt();
    QString nimi = map.value("nimi").toString();
    asiakasIdt_.insert(nimi, id);
}

void VanhatuontiDlg::siirraTositteet()
{
    QSqlQuery tositekysely(kpdb_);
    tositekysely.setForwardOnly(true);
    tositekysely.exec("SELECT Tosite.id as id, pvm, otsikko, kommentti, tunniste, tosite.json as json, tunnus, Tosite.laji AS laji FROM Tosite JOIN Tositelaji ON Tosite.laji=Tositelaji.id WHERE Tosite.pvm NOT NULL");
    QSqlQuery vientikysely(kpdb_);
    vientikysely.setForwardOnly(true);
    QSqlQuery merkkauskysely( kpdb_ );
    merkkauskysely.setForwardOnly(true);
    while( tositekysely.next()) {
        std::cout << "T";
        std::cout.flush();
        int tositeid = tositekysely.value("id").toInt();
        Tosite tosite;
        QDate pvm = tositekysely.value("pvm").toDate();
        tosite.asetaPvm(pvm);
        tosite.asetaTyyppi(TositeTyyppi::TUONTI);
        tosite.asetaOtsikko(tositekysely.value("otsikko").toString());
        tosite.setData(Tosite::LISATIEDOT, tositekysely.value("kommentti"));
        if( erisarja_)
            tosite.asetaSarja(tositekysely.value("tunnus").toString());
        else
            tosite.asetaSarja("");

        if( ui->sailytaNumerointiCheck)
            tosite.setData(Tosite::TUNNISTE, tositekysely.value("tunniste"));

        QVariantMap map = QJsonDocument::fromJson(tositekysely.value("json").toByteArray()).toVariant().toMap();
        if( map.contains("AlvTilitysAlkaa") && tositekysely.value("laji").toInt() == 0) {
            QVariantMap alvmap;
            alvmap.insert("kausialkaa", map.value("AlvTilitysAlkaa"));
            alvmap.insert("kausipaattyy", map.value("AlvTilitysPaattyy"));
            alvmap.insert("maksettava", map.value("MaksettavaAlv").toDouble() / 100.0);
            if( map.contains("Voittomarginaalialijaama")) {
                QVariantMap voittolukumap = map.value("Voittomarginaalialijaama").toMap();
                QVariantMap voittoMap;
                QMapIterator<QString,QVariant> iter(voittolukumap);
                while( iter.hasNext()) {
                    iter.next();
                    voittoMap.insert(iter.key() + ".00", iter.value().toDouble() / 100.0);
                }
                alvmap.insert("marginaalialijaama", voittoMap);
            }
            tosite.setData(Tosite::ALV, alvmap);
            tosite.asetaTyyppi(TositeTyyppi::ALVLASKELMA);
        }

        vientikysely.exec(QString("SELECT vienti.id as id, pvm, tili.nro as tilinumero, debetsnt, kreditsnt, selite, alvkoodi, alvprosentti, "
                                  "kohdennus, eraid, viite, iban, laskupvm, erapvm, "
                          "arkistotunnus, asiakas, vienti.json FROM Vienti "
                          "LEFT OUTER JOIN Tili ON Vienti.tili=Tili.id   WHERE tosite=%1 ORDER BY vientirivi").arg(tositeid));
        while(vientikysely.next()) {
            TositeVienti vienti;
            int vientiid = vientikysely.value("id").toInt();
            vienti.setId( vientiid );
            QDate vientiPvm = vientikysely.value("pvm").toDate();
            int alkuptili = vientikysely.value("tilinumero").toInt();
            if( alkuptili == 0)
                vientiPvm = tosite.pvm();    // Maksuperusteiset laskut kirjanpitoon kokonaan maksupäivänä
            vienti.setPvm( vientiPvm );
            int tili =  tilimuunto( alkuptili );
            vienti.setTili( tili );
            qlonglong debetsnt = vientikysely.value("debetsnt").toLongLong();
            qlonglong kreditsnt = vientikysely.value("kreditsnt").toLongLong();

            if( debetsnt < 0) {
                kreditsnt -= debetsnt;
                debetsnt = 0;
            }

            if( kreditsnt < 0) {
                debetsnt -= kreditsnt;
                kreditsnt = 0;
            }

            if( debetsnt > kreditsnt)
                vienti.setDebet(debetsnt - kreditsnt);
            else if( kreditsnt)
                vienti.setKredit( kreditsnt - debetsnt);
            vienti.setSelite( vientikysely.value("selite").toString());
            vienti.setAlvKoodi( vientikysely.value("alvkoodi").toInt());
            vienti.setAlvProsentti( vientikysely.value("alvprosentti").toDouble());
            vienti.setKohdennus( vientikysely.value("kohdennus").toInt());
            vienti.setEra( vientikysely.value("eraid").toInt());
            vienti.setArkistotunnus( vientikysely.value("arkistotunnus").toString());

            if( vientikysely.value("laskupvm").toDate().isValid())
                tosite.asetaLaskupvm(vientikysely.value("laskupvm").toDate());
            if( vientikysely.value("viite").toString().length()>1)
                tosite.asetaViite(vientikysely.value("viite").toString());
            if( vientikysely.value("erapvm").toDate().isValid())
                tosite.asetaErapvm( vientikysely.value("erapvm").toDate());

            QVariantMap vientiJson = QJsonDocument::fromJson( vientikysely.value("json").toByteArray() ).toVariant().toMap();

            // Asiakas tai toimittaja
            QString asiakasToimittaja = vientikysely.value("asiakas").toString();
            if( asiakasToimittaja.isEmpty())
                asiakasToimittaja = vientiJson.value("SaajanNimi").toString();
            if( !asiakasToimittaja.isEmpty() && asiakasIdt_.contains(asiakasToimittaja)) {
                vienti.setKumppani(asiakasIdt_.value(asiakasToimittaja));
                tosite.asetaKumppani(asiakasIdt_.value(asiakasToimittaja));
            }

            // Myyntilaskujen ja ostolaskujen tyypit
            // Näin saadaan laskut seurantaan
            Tili tilio = kp()->tilit()->tiliNumerolla(tili);
            if( vientiid == vientikysely.value("eraid").toInt()) {
                if( tilio.onko(TiliLaji::OSTOVELKA))
                    vienti.setTyyppi(TositeTyyppi::MENO + TositeVienti::VASTAKIRJAUS);
                else if( tilio.onko(TiliLaji::MYYNTISAATAVA))
                    vienti.setTyyppi(TositeTyyppi::TULO + TositeVienti::VASTAKIRJAUS);
            }

            // Merkkaukset
            QVariantList merkkaukset;
            merkkauskysely.exec(QString("SELECT kohdennus FROM Merkkaus WHERE vienti=%1").arg(vientiid));
            while( merkkauskysely.next() ) {
                merkkaukset.append( merkkauskysely.value(0) );
            }
            if( !merkkaukset.isEmpty())
                vienti.setMerkkaukset(merkkaukset);

            if( vientiJson.contains("Laskurivit"))
                laskuTiedot(vientikysely, tosite );

            if( vientiJson.contains("Tasaerapoisto"))
                vienti.setTasaerapoisto( vientiJson.value("Tasaerapoisto").toInt() );


            if( !tilio.onkoValidi()) {
                qDebug() << " ******* TILIVIRHE ************ " << tili;
                continue;   // Maksuperusteisten maksunvalvontariviä ei voi lisätä
            }
            tosite.viennit()->lisaa(vienti);

        }
        connect(&tosite, &Tosite::tallennusvirhe, [] (int virhe) { qDebug() << "TOSITEVIRHE " << virhe;});
        tosite.tallenna();
        ui->progressBar->setValue( ui->progressBar->value() + 1 );
        siirraLiiteet(tositeid, tosite.id());
        qApp->processEvents();
    }
}

void VanhatuontiDlg::laskuTiedot(const QSqlQuery &vientikysely, Tosite &tosite)
{
    QVariantMap vientiJson = QJsonDocument::fromJson( vientikysely.value("json").toByteArray() ).toVariant().toMap();
    QVariantList tuontiRivit = vientiJson.value("Laskurivit").toList();

    QVariantList rivit;
    for(auto item : tuontiRivit) {
        QVariantMap map = item.toMap();
        QVariantMap riviMap;

        riviMap.insert("tuote", map.value("Tuotekoodi"));
        riviMap.insert("myyntikpl", map.value("Maara"));
        riviMap.insert("ahinta", map.value("YksikkohintaSnt").toDouble() / 100.0);
        riviMap.insert("nimike", map.value("Nimike"));
        riviMap.insert("yksikko", map.value("Yksikko"));
        riviMap.insert("tili", tilimuunto( map.value("tili").toInt() ));
        riviMap.insert("kohdennus", map.value("Kohdennus"));
        riviMap.insert("alvkoodi", map.value("Alvkoodi"));
        riviMap.insert("alvprosentti", map.value("Alvprosentti").toDouble());
        riviMap.insert("aleprosentti", map.value("AleProsentti").toDouble());
        rivit.append(riviMap);
    }

    tosite.rivit()->lataa(rivit);

    QVariantMap lasku;
    tosite.lasku().setOsoite(vientiJson.value("Osoite").toString());
    tosite.lasku().setEmail(vientiJson.value("Email").toString());
    tosite.lasku().setAsiakasViite(vientiJson.value("AsiakkaanViite").toString());
    tosite.lasku().setKieli(vientiJson.value("Kieli").toString());
    tosite.lasku().setViivastyskorko(vientiJson.value("Viivastyskorko").toDouble());
    tosite.lasku().setNumero(vientikysely.value("viite").toString());
    tosite.lasku().setErapaiva( vientikysely.value("erapvm").toDate());
    tosite.lasku().setLaskunpaiva(vientikysely.value("laskupvm").toDate());
    tosite.lasku().setLahetystapa(Lasku::TUOTULASKU);

    lasku.insert("maksutapa", Lasku::LASKU);
    int kirjausperuste = vientiJson.value("Kirjausperuste").toInt();
    if( kirjausperuste == 0)
        tosite.lasku().setMaksutapa(Lasku::SUORITEPERUSTE);
    else if( kirjausperuste == 3)
        tosite.lasku().setMaksutapa(Lasku::KATEINEN);

/*    if( vientiJson.contains("Hyvityslasku"))
        tosite.asetaTyyppi( TositeTyyppi::HYVITYSLASKU);
    else if( vientiJson.contains("Maksumuistutus"))
        tosite.asetaTyyppi( TositeTyyppi::MAKSUMUISTUTUS);
    else
        tosite.asetaTyyppi(TositeTyyppi::MYYNTILASKU);
*/

}

void VanhatuontiDlg::siirraLiiteet(int vanhaTositeId, int uusiTositeId)
{
    QSqlQuery sql(kpdb_);
    sql.setForwardOnly(true);
    sql.exec(QString("SELECT liite.id FROM Liite WHERE tosite=%1 ORDER BY liiteno").arg(vanhaTositeId));
    while( sql.next()) {
        siirraLiite( sql.value(0).toInt(), uusiTositeId );
        ui->progressBar->setValue( ui->progressBar->value() + 1 );
        qApp->processEvents();
    }
}

void VanhatuontiDlg::siirraLiite(int id, int uusiTositeId)
{
    QSqlQuery sql(kpdb_);
    sql.setForwardOnly(true);
    sql.exec(QString("SELECT tosite, liite.otsikko, data, tosite.json FROM Liite LEFT OUTER JOIN Tosite ON Liite.tosite=Tosite.id WHERE liite.id=%1").arg(id));
    while( sql.next()) {
        std::cout << "L";
        std::cout.flush();
        int tosite = sql.value(0).toInt();
        QString otsikko = sql.value(1).toString();
        QByteArray data = sql.value(2).toByteArray();

        if( sql.value(3).toString().startsWith("{\"Lasku\":") ) {
            KpKysely* kysely = kpk(QString("/liitteet/%1/lasku").arg(uusiTositeId), KpKysely::PUT);
            kysely->lahetaTiedosto(data);
            delete kysely;
        }
        else if( tosite == 0) {
            // Tilinpäätöksen tallentuminen oikealla roolinimellä
            if( tilikausipaivat_.contains(otsikko))
                otsikko = "TP_" + tilikausipaivat_.value(otsikko);

            KpKysely* kysely = kpk(QString("/liitteet/0/%1").arg(otsikko), KpKysely::PUT);
            kysely->lahetaTiedosto(data);
            delete kysely;
        } else {
            KpKysely* kysely = kpk(QString("/liitteet/%1").arg(uusiTositeId), KpKysely::POST);
            kysely->lahetaTiedosto(data);
            delete kysely;
        }
        ui->progressBar->setValue( ui->progressBar->value() + 1 );
        qApp->processEvents();
    }
}

void VanhatuontiDlg::siirraLogo()
{
    QSqlQuery sql(kpdb_);
    sql.setForwardOnly(true);
    sql.exec(QString("SELECT data FROM Liite WHERE tosite IS NULL AND otsikko = 'logo'"));
    if( sql.next()) {
        QByteArray data = sql.value("data").toByteArray();
        KpKysely *kysely = kpk("/liitteet/0/logo", KpKysely::PUT);
        kysely->lahetaTiedosto(data);
        delete kysely;
    }
}


int VanhatuontiDlg::tilimuunto(int tilinumero) const
{
    return tilinMuunto_.value(tilinumero, tilinumero);
}

