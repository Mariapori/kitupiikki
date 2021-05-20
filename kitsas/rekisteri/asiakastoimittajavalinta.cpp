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
#include "asiakastoimittajavalinta.h"
#include "asiakastoimittajalistamodel.h"

#include "asiakastoimittajadlg.h"

#include "validator/ytunnusvalidator.h"
#include "db/kirjanpito.h"

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCompleter>
#include <QComboBox>
#include <QCompleter>

#include <QDebug>
#include <QKeyEvent>
#include <QFocusEvent>

AsiakasToimittajaValinta::AsiakasToimittajaValinta(QWidget *parent) :
    QWidget(parent),
    combo_( new QComboBox(this)),
    button_(new QPushButton(this)),
    model_( AsiakasToimittajaListaModel::instanssi()),
    dlg_( new AsiakasToimittajaDlg(this))

{
    button_->setIcon(QIcon(":/pic/uusiasiakas.png"));

    QHBoxLayout* leiska = new QHBoxLayout(this);
    leiska->setContentsMargins(0,0,0,0);
    leiska->addWidget( combo_ );
    leiska->addWidget(button_);
    leiska->setStretchFactor(combo_, 9);
    setLayout(leiska);

    combo_->setEditable(true);
    combo_->setModel( model_ );
    combo_->completer()->setCompletionMode(QCompleter::PopupCompletion);

    connect( button_, &QPushButton::clicked, this, &AsiakasToimittajaValinta::muokkaa);
    connect( model_, &AsiakasToimittajaListaModel::modelReset, this, &AsiakasToimittajaValinta::modelLadattu);


    connect(combo_, QOverload<int>::of(&QComboBox::activated), this, &AsiakasToimittajaValinta::valitseAsiakas);
    connect( combo_->lineEdit(), &QLineEdit::editingFinished, this, &AsiakasToimittajaValinta::syotettyNimi);

    connect( combo_, &QComboBox::currentTextChanged, this, &AsiakasToimittajaValinta::nimiMuuttui );

    connect( dlg_, &AsiakasToimittajaDlg::tallennettu, this, &AsiakasToimittajaValinta::talletettu);

    combo_->lineEdit()->setPlaceholderText(tr("Y-tunnus tai nimi"));
    combo_->installEventFilter(this);

    setFocusProxy( combo_ );
    button_->setFocusPolicy(Qt::NoFocus);
    clear();
}

QString AsiakasToimittajaValinta::nimi() const
{
    return combo_->currentText();
}

QVariantMap AsiakasToimittajaValinta::map() const
{
    QVariantMap vmap;
    vmap.insert("id", id());
    vmap.insert("nimi", nimi());
    return vmap;
}

void AsiakasToimittajaValinta::set(int id, const QString &nimi)
{
    if( id == id_)
        return;

    ibanit_.clear();

    if( combo_ ) {
       combo_->setCurrentText(nimi);
    }

    if( id ) {
        ladattu_ = id;
        model_->lataa();
    }

}


void AsiakasToimittajaValinta::clear()
{
    id_ = 0;
    ladattu_ = 0;
    combo_->setCurrentIndex(-1);
    combo_->setCurrentText(QString());

}


void AsiakasToimittajaValinta::tuonti(const QVariantMap &data)
{
    QString alvtunnari = data.contains("alvtunnus") ? data.value("alvtunnus").toString() :
            "FI" + data.value("kumppaniytunnus").toString();
    alvtunnari.remove('-');

    if( model_->idAlvTunnuksella( alvtunnari) ) {
        // Valitaan Y-tunnuksella
        combo_->setCurrentIndex( combo_->findData( model_->idAlvTunnuksella(alvtunnari) ) );
        emit valittu( combo_->currentData(AsiakasToimittajaListaModel::IdRooli).toInt() );
    } else if( combo_->findText(
                   data.contains("kumppaninimi") ?
                   data.value("kumppaninimi").toString()
                   : data.value("nimi").toString()) > -1) {
        // Valitaan nimellä
        combo_->setCurrentIndex( combo_->findText( data.value("kumppaninimi").toString() ) );
        emit valittu( combo_->currentData(AsiakasToimittajaListaModel::IdRooli).toInt() );
    } else {
        // Siirrytään dialogiin                
        // Pitäisikö yrittää vielä tilinumerolla ?
        if( data.value("iban").toStringList().isEmpty()) {
            ibanLoytyi(data, nullptr);
        } else {
            QString ekaIban = data.value("iban").toStringList().value(0);
            KpKysely* ibankysely = kpk("/kumppanit");
            ibankysely->lisaaAttribuutti("iban", ekaIban);
            connect(ibankysely, &KpKysely::vastaus, [this,data] (QVariant* vastaus) {this->ibanLoytyi(data, vastaus);} );
            connect(ibankysely, &KpKysely::virhe, [this, data] {this->ibanLoytyi(data, nullptr);});
            ibankysely->kysy();
        }
    }
    ibanit_ = data.value("iban").toStringList();
}

void AsiakasToimittajaValinta::valitseAsiakas()
{
    setId( combo_->currentData().toInt() );
}

void AsiakasToimittajaValinta::nimiMuuttui()
{
    int id = combo_->currentData(AsiakasToimittajaListaModel::IdRooli).toInt();
    setId( id );

    // Jos on syötetty y-tunnus, haetaan sillä
    if( YTunnusValidator::kelpaako( combo_->currentText() )) {
        QString alvtunnari = "FI" + combo_->currentText();
        int hakuId = model_->idAlvTunnuksella( alvtunnari.remove('-'));
        if( hakuId )
            combo_->setCurrentIndex( combo_->findData(hakuId) );
        else
            dlg_->ytunnuksella( combo_->currentText());
    } else
        emit muuttui( combo_->currentText());
}

void AsiakasToimittajaValinta::syotettyNimi()
{
    if( combo_->currentText().isEmpty())
        clear();

    if( !id_ && !combo_->currentText().isEmpty() && !YTunnusValidator::kelpaako( combo_->currentText() ) ) {
        if(property("MuokkaaUusi").toBool() )
            muokkaa();
    }
}

void AsiakasToimittajaValinta::muokkaa()
{

    if( id_ )
        dlg_->muokkaa(id_);
    else
        dlg_->uusi( combo_->currentText());

}

void AsiakasToimittajaValinta::talletettu(int id, const QString& /*nimi*/)
{
    ladattu_ = id;
    model_->lataa();
    emit valittu(id);

}

void AsiakasToimittajaValinta::modelLadattu()
{
    if( ladattu_ )
    {
        setId(ladattu_);
        int indeksi = combo_->findData( id_ );
        combo_->setCurrentIndex(indeksi);
    }
}

void AsiakasToimittajaValinta::ibanLoytyi(const QVariantMap &tuontiData, QVariant *data)
{
    QVariantMap map = data ? data->toMap() : QVariantMap();
    if(map.isEmpty()) {
        dlg_->tuonti(tuontiData);
    } else {
        set(map.value("id").toInt(), map.value("nimi").toString());
    }
    ibanit_ = tuontiData.value("iban").toStringList();
}


void AsiakasToimittajaValinta::setId(int id)
{    
    id_ = id;
    ibanit_.clear();

    if( id ) {
        button_->setIcon(QIcon(":/pic/muokkaaasiakas.png"));
        emit valittu(id);
    } else
        button_->setIcon(QIcon(":/pic/uusiasiakas.png"));
}

bool AsiakasToimittajaValinta::eventFilter(QObject *watched, QEvent *event)
{
    if( watched == combo_ && event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( keyEvent->key() == Qt::Key_Enter ||
                keyEvent->key() == Qt::Key_Return) {
            muokkaa();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}


