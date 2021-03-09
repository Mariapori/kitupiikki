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
#include "uusivelho.h"

#include "ui_uusiharjoitus.h"
#include "ui_uusitilikartta.h"
#include "ui_uusitiedot.h"
#include "ui_uusiloppu.h"
#include "ui_numerointi.h"
#include "ui_uusivastuu.h"
#include "ui_varmista.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <QDebug>
#include <QPixmap>

#include "uusialkusivu.h"
#include "tilikausisivu.h"
#include "sijaintisivu.h"
#include "tiedotsivu.h"

#include "validator/ibanvalidator.h"
#include "validator/ytunnusvalidator.h"

#include "db/kirjanpito.h"
#include "pilvi/pilvimodel.h"

#include "db/tositetyyppimodel.h"
#include "sqlite/sqlitemodel.h"

#include <iostream>

UusiVelho::UusiVelho(QWidget *parent) :
    QWizard(parent)
{
    setPixmap( QWizard::LogoPixmap, QPixmap(":/pic/possu64.png")  );


    addPage( new UusiAlkuSivu );
    addPage( new VarmistaSivu );
    addPage( new Harjoitussivu );
    addPage( new VastuuSivu );
    addPage( new Tilikarttasivu(this) );
    addPage( new TiedotSivu(this));
    addPage( new TilikausiSivu(this) );
    addPage( new NumerointiSivu );
    addPage( new SijaintiSivu );
    addPage( new LoppuSivu );

    setOption(HaveHelpButton, true);
    connect(this, &UusiVelho::helpRequested, [] { kp()->ohje("aloitus/uusi"); });

}

void UusiVelho::lataaKartta(const QString &polku)
{
    asetukset_ = asetukset(polku);

    // Tilit oma json-tiedosto
    {
        QFile tilit(polku + "/tilit.json");
        if( tilit.open(QIODevice::ReadOnly) ) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(tilit.readAll(), &error);
            QVariant variant = doc.toVariant();
            tilit_ = variant.toList();
        }
    }

}

QVariantMap UusiVelho::data() const
{
    QVariantMap map;
    QVariantMap asetusMap(asetukset_);
    QVariantMap initMap;

    if( field("harjoitus").toBool())
        asetusMap.insert("Harjoitus", "ON");

    if( field("erisarjaan").toBool())
        asetusMap.insert("erisarjaan", "ON");

    if( field("kateissarjaan").toBool())
            asetusMap.insert("kateissarjaan", "ON");

    asetusMap.insert("KpVersio", SQLiteModel::TIETOKANTAVERSIO );
    asetusMap.insert("LuotuVersiolla", qApp->applicationVersion());
    asetusMap.insert("Luotu", QDateTime::currentDateTime());
    asetusMap.insert("UID", Kirjanpito::satujono(16));

    initMap.insert("asetukset", asetusMap);
    initMap.insert("tilit", tilit_);
    initMap.insert("tilikaudet", tilikaudet_);

    map.insert("name", asetukset_.value("Nimi"));
    map.insert("trial", field("harjoitus").toBool());
    map.insert("init", initMap);
    if(!field("ytunnus").toString().isEmpty())
        map.insert("businessid", field("ytunnus").toString());

//    std::cout << QJsonDocument::fromVariant(map).toJson(QJsonDocument::Compact).toStdString();

    return  map;
}

QString UusiVelho::polku() const
{
    if( field("pilveen").toBool())
        return QString();
    return field("sijainti").toString()+"/"+field("tiedosto").toString();
}

int UusiVelho::nextId() const
{
    if( currentId() == ALOITUS &&
            field("pilveen").toBool())
        return HARJOITUS;

    if( currentId() == HARJOITUS &&
            field("harjoitus").toBool())
        return TILIKARTTA;

    if( currentId() == NUMEROINTI &&
            field("pilveen").toBool())
        return LOPPU;

    return QWizard::nextId();
}

QVariantMap UusiVelho::asetukset(const QString &polku)
{
    QVariantMap map;

    // tilikartan tiedot
    QFile asetukset(polku + "/tilikartta.json");
    if( asetukset.open(QIODevice::ReadOnly))
        map = QJsonDocument::fromJson( asetukset.readAll() ).toVariant().toMap();

    // json asetuksille
    {
        QFile asetukset(polku + "/asetukset.json");
        if( asetukset.open(QIODevice::ReadOnly))
            map.unite(QJsonDocument::fromJson( asetukset.readAll() ).toVariant().toMap());
    }
    // json-tiedosto raporteille
    {
        QFile raportit(polku + "/raportit.json");
        if( raportit.open(QIODevice::ReadOnly))
            map.unite(QJsonDocument::fromJson( raportit.readAll() ).toVariant().toMap());

    }

    // Tilinpäätöksen pohja on tekstitiedosto, jossa kielet on merkattu []-tageilla
    // Luetaan osaksi asetuksia
    {
        QFile pohja(polku + "/tilinpaatos.txt");
        QString kieli;
        QStringList rivit;
        if( pohja.open(QIODevice::ReadOnly)) {
            QTextStream luku(&pohja);
            luku.setCodec("utf-8");
            while(!luku.atEnd()) {
                QString rivi = luku.readLine();
                if( rivi.startsWith("[") && rivi.endsWith("]")) {
                    if( rivit.count() )
                        map.insert("tppohja/" + kieli, rivit.join('\n'));
                    rivit.clear();
                    kieli=rivi.mid(1, rivi.length()-2);
                } else
                    rivit.append(rivi);
            }
            map.insert("tppohja/" + kieli,
                       rivit.join('\n'));
        }
    }
    return map;
}



UusiVelho::Harjoitussivu::Harjoitussivu() :
    ui( new Ui::UusiHarjoitus)
{
    ui->setupUi(this);
    setTitle(tr("Harjoitus vai todellinen?"));
    registerField("harjoitus", ui->harjoitusButton);
}


UusiVelho::Tilikarttasivu::Tilikarttasivu(UusiVelho *wizard) :
    ui ( new Ui::UusiTilikartta),
    velho (wizard)
{
    ui->setupUi(this);
    setTitle( tr("Tilikartta"));
}

bool UusiVelho::Tilikarttasivu::validatePage()
{
    if( ui->yhdistysButton->isChecked() )
        velho->lataaKartta(":/tilikartat/yhdistys");
    else if(ui->elinkeinoRadio->isChecked())
        velho->lataaKartta(":/tilikartat/yritys");
    else if(ui->asoyButton->isChecked())
        velho->lataaKartta(":/tilikartat/asoy");

    return true;
}


UusiVelho::NumerointiSivu::NumerointiSivu()
    : ui(new Ui::UusiNumerointi)
{
    ui->setupUi(this);
    setTitle(tr("Tositteiden numerointi"));
    registerField("erisarjaan", ui->erisarjaan);
    registerField("kateissarjaan", ui->kateissarjaan);
}

UusiVelho::VarmistaSivu::VarmistaSivu() :
    ui( new Ui::Varmista)
{
    ui->setupUi(this);
    setTitle(tr("Huolehdi kirjanpitosi varmuuskopioinnista"));
}

UusiVelho::VastuuSivu::VastuuSivu() :
    ui( new Ui::Uusivastuu)
{
    ui->setupUi(this);
    setTitle(tr("Vastuu kirjanpidosta"));
}

UusiVelho::LoppuSivu::LoppuSivu() :
    ui( new Ui::UusiLoppu)
{
    ui->setupUi(this);
    setTitle(tr("Valmis"));
}

void UusiVelho::LoppuSivu::initializePage()
{
    ui->koneLabel->setText(tr("<p>Kirjanpitosi tallennetaan tietokoneellesi hakemistoon <tt>%1</tt> "
                              "tiedostoon <tt>%2</tt></p>"
                              "<p><b>Muista tiedostosi sijainti</b>, jotta pystyt avaamaan sen myöhemmin.</p>"
                              "<p><b>Huolehdi jatkuvasti tämän tiedoston varmuuskopioinnista!</b>")
                           .arg(field("sijainti").toString())
                           .arg(field("tiedosto").toString()));

    bool pilveen = field("pilveen").toBool();
    ui->pilviLabel->setVisible(pilveen);
    ui->koneLabel->setVisible(!pilveen);

}
