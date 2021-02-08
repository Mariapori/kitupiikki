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

#ifndef TILINVALINTALINE_H
#define TILINVALINTALINE_H

#include <QLineEdit>
#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "kirjanpito.h"
#include "kirjaus/tilidelegaatti.h"


/**
 * @brief Kantaluokka tilien valinnan lineEditille
 */
class KantaTilinvalintaLine : public QLineEdit
{
    Q_OBJECT

public:
    KantaTilinvalintaLine( QWidget *parent=nullptr);

    int valittuTilinumero() const;
    Tili valittuTili() const;
    Tili* tili() const;


public slots:
    void valitseTiliNumerolla(int tilinumero);
    void valitseTili(const Tili &tili);
    void valitseTili(const Tili* tili);

    void suodataTyypilla(const QString& regexp);

protected:
    QSortFilterProxyModel *proxyTyyppi_;
    QSortFilterProxyModel *proxyTila_;

    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent *event) override;

};

/**
 * @brief QLineEditor, joka valitsee tilejä delegaatille
 *
 * Tämä säädetty delegaatin käyttöön: jos pitäisi mennä
 * valintaikkunaan, heittää fokuksen vanhemmalle
 *
 */
class TilinvalintaLineDelegaatille : public KantaTilinvalintaLine
{
    Q_OBJECT
public:
    TilinvalintaLineDelegaatille(QWidget *parent = nullptr);
    QString tilinimiAlkaa() const { return alku_; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;


protected:
    QString alku_;

};

/**
 * @brief Tilin valinta täydentyvällä editorilla, jossa pomppudialogi
 */
class TilinvalintaLine : public KantaTilinvalintaLine
{
    Q_OBJECT
public:
    TilinvalintaLine(QWidget *parent = nullptr);

public slots:
    void asetaModel( TiliModel *model);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;    
    TiliModel *model_;
};

#endif // TILINVALINTALINE_H
