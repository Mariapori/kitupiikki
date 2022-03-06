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
#ifndef ERAROUTE_H
#define ERAROUTE_H

#include "../sqliteroute.h"
#include "model/euro.h"
class Tili;


class EraRoute : public SQLiteRoute
{
public:
    EraRoute(SQLiteModel *model);
    QVariant get(const QString &polku, const QUrlQuery &urlquery = QUrlQuery()) override;

protected:
    QVariant erittely(const QDate& mista, const QDate& pvm);

    QVariant taysiErittely(Tili* tili, const QDate& mista, const QDate& mihin, const Euro& alkusaldo, const Euro& loppusaldo);
    QVariant listaErittely(Tili* tili, const QDate& mista, const QDate& mihin, const Euro& alkusaldo, const Euro& loppusaldo);
    QVariant muutosErittely(Tili* tili, const QDate& mista, const QDate& mihin, const Euro& alkusaldo, const Euro& loppusaldo);
};

#endif // ERAROUTE_H
