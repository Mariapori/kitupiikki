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
#ifndef ENNAKKOHYVITYSDIALOGI_H
#define ENNAKKOHYVITYSDIALOGI_H

#include <QDialog>
#include <QItemSelection>
#include "model/euro.h"

namespace Ui {
class EnnakkoHyvitysDialogi;
}

class EnnakkoHyvitysModel;
class LaskuDialogi;

class EnnakkoHyvitysDialogi : public QDialog
{
    Q_OBJECT

public:
    explicit EnnakkoHyvitysDialogi(EnnakkoHyvitysModel *model, QWidget* parent = nullptr);
    ~EnnakkoHyvitysDialogi() override;

    int eraId() const;
    Euro euro() const;

private slots:
    void riviValittu(const QItemSelection& valinta);

private:
    Ui::EnnakkoHyvitysDialogi *ui;    
};

#endif // ENNAKKOHYVITYSDIALOGI_H
