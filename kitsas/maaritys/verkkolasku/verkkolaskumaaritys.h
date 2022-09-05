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
#ifndef VERKKOLASKUMAARITYS_H
#define VERKKOLASKUMAARITYS_H

#include "../maarityswidget.h"
#include <QVariant>

namespace Ui {
    class Finvoicevalinnat;
}


class VerkkolaskuMaaritys : public MaaritysWidget
{
    Q_OBJECT
public:

    enum {
        EIKAYTOSSA = 0,
        PAIKALLINEN = 1,
        MAVENTA = 2
    };

    VerkkolaskuMaaritys();
    ~VerkkolaskuMaaritys() override;

    void paivitaMaventa();
    bool tallenna() override;
    bool nollaa() override;
    bool onkoMuokattu() override;

protected:
    void valitseKansio();
    void valintaMuuttui();
    void maaritaMaventa();
    void maventaTiedot(QVariant* data);
    void eiTietoja();
    void setFlow(bool on);
    void noudaNyt();
    void velho();

private:
    Ui::Finvoicevalinnat *ui;
    QVariantMap maventaInfo_;
};

#endif // VERKKOLASKUMAARITYS_H
