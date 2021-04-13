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
#ifndef OSTOLASKUTROUTE_H
#define OSTOLASKUTROUTE_H

#include "../sqliteroute.h"

class OstolaskutRoute : public SQLiteRoute
{
public:
    OstolaskutRoute(SQLiteModel *model);
    QVariant get(const QString &polku, const QUrlQuery &urlquery = QUrlQuery()) override;

private:
    QString sqlKysymys(const QUrlQuery &urlquery, bool hyvitys = false) const;
};

#endif // OSTOLASKUTROUTE_H
