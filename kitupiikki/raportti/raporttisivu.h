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

#ifndef RAPORTTISIVU_H
#define RAPORTTISIVU_H

#include <QWidget>
#include <QPrinter>

#include "ui_raportti.h"
#include "raportti.h"

class RaporttiSivu : public QWidget
{
    Q_OBJECT
public:
    enum { RAPORTTIID = Qt::UserRole } ;

    explicit RaporttiSivu(QWidget *parent = 0);

signals:

public slots:
    void raporttiValittu(QListWidgetItem *item);
    void tulosta();
    void esikatsele();
    void tyhjenna();

protected:
    void lisaaRaportti(Raportti *raportti);

protected:
    Ui::RaporttiWg *ui;
    QList<Raportti*> raportit;
    Raportti *nykyraportti;
    QPrinter printer;
};

#endif // RAPORTTISIVU_H
