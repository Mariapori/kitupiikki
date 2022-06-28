/*
   Copyright (C) 2017 Arto Hyvättinen

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

#ifndef TUOTEMODEL_H
#define TUOTEMODEL_H

#include "tuote.h"
#include <QAbstractTableModel>

/**
 * @brief Laskutuksessa käytettävät tuotteet
 * 
 * Laskutusdialogissa on mahdollisuus valita valmiita rivejä tallennettavaksi "tuotteiksi",
 * joita on helppo uudelleenkäyttää myöhemmin
 *
 * TuoteModel tallettaa muutokset välittömästi myös tietokantaan
 * 
 */
class TuoteModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TuoteModel(QObject *parent = nullptr);

    enum TuoteSarake { NIMIKE, NETTO, BRUTTO };
    enum { IdRooli = Qt::UserRole , MapRooli = Qt::UserRole + 2};
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    QString nimike(int id) const;
    QByteArray csv() const;
    Tuote tuote(int id) const;

public slots:
    void lataa();
    void paivitaTuote(Tuote tuote);
    void poistaTuote(int id);

private slots:
    void dataSaapuu(QVariant* data);
    void muokattu(QVariant* data);
    
private:
    QList<Tuote> tuotteet_;
    int desimaalit_ = 2;
    
};

#endif // TUOTEMODEL_H
