/*
   Copyright (C) 2018 Arto Hyvättinen

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

#ifndef CSVTUONTI_H
#define CSVTUONTI_H

#include <QStringList>

#include "ui_csvtuontidlg.h"

/**
 * @brief csv-tiedoston tuominen
 */

namespace Tuonti {



class CsvTuonti : public QDialog
{
    Q_OBJECT
public:
    enum Sarakemuoto
    {
        TYHJA = 0,
        TEKSTI = 10,
        LUKUTEKSTI = 11,
        LUKU = 12,
        RAHA = 20,
        TILI = 21,
        VIITE = 22,
        ALLESATA = 23,
        SUOMIPVM = 30,
        ISOPVM = 31,
        USPVM = 32

    };

    enum Tuominen
    {
        EITUODA = 0,
        PAIVAMAARA,
        TOSITETUNNUS,
        TILINUMERO,
        DEBETEURO,
        KREDITEURO,
        RAHAMAARA,
        SELITE,
        IBAN,
        VIITENRO,
        ARKISTOTUNNUS,
        KOHDENNUS,
        TILINIMI,
        BRUTTOALVP,
        ALVPROSENTTI,
        ALVKOODI,
        SAAJAMAKSAJA,
        KTOKOODI,
        DEBETTILI,
        KREDITTILI,
        RAHASENTIT
    };

    enum { TyyppiRooli = Qt::UserRole + 1};

    CsvTuonti();
    ~CsvTuonti();

    static QVariantMap tuo(const QByteArray& data);

    /**
     * @brief Haistelee koodauksen ja palauttaa unicodea
     * @param data Raakadata
     * @return
     */
    static QString haistettuKoodattu(const QByteArray& data);

    /**
     * @brief Päättelee yhden rivin pohjalta erotinmerkin
     * @param data Csv-tietoa
     * @return Vaihtoehtoina , ; TAB
     */
    static QChar haistaErotin(const QString& data);

    /**
     * @brief Sijoittaa csv:n listamuotoon
     * @param data
     * @return
     */
    static QList<QStringList> csvListana(const QByteArray& data);

    static QString tyyppiTeksti(int muoto);
    static QString tuontiTeksti(int tuominen);

    /**
     * @brief Päättelee, onko data csv-tietoa
     * @param data
     * @return
     */
    static bool onkoCsv(const QByteArray& data);

public slots:
    void paivitaOletukset();
    void tarkistaTiliValittu();

protected:
    QVariantMap tuonti(const QByteArray& data);

    QVariantMap kirjaukset();
    QVariantMap tiliote();

    int tuoListaan(const QByteArray& data);

    QList<QStringList> csv_;
    QVector<Sarakemuoto> muodot_;

    Ui::CsvTuonti *ui;
};

}

#endif // CSVTUONTI_H
