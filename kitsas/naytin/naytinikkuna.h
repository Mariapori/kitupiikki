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
#ifndef NAYTINIKKUNA_H
#define NAYTINIKKUNA_H


#include "raportti/raportinkirjoittaja.h"
#include "raportti/raporttivalinnat.h"
#include <QMainWindow>
#include <QCloseEvent>

class NaytinView;
class QAction;



/**
 * @brief Ikkuna kaikenlaiseen näyttämiseen
 */
class NaytinIkkuna : public QMainWindow
{
    Q_OBJECT
public:
    NaytinIkkuna();
    ~NaytinIkkuna() override;

    NaytinView* view() { return view_;}

    static void naytaRaportti(RaportinKirjoittaja raportti);
    static void naytaRaportti(const RaporttiValinnat& valinnat);
    static void nayta(const QByteArray &data);
    static void nayta(const QString& teksti);
    static void naytaTiedosto(const QString& tiedostonnimi);
    static void naytaLiite(const int liiteId);
    static void naytaLiite(const int tositeId, const QString& rooli);
    static void naytaLasku(const int tositeId);

protected:        
    static void naytaLiite(const QString& hakulauseke);        

    void closeEvent(QCloseEvent* event) override;

private slots:
    void sisaltoMuuttui();
    void lataaLaskuTosite(int tositeId);
    void tositeLadattu();
    void tyhjaRaportti();

private:
    void teeToolbar();

private:
    NaytinView *view_;

    QAction *paivitaAktio_;
    QAction *avaaAktio_;
    QAction *raitaAktio_;
    QAction *sivunAsetusAktio_;

    QAction *csvAktio_;
    QAction *htmlAktio_;

    QAction* zoomAktio_;
    QAction* zoomInAktio_;
    QAction* zoomOutAktio_;



};

#endif // NAYTINIKKUNA_H
