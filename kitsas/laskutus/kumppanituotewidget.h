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
#ifndef KUMPPANITUOTEWIDGET_H
#define KUMPPANITUOTEWIDGET_H

#include <QWidget>

namespace Ui {
class KumppaniTuoteWidget;
}

class AsiakkaatModel;
class TuoteModel;
class QSortFilterProxyModel;
class VakioViiteModel;
class HuoneistoModel;

class KumppaniTuoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KumppaniTuoteWidget(QWidget *parent = nullptr);
    ~KumppaniTuoteWidget();

    enum { REKISTERI, ASIAKKAAT, TOIMITTAJAT, TUOTTEET, VAKIOVIITTEET, HUONEISTOT, RYHMAT };

public slots:
    void nayta(int valilehti);
    void suodata(const QString& suodatus);
    void suodataRyhma(int ryhma);

private slots:
    void ilmoitaValinta();
    void uusi();
    void muokkaa();
    void poista();
    void paivita();
    void tuo();
    void vie();
    void raportti();
    void yhdista();
    void laskuta();

signals:
    void kumppaniValittu(const QString& nimi, int id);
    void viiteValittu(const QString& viite);
    void ryhmaValittu(int ryhma);

private:
    Ui::KumppaniTuoteWidget *ui;

    QSortFilterProxyModel *proxy_;
    AsiakkaatModel* asiakkaat_;
    VakioViiteModel* vakioviitteet_;
    HuoneistoModel* huoneistot_;

    int valilehti_;
    int ryhma_ = 0;
};

#endif // KUMPPANITUOTEWIDGET_H
