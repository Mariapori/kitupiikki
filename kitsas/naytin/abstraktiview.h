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
#ifndef ABSTRAKTIVIEW_H
#define ABSTRAKTIVIEW_H

#include <QGraphicsView>
#include "naytin/naytinscene.h"

class QPrinter;

namespace Naytin {

class AbstraktiView : public QGraphicsView
{
    Q_OBJECT
public:
    AbstraktiView();
    virtual ~AbstraktiView() override;
    double zoomaus() const { return zoomaus_;}

    virtual QString tiedostonMuoto() const = 0;
    virtual QString tiedostonPaate() const = 0;
    virtual QString otsikko() const { return QString(); }

    virtual QByteArray data() const  = 0;

public slots:
    virtual void paivita() const = 0;
    virtual void tulosta(QPrinter* printer) const = 0;

    virtual void zoomIn();
    virtual void zoomOut();
    virtual void zoomFit();

    virtual void salliPudotus(bool sallittu = true);

signals:
    void tiedostoPudotettu(const QString polku);

protected:
    void resizeEvent(QResizeEvent *event) override;

protected:
    double zoomaus_;
    NaytinScene* skene_;

};


}


#endif // ABSTRAKTIVIEW_H
