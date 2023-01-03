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
#ifndef KPKYSELY_H
#define KPKYSELY_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QUrlQuery>


class YhteysModel;

/**
 * @brief Taustajärjestelmälle tehtävät kyselyt
 */
class KpKysely : public QObject
{
    Q_OBJECT
public:

    enum Metodi {
        GET,
        POST,
        PATCH,
        PUT,
        DELETE
    };

    enum Tila {
        ALUSTUS,
        OK,
        VIRHE
    };

    KpKysely(YhteysModel *parent, Metodi metodi = GET, QString polku = QString());

    void asetaUrl(const QString& kysely);

    QString polku() const { return polku_;}

    void lisaaAttribuutti(const QString& avain, const QString& arvo = QString());
    void lisaaAttribuutti(const QString& avain, const QDate& paiva);
    void lisaaAttribuutti(const QString& avain, int arvo);

    QString attribuutti(const QString& avain) const;
    Metodi metodi() const { return metodi_;}
    Tila tila() const { return tila_;}
    QUrlQuery urlKysely() const { return kysely_;}

    static QString tiedostotyyppi(const QByteArray& ba);

signals:
    /**
     * @brief Vastaus kyselyyn
     * @param reply Vastauksen runko
     */
    void vastaus(QVariant* reply);
    /**
     * @brief POST-kyselyn vastaus, kun kysely onnistuu
     * @param reply Vastauksen runko
     * @param lisattyId Vastauksen Location-headerin lopussa oleva numero
     */
    void lisaysVastaus(const QVariant& reply, int lisattyId);
    void virhe(int virhe, const QString& selitys = QString(), QVariant vastaus = QVariant());

public slots:
    virtual void kysy(const QVariant& data = QVariant()) = 0;
    virtual void lahetaTiedosto(const QByteArray& ba, const QMap<QString,QString> &meta = QMap<QString,QString>()) = 0;


protected:
    Metodi metodi_;
    QString polku_;
    QUrlQuery kysely_;
    QVariant vastaus_;
    Tila tila_;

};

#endif // KPKYSELY_H
