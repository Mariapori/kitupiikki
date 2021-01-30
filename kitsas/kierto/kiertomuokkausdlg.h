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
#ifndef KIERTOMUOKKAUSDLG_H
#define KIERTOMUOKKAUSDLG_H

#include <QDialog>

namespace Ui {
class KiertoMuokkausDlg;
}

class KiertoMuokkausModel;
class QSortFilterProxyModel;

class KiertoMuokkausDlg : public QDialog
{
    Q_OBJECT

public:
    explicit KiertoMuokkausDlg(int id = 0, QWidget *parent = nullptr, bool portaali = true);
    ~KiertoMuokkausDlg() override;

    void accept() override;

private:
    void tyyppiMuuttuu();
    void tallennettu();
    void alusta();
    void kayttajatSaapuu(QVariant* data);
    void paivitaRooliValinta();
    void lisaaRivi();
    void poistaRivi();
    void lataa(QVariant* data);
    QVariantMap data() const;

private:
    Ui::KiertoMuokkausDlg *ui;
    KiertoMuokkausModel *model;
    QSortFilterProxyModel *proxy;
    int kiertoId_ = 0;
};

#endif // KIERTOMUOKKAUSDLG_H
