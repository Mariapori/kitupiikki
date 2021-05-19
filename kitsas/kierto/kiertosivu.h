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
#ifndef KIERTOSIVU_H
#define KIERTOSIVU_H

#include "kitupiikkisivu.h"

class KiertoSelausModel;

namespace Ui {
    class KiertoSivu;
}

class KiertoSivu : public KitupiikkiSivu
{
    Q_OBJECT
public:
    enum { TYOLISTA, KAIKKI};

    KiertoSivu(QWidget *parent = nullptr);
    ~KiertoSivu() override;

    void siirrySivulle() override;

    QString ohjeSivunNimi() override { return "kierto";}
signals:
    void tositeValittu(int id);
private:
    void naytaTosite(const QModelIndex& index);
    void vaihdaTab(int tab);

private:
    Ui::KiertoSivu* ui;
    KiertoSelausModel *model;
};

#endif // KIERTOSIVU_H
