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
#ifndef MYYNTILASKUTROUTE_H
#define MYYNTILASKUTROUTE_H

#include "../sqliteroute.h"

class MyyntilaskutRoute : public SQLiteRoute
{
public:
    MyyntilaskutRoute(SQLiteModel *model);
    QVariant get(const QString &polku, const QUrlQuery &urlquery = QUrlQuery()) override;

protected:
    QString sqlKysymys(const QUrlQuery& urlquery, const QString& ehdot, bool hyvitys = false) const;
};

#endif // MYYNTILASKUTROUTE_H
