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
#ifndef POPPLERANALYZERDOCUMENT_H
#define POPPLERANALYZERDOCUMENT_H

#include "pdftoolkit.h"
#include <QPdfDocument>


class PopplerAnalyzerDocument : public PdfAnalyzerDocument
{
    Q_OBJECT

    class Palanen {
    public:
        explicit Palanen();
        int vasen, oikea, yla, ala;
        QString teksti;
        Palanen* seuraava = nullptr;
    };


public:    
    PopplerAnalyzerDocument(const QByteArray& data);
    ~PopplerAnalyzerDocument();

    virtual int pageCount() override;
    virtual PdfAnalyzerPage page(int page) override;
    virtual QList<PdfAnalyzerPage> allPages() override;
    virtual QString title() const override;


protected:
    void status(QPdfDocument::Status s);

private:
    QPdfDocument doc_;


};

#endif // POPPLERANALYZERDOCUMENT_H
