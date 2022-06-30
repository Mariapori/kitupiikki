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

#ifndef MAARITYSWIDGET_H
#define MAARITYSWIDGET_H

#include <QWidget>

/**
 * @brief Määrityssivujen widgetin kantaluokka
 *
 * Määrityssivu luodaan vasta, kun se näytetään.
 * Sivu alustetaan nollaa()-funktiota kutsuttaessa ja tallennetaan
 * tallenna()-kutsusta
 *
 * onkomuokattu() palauttaa, onko tässä widgetissa tallentamattomia muutoksia.
 * Jos tallentamattomalta sivulta poistutaan, kysytään ensin varmennus
 *
 */
class MaaritysWidget : public QWidget
{
    Q_OBJECT
public:
    MaaritysWidget(QWidget *parent = nullptr);
    ~MaaritysWidget();

signals:
    /**
     * @brief Ilmoittaa, onko Tallennna-nappi käytössä
     * @param onko
     */
    void tallennaKaytossa(bool onko);

public:
    /**
     * @brief Lataa näytettävälle sivulle tallennetut tiedot
     * @return onnistuiko
     */
    virtual bool nollaa() {return false;}
    /**
     * @brief Tallettaa tehdyt asetukset
     * @return onnistuiko
     */
    virtual bool tallenna() {return false;}
    /**
     * @brief Onko sivulla tallentamattomia muokkauksia
     * @return tosi jos tallentamttomia muutoksia
     */
    virtual bool onkoMuokattu() { return false; }

    /**
     * @brief Näytetäänkö tällä sivulla Vie tilikartta -nappi
     * @return
     */
    virtual bool naytetaankoVienti() { return false;}

    /**
     * @brief Näytetäänkö tällä sivulla Tallennus- ja Peru-napit
     * @return
     */
    virtual bool naytetaankoTallennus() { return true; }

    /**
     * @brief Näytettävän ohjesivun nimi
     * @return
     */
    virtual QString ohjesivu() { return ""; }

protected:
    void connectMuutokset();
    void tarkastaMuokkaus();
};

#endif // MAARITYSWIDGET_H
