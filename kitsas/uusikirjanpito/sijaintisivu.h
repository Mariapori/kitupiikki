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

#ifndef SIJAINTISIVU_H
#define SIJAINTISIVU_H

#include <QWizardPage>

#include "ui_uusisijainti.h"

/**
 * @brief Uuden kirjanpidon velhon sivu, jossa kysytään tiedoston sijainti
 */
class SijaintiSivu : public QWizardPage
{
    Q_OBJECT

protected:
    Ui::UusiSijainti *ui;
public:
    SijaintiSivu();
    ~SijaintiSivu() override;
    void cleanupPage() override {;}

protected slots:
    void vaihdaSijainti();

protected:
    void initializePage() override;
    QString estaTuplaTiedosto(QString tiedosto);
    bool validatePage() override;
};

#endif // SIJAINTISIVU_H
