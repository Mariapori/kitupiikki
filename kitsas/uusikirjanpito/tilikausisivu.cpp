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
#include "ui_tilikausi.h"
#include "uusivelho.h"
#include "tilikausisivu.h"
#include <QDate>

TilikausiSivu::TilikausiSivu(UusiVelho* wizard) :
    ui( new Ui::TilikausiSivu),
    velho( wizard)
{
    setTitle(tr("Tilikauden tiedot"));

    ui->setupUi(this);
    ui->eiavaustaVaroitus->setVisible(velho->tilikaudet_.count() == 1);

    int tamavuosi = QDate::currentDate().year();

    connect( ui->ekaAlkaa, SIGNAL(dateChanged(QDate)),
             this, SLOT(alkuPaivaMuuttui(QDate)));

    connect( ui->ekaPaattyy, &QDateEdit::dateChanged, this, &TilikausiSivu::loppuPaivaMuuttui);
    connect( ui->edellinenAlkoi, &QDateEdit::dateChanged, this, &TilikausiSivu::loppuPaivaMuuttui);

    ui->ekaAlkaa->setDate(QDate(tamavuosi,1,1));
    ui->ekaPaattyy->setDate(QDate(tamavuosi,12,31));

    ui->edellinenAlkoi->setDate(QDate(tamavuosi-1,1,1));
        ui->edellinenPaattyi->setDate(QDate(tamavuosi-1,12,31));

}

TilikausiSivu::~TilikausiSivu()
{
    delete ui;
}

bool TilikausiSivu::isComplete() const
{
    return( ui->ekaAlkaa->date() < ui->ekaPaattyy->date() && ui->ekaPaattyy->date() <= ui->ekaAlkaa->date().addMonths(18) &&
            ui->edellinenAlkoi->date() <= ui->edellinenPaattyi->date() && ui->edellinenPaattyi->date() <= ui->edellinenAlkoi->date().addMonths(18));
}

bool TilikausiSivu::validatePage()
{
    velho->tilikaudet_.clear();

    if( !ui->aloittavaTilikausiCheck->isChecked())
    {
        QVariantMap vanha;
        vanha.insert("alkaa", ui->edellinenAlkoi->date().toString(Qt::ISODate));
        vanha.insert("loppuu", ui->edellinenPaattyi->date().toString(Qt::ISODate));
        velho->tilikaudet_.append(vanha);
        velho->asetukset_.insert("Tilinavaus",2);
        velho->asetukset_.insert("TilinavausPvm", ui->edellinenPaattyi->date());
    } else {
        velho->asetukset_.remove("Tilinavaus");
        velho->asetukset_.remove("TilinavausPvm");
    }

    QVariantMap uusi;
    uusi.insert("alkaa", ui->ekaAlkaa->date().toString(Qt::ISODate));
    uusi.insert("loppuu", ui->ekaPaattyy->date().toString(Qt::ISODate));
    velho->tilikaudet_.append(uusi);

    velho->asetukset_.insert("TilitPaatetty",
                             ui->ekaAlkaa->date().addDays(-1).toString(Qt::ISODate));
    velho->asetukset_.insert("AlvAlkaa", ui->ekaAlkaa->date());

    return true;
}

void TilikausiSivu::alkuPaivaMuuttui(const QDate &date)
{
    ui->ekaPaattyy->setDate( date.addYears(1).addDays(-1)  );

    ui->edellinenPaattyi->setDate( date.addDays(-1));
    ui->edellinenAlkoi->setDate( date.addYears(-1));

    loppuPaivaMuuttui();
}

void TilikausiSivu::loppuPaivaMuuttui()
{

    if( ui->ekaAlkaa->date() < ui->ekaPaattyy->date() && ui->ekaPaattyy->date() <= ui->ekaAlkaa->date().addMonths(18)  )
        ui->ekaPaattyy->setStyleSheet("color: black;");
    else
        ui->ekaPaattyy->setStyleSheet("color: red;");

    if( ui->edellinenAlkoi->date() <= ui->edellinenPaattyi->date() && ui->edellinenPaattyi->date() <= ui->edellinenAlkoi->date().addMonths(18) )
        ui->edellinenAlkoi->setStyleSheet("color: black;");
    else
        ui->edellinenAlkoi->setStyleSheet("color: red");


    emit completeChanged();
}
