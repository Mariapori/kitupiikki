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

#ifndef LIITEMODEL_H
#define LIITEMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QSqlDatabase>
#include <QBuffer>

/**
 * @brief Yhden liitteen tiedot. TositeModel käyttää.
 */
struct Liite
{
    int id = 0;
    int liiteno = 0;
    QString otsikko;
    QByteArray sha;

    QByteArray pdf;
    QByteArray thumbnail;
    bool muokattu = false;
};

class TositeModel;

/**
 * @brief Tositteen litteet esittävä model
 */
class LiiteModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum Roolit
    {
        IdRooli = Qt::UserRole + 1,
        OtsikkoRooli = Qt::UserRole + 2,
        Sharooli = Qt::UserRole + 5,
        TiedostoNimiRooli = Qt::UserRole + 6,
        PdfRooli = Qt::UserRole + 7,
        LiiteNumeroRooli = Qt::UserRole + 8
    };


    /**
     * @brief LiiteModel
     * @param tositemodel Tosite, jonka liitteitä käsitellään: jos 0, käsitellään liitteitä tositteella NULL
     * @param parent
     */
    LiiteModel(TositeModel *tositemodel, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    /**
     * @brief Lisää pdf:n
     * @param liite
     * @param otsikko
     * @return Liitteen nro
     */
    int lisaaLiite(const QByteArray &liite, const QString& otsikko);
    /**
     * @brief Jos samalla otsikolla olemassa, korvaa - muuten lisää
     * @param pdf
     * @param otsikko
     * @return
     */
    int asetaLiite(const QByteArray &liite, const QString& otsikko);

    int lisaaTiedosto(const QString& polku, const QString& otsikko);
    void poistaLiite(int indeksi);

    /**
     * @brief Palauttaa liitteen datan halutulla otsikolla
     * @param otsikko
     * @return
     */
    QByteArray liite(const QString& otsikko);

    bool muokattu() const { return muokattu_; }


public slots:
    void lataa();
    void tyhjaa();
    /**
     * @brief Tallentaa liitteet
     * @return tosi, jos onnistui
     */
    bool tallenna();

signals:
    void liiteMuutettu();


protected:
    int seuraavaNumero() const;

    TositeModel *tositeModel_;
    QList<Liite> liitteet_;
    QList<int> poistetutIdt_;
    bool muokattu_;
};

#endif // LIITEMODEL_H
