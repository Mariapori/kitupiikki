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

#ifndef TILIKARTTARAPORTTI_H
#define TILIKARTTARAPORTTI_H

#include "ui_tilikarttaraportti.h"

#include "raporttiwidget.h"
#include "raportinkirjoittaja.h"

#include "db/kirjanpito.h"

/**
 * @brief Tilikartan tulostava raportti
 */
class TilikarttaRaportti : public RaporttiWidget
{
    Q_OBJECT
public:

    TilikarttaRaportti();
    ~TilikarttaRaportti() override;

protected:
    void tallenna() override;

protected slots:
    /**
     * @brief Kun tilikausi vaihtuu, päivitetään saldopäivä sille
     */
    void paivitaPaiva();


protected:
    Ui::TilikarttaRaportti *ui;
};

#endif // TILIKARTTARAPORTTI_H
