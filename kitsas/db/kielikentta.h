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
#ifndef KIELIKENTTA_H
#define KIELIKENTTA_H

#include <QVariant>
#include <QMap>
#include <QVariantMap>

#include "kieli/monikielinen.h"

class QListWidget;

class KieliKentta
{
public:
    KieliKentta();
    KieliKentta(const QVariant& var);
    KieliKentta(const QString& var);

    void aseta(const QVariant& var);
    void aseta(const QString& nimi, const QString& kieli);
    QString teksti(QString kieli = QString()) const;
    QString kaannos(const QString& kieli) const;   

    void tyhjenna();

    QVariantMap map() const;

protected:
    Monikielinen moni_;
};

#endif // KIELIKENTTA_H
