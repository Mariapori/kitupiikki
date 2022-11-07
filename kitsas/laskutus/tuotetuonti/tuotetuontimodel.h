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
#ifndef TUOTETUONTIMODEL_H
#define TUOTETUONTIMODEL_H


#include "rekisteri/rekisterituontimodel.h"

class TuoteTuontiModel : public RekisteriTuontiModel
{
    Q_OBJECT

public:
    
    enum Sarakkeet {
        EITUODA,
        NIMIKE,
        YKSIKKO,
        NETTOHINTA,
        BRUTTOHINTA,
        KOHDENNUS,
        TILI,
        ALVKOODI,
        ALVPROSENTTI,
        KOODI
    };
    
    enum {
        MALLI,
        MUOTO
    };
    
    explicit TuoteTuontiModel(QObject *parent = nullptr);
    
    static QString otsikkoTeksti(int sarake);
    
    QVariantList lista() const override;

protected:
    QString otsikkoTekstini(int sarake) const override;  
    void arvaaSarakkeet() override;
    
};

#endif // TUOTETUONTIMODEL_H
