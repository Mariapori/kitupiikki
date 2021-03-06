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
#ifndef LASKUTAULUTILIOTEPROXYLLA_H
#define LASKUTAULUTILIOTEPROXYLLA_H

#include "model/laskutaulumodel.h"
class TilioteModel;

class LaskuTauluTilioteProxylla : public LaskuTauluModel
{
    Q_OBJECT
public:
    LaskuTauluTilioteProxylla(QObject *parent, TilioteModel *tiliote);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void paivitaSuoritukset();

protected:
    void tietoSaapuu(QVariant* var) override;

    TilioteModel* tiliote_;
    QMap<int,double> suoritukset_;
};

#endif // LASKUTAULUTILIOTEPROXYLLA_H
