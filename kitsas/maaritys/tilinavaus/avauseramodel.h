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
#ifndef AVAUSERAMODEL_H
#define AVAUSERAMODEL_H

#include "avauserakantamodel.h"

class AvausEraModel : public AvausEraKantaModel
{
    Q_OBJECT

public:
    enum { KUMPPANI, NIMI, SALDO, POISTOAIKA };

    AvausEraModel(QObject *parent = nullptr, int poistoaika = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void lisaaRivi();

    void lataa(QList<AvausEra> erat = QList<AvausEra>()) override;

protected:
    int poistoAika_ = 0;

};

#endif // AVAUSERAMODEL_H
