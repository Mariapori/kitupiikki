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
#ifndef PDFANALYZERPAGE_H
#define PDFANALYZERPAGE_H

#include <QRectF>

class PdfAnalyzerText;

class PdfAnalyzerPage
{
public:
    PdfAnalyzerPage(QSizeF size = QSizeF());
    ~PdfAnalyzerPage();

    PdfAnalyzerText* firstText() const;
    QSizeF size() const { return size_;}

    void addText(const QRectF& boundingRect,
                 const QString& text);
    void setSize(const QSizeF size);

    static inline double mmToPoints(double mm);
    static inline double pointsTomm(double points);

private:
    PdfAnalyzerText* first_ = nullptr;
    PdfAnalyzerText* last_ = nullptr;
    QSizeF size_;
};

#endif // PDFANALYZERPAGE_H
