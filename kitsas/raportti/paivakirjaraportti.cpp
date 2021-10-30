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

#include <QDateEdit>
#include "paivakirjaraportti.h"
#include "ui_paivakirja.h"
#include "db/kirjanpito.h"
#include "raporttivalinnat.h"

PaivakirjaRaportti::PaivakirjaRaportti()
    : PaakirjaPaivakirjaKantaRaporttiWidget()
{    

    ui->tiliBox->hide();
    ui->tiliCombo->hide();
    ui->laatuLabel->hide();
    ui->laatuSlider->hide();
}

PaivakirjaRaportti::~PaivakirjaRaportti()
{

}

void PaivakirjaRaportti::tallennaValinnat()
{
    kp()->raporttiValinnat()->aseta(RaporttiValinnat::Tyyppi, "paivakirja");
}

