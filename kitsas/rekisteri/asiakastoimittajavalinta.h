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
#ifndef ASIAKASTOIMITTAJAVALINTA_H
#define ASIAKASTOIMITTAJAVALINTA_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class AsiakasToimittajaListaModel;

class QComboBox;

class AsiakasToimittajaDlg;

class AsiakasToimittajaValinta : public QWidget
{
    Q_OBJECT
public:
    AsiakasToimittajaValinta(QWidget *parent = nullptr);

    int id() const { return id_;}
    QString nimi() const;
    QStringList ibanit() const { return ibanit_;}
    QVariantMap map() const;

signals:
    void valittu(int id);
    void muuttui(const QString& nimi);

public slots:
    void set(int id, const QString& nimi = QString());
    void clear();
    void tuonti(const QVariantMap &data);


private slots:
    void valitseAsiakas();
    void nimiMuuttui();
    void syotettyNimi();
    void muokkaa();
    void talletettu(int id, const QString &nimi);
    void modelLadattu();

    void ibanLoytyi(const QVariantMap& tuontiData, QVariant* data);

protected:
    void setId(int id);
    bool eventFilter(QObject *watched, QEvent *event) override;

    QComboBox* combo_;
    QPushButton* button_;

    AsiakasToimittajaListaModel* model_;

    AsiakasToimittajaDlg *dlg_ = nullptr;

    int id_=0;
    int ladattu_=0;
    QStringList ibanit_;
    QString nimi_;

    bool inSet_ = false;
};

#endif // ASIAKASTOIMITTAJAVALINTA_H
