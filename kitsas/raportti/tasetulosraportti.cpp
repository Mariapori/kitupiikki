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
#include "tasetulosraportti.h"
#include "db/kirjanpito.h"
#include "kieli/kielet.h"

#include <QJsonDocument>
#include <QStringListModel>
#include <QDebug>
#include <QMessageBox>

TaseTulosRaportti::TaseTulosRaportti(Raportoija::RaportinTyyppi raportinTyyppi, QWidget *parent) :
    RaporttiWidget (parent),
    ui( new Ui::MuokattavaRaportti ),
    tyyppi_(raportinTyyppi)

{
    ui->setupUi( raporttiWidget );

    paivitaMuodot();
    muotoVaihtui();

    connect( ui->muotoCombo, &QComboBox::currentTextChanged, this, &TaseTulosRaportti::muotoVaihtui);
    connect( ui->kieliCombo, &QComboBox::currentTextChanged, this, &TaseTulosRaportti::paivitaMuodot);

    if( tyyppi() == Raportoija::PROJEKTILASKELMA) {
        ui->kohdennusCheck->setVisible( kp()->kohdennukset()->kustannuspaikkoja() );
        ui->kohdennusCombo->setVisible( kp()->kohdennukset()->kustannuspaikkoja());
        ui->kohdennusCheck->setText( tr("Kustannuspaikalla"));
        ui->kohdennusCombo->valitseNaytettavat(KohdennusProxyModel::KUSTANNUSPAIKAT);
    }
    else if( tyyppi()!=Raportoija::TULOSLASKELMA || ( !kp()->kohdennukset()->kohdennuksia() && !kp()->kohdennukset()->merkkauksia() ))
    {
        ui->kohdennusCheck->setVisible(false);
        ui->kohdennusCombo->setVisible(false);
    } else {
        ui->kohdennusCombo->valitseNaytettavat(KohdennusProxyModel::KAIKKI);
    }

    QStringList tyyppiLista;
    tyyppiLista << tr("Toteutunut") << tr("Budjetti") << tr("Budjettiero €") << tr("Toteutunut %");
    QStringListModel *tyyppiListaModel = new QStringListModel(this);
    tyyppiListaModel->setStringList(tyyppiLista);

    ui->tyyppi1->setModel(tyyppiListaModel);
    ui->tyyppi2->setModel(tyyppiListaModel);
    ui->tyyppi3->setModel(tyyppiListaModel);
    ui->tyyppi4->setModel(tyyppiListaModel);

    // Jos alkupäivämäärä on tilikauden aloittava, päivitetään myös päättymispäivä tilikauden päättäväksi
    connect( ui->alkaa1Date, &QDateEdit::dateChanged, this, [this](const QDate& date){  if( kp()->tilikaudet()->tilikausiPaivalle(date).alkaa() == date) this->ui->loppuu1Date->setDate( kp()->tilikaudet()->tilikausiPaivalle(date).paattyy() );  });
    connect( ui->alkaa2Date, &QDateEdit::dateChanged, this, [this](const QDate& date){  if( kp()->tilikaudet()->tilikausiPaivalle(date).alkaa() == date) this->ui->loppuu2Date->setDate( kp()->tilikaudet()->tilikausiPaivalle(date).paattyy() );  });
    connect( ui->alkaa3Date, &QDateEdit::dateChanged, this, [this](const QDate& date){  if( kp()->tilikaudet()->tilikausiPaivalle(date).alkaa() == date) this->ui->loppuu3Date->setDate( kp()->tilikaudet()->tilikausiPaivalle(date).paattyy() );  });
    connect( ui->alkaa4Date, &QDateEdit::dateChanged, this, [this](const QDate& date){  if( kp()->tilikaudet()->tilikausiPaivalle(date).alkaa() == date) this->ui->loppuu4Date->setDate( kp()->tilikaudet()->tilikausiPaivalle(date).paattyy() );  });

    for(QObject* widget : ui->pvmKehys->children()) {
        QDateEdit* date = qobject_cast<QDateEdit*>(widget);
        if(date)
            connect(date, &QDateEdit::dateChanged, this, &TaseTulosRaportti::paivitaKohdennusPaivat);
        else {
            QCheckBox* box = qobject_cast<QCheckBox*>(widget);
            if(box)
                connect(box, &QCheckBox::toggled, this, &TaseTulosRaportti::paivitaKohdennusPaivat);
        }
    }

    paivitaUi();
}

/*
void TaseTulosRaportti::esikatsele()
{
    Raportoija *raportoija = new Raportoija( ui->muotoCombo->currentData().toString(),
                                             ui->kieliCombo->currentData().toString(),
                                             this,
                                             tyyppi());



    if( raportoija->onkoKausiraportti())
    {
        raportoija->lisaaKausi( ui->alkaa1Date->date(), ui->loppuu1Date->date(), ui->tyyppi1->currentIndex());
        if( ui->sarake2Box->isChecked())
            raportoija->lisaaKausi( ui->alkaa2Date->date(), ui->loppuu2Date->date(), ui->tyyppi2->currentIndex());
        if( ui->sarake3Box->isChecked())
            raportoija->lisaaKausi( ui->alkaa3Date->date(), ui->loppuu3Date->date(), ui->tyyppi3->currentIndex());
        if( ui->sarake4Box->isChecked())
            raportoija->lisaaKausi( ui->alkaa4Date->date(), ui->loppuu4Date->date(), ui->tyyppi4->currentIndex());
    }
    else
    {
        raportoija->lisaaTasepaiva( ui->loppuu1Date->date());
        if( ui->sarake2Box->isChecked())
            raportoija->lisaaTasepaiva( ui->loppuu2Date->date());
        if( ui->sarake3Box->isChecked())
            raportoija->lisaaTasepaiva( ui->loppuu3Date->date());
        if( ui->sarake4Box->isChecked())
            raportoija->lisaaTasepaiva( ui->loppuu4Date->date());
    }

    connect( raportoija, &Raportoija::valmis, this, &RaporttiWidget::nayta);
    connect( raportoija, &Raportoija::tyhjaraportti, this, [this] () { this->nayta(RaportinKirjoittaja()); });

    raportoija->kirjoita( ui->erittelyCheck->isChecked(),
                          ui->kohdennusCheck->isChecked() ? ui->kohdennusCombo->kohdennus() : -1);

}
*/

void TaseTulosRaportti::tallenna()
{
    QString tyyppiteksti;
    if( tyyppi() == Raportoija::TASE)
        tyyppiteksti = "tase";
    else if(tyyppi() == Raportoija::KOHDENNUSLASKELMA)
        tyyppiteksti = "kohdennus";
    else if(tyyppi() == Raportoija::PROJEKTILASKELMA)
        tyyppiteksti = "projektit";
    else
        tyyppiteksti = "tulos";

    aseta(RaporttiValinnat::Tyyppi, tyyppiteksti);
    aseta(RaporttiValinnat::RaportinMuoto, ui->muotoCombo->currentData().toString());
    aseta(RaporttiValinnat::Kieli, ui->kieliCombo->currentData().toString());
    aseta(RaporttiValinnat::TulostaErittely, ui->erittelyCheck->isChecked());
    aseta(RaporttiValinnat::Kohdennuksella, ui->kohdennusCombo->currentData().toInt());

    kp()->raporttiValinnat()->tyhjennaSarakkeet();
    lisaaSarake(true, ui->alkaa1Date->date(), ui->loppuu1Date->date(), ui->tyyppi1->currentIndex());
    lisaaSarake(ui->sarake2Box->isChecked(), ui->alkaa2Date->date(), ui->loppuu2Date->date(), ui->tyyppi2->currentIndex());
    lisaaSarake(ui->sarake3Box->isChecked(), ui->alkaa3Date->date(), ui->loppuu3Date->date(), ui->tyyppi3->currentIndex());
    lisaaSarake(ui->sarake4Box->isChecked(), ui->alkaa4Date->date(), ui->loppuu4Date->date(), ui->tyyppi4->currentIndex());


}

void TaseTulosRaportti::muotoVaihtui()
{
    if( paivitetaan_)
        return;

    QString raportti = ui->muotoCombo->currentData().toString();
    QString kaava = kp()->asetukset()->asetus(raportti);
    if( kaava.isEmpty())
        return;
    kaava_ = kaava;
    paivitaKielet();
}

void TaseTulosRaportti::paivitaKielet()
{           
    QJsonDocument doc = QJsonDocument::fromJson( kaava_.toUtf8() );

    QVariantMap kielet = doc.toVariant().toMap().value("nimi").toMap();
    ui->kieliCombo->clear();

    for(const auto& kieli : kielet.keys()) {
        ui->kieliCombo->addItem( QIcon(":/liput/" + kieli + ".png"), kp()->asetukset()->kieli(kieli), kieli );
    }
    int kieliIndeksi = ui->kieliCombo->findData( Kielet::instanssi()->nykyinen() );
    if( kieliIndeksi > -1)
        ui->kieliCombo->setCurrentIndex( kieliIndeksi );
}

void TaseTulosRaportti::paivitaMuodot()
{
    paivitetaan_ = true;

    QStringList muodot;
    if(tyyppi() == Raportoija::TASE )
        muodot = kp()->asetukset()->avaimet("tase/");
    else
        muodot = kp()->asetukset()->avaimet("tulos/");    

    QString nykymuoto = ui->muotoCombo->currentData().toString();
    if( nykymuoto.isEmpty())
        nykymuoto=tyyppi() == Raportoija::TASE ? "tase/yleinen" : "tulos/yleinen";

    QString kieli = ui->kieliCombo->currentData().toString().isEmpty() ? "fi" : ui->kieliCombo->currentData().toString();

    ui->muotoCombo->clear();

    for( const auto& muoto : qAsConst( muodot )) {
        QString kaava = kp()->asetukset()->asetus(muoto);
        QJsonDocument doc = QJsonDocument::fromJson( kaava.toUtf8() );
        QVariantMap map = doc.toVariant().toMap().value("muoto").toMap();
        QString muotonimi = map.value( kieli ).toString();
        ui->muotoCombo->addItem( muotonimi, muoto );
    }

    ui->muotoCombo->setCurrentIndex( ui->muotoCombo->findData(nykymuoto) );
    paivitetaan_ = false;
}

void TaseTulosRaportti::paivitaKohdennusPaivat()
{
    QDate pienin = ui->alkaa1Date->date();
    QDate isoin = ui->loppuu1Date->date();

    if( ui->sarake2Box->isChecked() && ui->alkaa2Date->date() < pienin) pienin = ui->alkaa2Date->date();
    if( ui->sarake3Box->isChecked() && ui->alkaa3Date->date() < pienin) pienin = ui->alkaa3Date->date();
    if( ui->sarake4Box->isChecked() && ui->alkaa4Date->date() < pienin) pienin = ui->alkaa4Date->date();
    if( ui->sarake2Box->isChecked() && ui->loppuu2Date->date() > isoin) isoin = ui->loppuu2Date->date();
    if( ui->sarake3Box->isChecked() && ui->loppuu3Date->date() > isoin) isoin = ui->loppuu3Date->date();
    if( ui->sarake4Box->isChecked() && ui->loppuu4Date->date() > isoin) isoin = ui->loppuu4Date->date();

    ui->kohdennusCombo->suodataValilla(pienin, isoin);
}

void TaseTulosRaportti::paivitaUi()
{
    // Jos tehdään Raportoija::TASElaskelmaa, piilotetaan turhat tiedot!
    ui->alkaa1Date->setVisible( tyyppi() != Raportoija::TASE );
    ui->alkaa2Date->setVisible( tyyppi() != Raportoija::TASE );
    ui->alkaa3Date->setVisible( tyyppi() != Raportoija::TASE );
    ui->alkaa4Date->setVisible( tyyppi() != Raportoija::TASE );
    ui->alkaaLabel->setVisible( tyyppi() != Raportoija::TASE );
    ui->paattyyLabel->setVisible( tyyppi() != Raportoija::TASE );

    ui->tyyppi1->setVisible( tyyppi() != Raportoija::TASE);
    ui->tyyppi2->setVisible( tyyppi() != Raportoija::TASE);
    ui->tyyppi3->setVisible( tyyppi() != Raportoija::TASE);
    ui->tyyppi4->setVisible( tyyppi() != Raportoija::TASE);


    // Sitten laitetaan valmiiksi tilikausia nykyisestä taaksepäin
    int tilikausiIndeksi = kp()->tilikaudet()->indeksiPaivalle( kp()->paivamaara() );
    // #160 Tai sitten viimeinen tilikausi
    if( tilikausiIndeksi < 0)
        tilikausiIndeksi = kp()->tilikaudet()->rowCount(QModelIndex()) - 1;

    if( tilikausiIndeksi > -1 )
    {
        ui->alkaa1Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).alkaa() );
        ui->loppuu1Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).paattyy() );
    }
    if( tilikausiIndeksi > 0)
    {
        ui->alkaa2Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi-1).alkaa() );
        ui->loppuu2Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi-1).paattyy() );
    }
    else if( tilikausiIndeksi > -1)
    {
        ui->alkaa2Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).alkaa() );
        ui->loppuu2Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).paattyy() );
    }


    ui->sarake2Box->setChecked(tilikausiIndeksi > 0);

    if( tilikausiIndeksi > 1)
    {
        ui->alkaa3Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi-2).alkaa() );
        ui->loppuu3Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi-2).paattyy() );
    }
    else if( tilikausiIndeksi > -1)
    {
        ui->alkaa3Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).alkaa() );
        ui->loppuu3Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).paattyy() );
    }


    ui->sarake3Box->setChecked(tilikausiIndeksi > 1);

    // Neljäs sarake oletuksena poissa käytöstä
    // Sitä tarvitaan lähinnä budjettivertailuihin
    if( tilikausiIndeksi > -1)
    {
        ui->alkaa4Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).alkaa() );
        ui->loppuu4Date->setDate( kp()->tilikaudet()->tilikausiIndeksilla(tilikausiIndeksi).paattyy() );
    }
    ui->sarake4Box->setChecked(false);
}

void TaseTulosRaportti::lisaaSarake(bool kaytossa, const QDate &alku, const QDate &loppu, int valintaIndeksi)
{
    if( kaytossa ) {
        RaporttiValintaSarake::SarakeTyyppi sarakeTyyppi = RaporttiValintaSarake::Toteutunut;
        if( valintaIndeksi == RaporttiValintaSarake::Budjetti)
            sarakeTyyppi = RaporttiValintaSarake::Budjetti;
        else if(valintaIndeksi == RaporttiValintaSarake::BudjettiEro)
            sarakeTyyppi = RaporttiValintaSarake::BudjettiEro;
        else if(valintaIndeksi == RaporttiValintaSarake::ToteumaProsentti)
            sarakeTyyppi = RaporttiValintaSarake::ToteumaProsentti;
        kp()->raporttiValinnat()->lisaaSarake(RaporttiValintaSarake(alku, loppu, sarakeTyyppi));
    }
}
