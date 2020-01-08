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
#ifndef SIIRTOAPURI_H
#define SIIRTOAPURI_H

#include "apuriwidget.h"

#include <QVariantMap>

namespace Ui {
class SiirtoApuri;
}

class SiirtoApuri : public ApuriWidget
{
    Q_OBJECT

public:
    explicit SiirtoApuri(QWidget *parent = nullptr, Tosite* tosite = nullptr);
    virtual ~SiirtoApuri() override;


    void otaFokus() override;

private slots:
    void tililtaMuuttui();
    void tililleMuuttui();
    void eraValittu(bool debet, int eraId, double avoinna, const QString& selite);

protected:
    bool teeTositteelle() override;
    void teeReset() override;
    void paivitaKateislaji();
    void haeAlkuperaistosite(bool debet, int eraId);
    void tositeSaapuu(bool debet, QVariant* data);

    void erikoisviennit(const QVariantList lista, double eurot, QVariantList& viennit);

private:
    Ui::SiirtoApuri *ui;
    QVariantList kreditAlkuperaiset_;
    QVariantList debetAlkuperaiset_;


};

#endif // SIIRTOAPURI_H
