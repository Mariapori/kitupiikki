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
#ifndef AVAUSERADLG_H
#define AVAUSERADLG_H

#include <QDialog>

#include "avauserakantamodel.h"
#include "tilinavausmodel.h"

namespace Ui {
class AvausEraDlg;
}

class AvausEraDlg : public QDialog
{
    Q_OBJECT

public:
    enum AvausDlgLaji {TASE_ERAT, KOHDENUKSET, KUUKAUDET};

    AvausEraDlg(TilinavausModel *avaus, int tilinumero, QWidget* parent = nullptr);
/*
    explicit AvausEraDlg(int tili, bool kohdennukset = false,
                         QList<AvausEra> erat = QList<AvausEra>(), QWidget *parent = nullptr);
*/
    ~AvausEraDlg();

    QList<AvausEra> erat() const;

    void accept() override;

protected slots:
    void paivitaSumma();
    void lisaaTarvittaessa();

private:
    Ui::AvausEraDlg *ui;
    AvausEraKantaModel *model_;
    TilinavausModel *avaus_;
    Tili tili_;
};

#endif // AVAUSERADLG_H
