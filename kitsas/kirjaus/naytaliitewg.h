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

#ifndef NAYTALIITEWG_H
#define NAYTALIITEWG_H

#include <QStackedWidget>
#include "ui_tositewg.h"


class NaytinView;

/**
 * @brief Liitteen näyttäminen
 *
 * Tämä widget muodostaa KirjausSivun yläpuoliskon.
 *
 * naytaTiedosto-funktiolla (slot) näytetään haluttu tiedosto
 * Jos käyttäjä valitsee tai tiputtaa tiedoston, lähetetään lisaaLiite-signal
 *
 * Tällä hetkellä tukee pdf-tietostoja
 *
 */
class NaytaliiteWg : public QStackedWidget
{
    Q_OBJECT
public:
    NaytaliiteWg(QWidget *parent=nullptr);
    ~NaytaliiteWg();

    NaytinView *liiteView() { return view; }

public slots:
    void valitseTiedosto();
    void naytaPdf(const QByteArray& pdfdata, bool salliPudotus = true);
    void leikepoydalta();

    void naytaPohjat(bool nayta);
    void pohjatSaapui();
    void tarkistaLeikepoyta();

signals:
    void lisaaLiite(const QString& polku);
    void lisaaLiiteDatalla(const QByteArray& data, const QString& nimi);
    void lataaPohja(int tositeId);


protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

protected:
    Ui::TositeWg *ui;

    NaytinView *view;
    bool pohjatNakyvilla_ = true;

};

#endif // NAYTALIITEWG_H
