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

#ifndef PDFIKKUNA_H
#define PDFIKKUNA_H

#include <QMainWindow>

class QGraphicsScene;
class QGraphicsView;

/**
 * @brief Ikkuna pdf-tiedostojen näyttämiseen
 */
class PdfIkkuna : public QMainWindow
{
    Q_OBJECT
public:
    PdfIkkuna(const QByteArray &pdfdata, QWidget *parent = nullptr);
    ~PdfIkkuna();

signals:

public slots:
    void tulosta();
    void avaaOhjelmalla();

public:
    /**
     * @brief Näyttää pdf-tiedoston
     * @param pdfdata
     */
    static void naytaPdf(const QByteArray &pdfdata);

    static void naytaLiite(const int tositeId, const int liiteId=1);

    static void naytaPdf(const QString& tiedostonnimi);

protected:
    void resizeEvent(QResizeEvent *event);

    QGraphicsScene *scene;
    QGraphicsView *view;

    QByteArray data;

    double leveys = 0.0;
};

#endif // PDFIKKUNA_H