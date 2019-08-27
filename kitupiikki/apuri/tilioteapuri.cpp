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
#include "tilioteapuri.h"
#include "ui_tilioteapuri.h"
#include "tiliotemodel.h"

#include "tiliotekirjaaja.h"
#include "model/tosite.h"
#include "model/tositeviennit.h"

#include <QDate>
#include <QSortFilterProxyModel>

#include "kirjaus/tilidelegaatti.h"
#include "kirjaus/eurodelegaatti.h"
#include "kirjaus/kohdennusdelegaatti.h"

#include "kirjaus/kirjauswg.h"

#include "tiliotekirjaaja.h"

#include <QDebug>

TilioteApuri::TilioteApuri(QWidget *parent, Tosite *tosite)
    : ApuriWidget (parent,tosite),
      ui( new Ui::TilioteApuri),
      model_(new TilioteModel(this)),
      kirjaaja_(new TilioteKirjaaja(this))
{

    ui->setupUi(this);

    ui->oteView->setItemDelegateForColumn( TilioteModel::TILI, new TiliDelegaatti() );
    ui->oteView->setItemDelegateForColumn( TilioteModel::EURO, new EuroDelegaatti() );
    ui->oteView->setItemDelegateForColumn( TilioteModel::KOHDENNUS, new KohdennusDelegaatti() );    

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel( model_ );

    ui->oteView->setModel(proxy_);
    ui->oteView->sortByColumn(TilioteModel::PVM, Qt::AscendingOrder);
    ui->oteView->installEventFilter(this);

    laitaPaivat( tosite->data(Tosite::PVM).toDate() );

    connect( ui->lisaaRiviNappi, &QPushButton::clicked, this, &TilioteApuri::lisaaRivi);
    connect( ui->lisaaTyhjaBtn, &QPushButton::clicked, this, &TilioteApuri::lisaaTyhjaRivi );
    connect( ui->oteView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
             this, SLOT(riviValittu()));
    connect(ui->muokkaaNappi, &QPushButton::clicked, this, &TilioteApuri::muokkaa);
    connect(ui->poistaNappi, &QPushButton::clicked, this, &TilioteApuri::poista);

    connect( model_, &TilioteModel::dataChanged, this, &TilioteApuri::tositteelle);
    connect( model_, &TilioteModel::rowsInserted, this, &TilioteApuri::tositteelle);
    connect( model_, &TilioteModel::rowsRemoved, this, &TilioteApuri::tositteelle);
    connect( model_, &TilioteModel::modelReset, this, &TilioteApuri::tositteelle);

    connect( ui->alkuDate, &KpDateEdit::dateChanged, this, &TilioteApuri::tiliPvmMuutos);
    connect( ui->loppuDate, &KpDateEdit::dateChanged, this, &TilioteApuri::tiliPvmMuutos);

    connect( tosite, &Tosite::pvmMuuttui, this, &TilioteApuri::laitaPaivat);

    ui->tiliCombo->suodataTyypilla("ARP");

    ui->oteView->horizontalHeader()->setSectionResizeMode( TilioteModel::SELITE, QHeaderView::Stretch );
}

TilioteApuri::~TilioteApuri()
{

}

bool TilioteApuri::teeTositteelle()
{
    tosite()->viennit()->asetaViennit( model_->viennit(  ui->tiliCombo->valittuTilinumero() ) );
    if( tosite()->data(Tosite::OTSIKKO).toString().isEmpty())
        tosite()->setData(Tosite::OTSIKKO, tr("Tiliote %1").arg(tosite()->data(Tosite::PVM).toDate().toString("dd.MM.yyyy")));

    naytaSummat();    
    return true;
}

void TilioteApuri::teeReset()
{
    kirjaaja_->close();
    QVariantList viennit = tosite()->viennit()->viennit().toList();
    if( viennit.count() > 1) {
        TositeVienti ekarivi = viennit.first().toMap();
        ui->tiliCombo->valitseTili(ekarivi.tili());
    }
    model_->lataa(viennit);
    lataaHarmaat();    
}

void TilioteApuri::lisaaRivi()
{
    kirjaaja_->show();
}

void TilioteApuri::lisaaTyhjaRivi()
{
    TilioteModel::Tilioterivi rivi;
    rivi.pvm = tosite()->data(Tosite::PVM).toDate();
    model_->lisaaRivi(rivi);
}

void TilioteApuri::riviValittu()
{
    ui->muokkaaNappi->setEnabled( ui->oteView->currentIndex().isValid() );
    ui->poistaNappi->setEnabled( ui->oteView->currentIndex().isValid());
}

void TilioteApuri::muokkaa()
{
    kirjaaja_->show();
    kirjaaja_->muokkaaRivia( proxy_->mapToSource( ui->oteView->currentIndex()).row()  );
}

void TilioteApuri::poista()
{
    model_->poistaRivi( proxy_->mapToSource( ui->oteView->currentIndex()).row() );
}

void TilioteApuri::naytaSummat()
{
    qlonglong panot = 0l;
    qlonglong otot = 0l;

    for(int i=0; i < model_->rowCount(); i++) {
        qlonglong sentit = model_->data( model_->index(i, TilioteModel::EURO), Qt::EditRole ).toLongLong();
        if( sentit > 0)
            panot += sentit;
        else
            otot += qAbs(sentit);
    }
    ui->infoLabel->setText(tr("Panot %L1 € \tOtot %L2 €").arg((panot / 100.0), 0, 'f', 2).arg((otot / 100.0), 0, 'f', 2));
}

void TilioteApuri::tiliPvmMuutos()
{
    qDebug() << "TiliPvmMuutos";
    // Otsikon päivittäminen
    lataaHarmaat();

    Tili tili = kp()->tilit()->tiliNumerolla( ui->tiliCombo->valittuTilinumero() );
    QString iban = tili.str("iban");

    tosite()->setData( Tosite::OTSIKKO,
                       tr("Tiliote %1 - %2 %3")
                       .arg( ui->alkuDate->date().toString("dd.MM.yyyy") )
                       .arg( ui->loppuDate->date().toString("dd.MM.yyyy"))
                       .arg(iban));

}

void TilioteApuri::lataaHarmaat()
{
    model_->lataaHarmaat( ui->tiliCombo->valittuTilinumero(),
                          ui->alkuDate->date(),
                          ui->loppuDate->date());
}

void TilioteApuri::laitaPaivat(const QDate &pvm)
{
    ui->loppuDate->setDate(pvm);
    ui->alkuDate->setDate( pvm.addDays(1).addMonths(-1) );
    tiliPvmMuutos();
}

bool TilioteApuri::eventFilter(QObject *watched, QEvent *event)
{

    if( watched == ui->oteView && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if( ( keyEvent->key() == Qt::Key_Enter ||
            keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Insert ||
            keyEvent->key() == Qt::Key_Tab) &&
                keyEvent->modifiers() == Qt::NoModifier )
        {

            // Insertillä suoraan uusi rivi
            if(  keyEvent->key() == Qt::Key_Insert )
            {
                lisaaTyhjaRivi();
            }

            if( ui->oteView->currentIndex().column() == TilioteModel::SELITE &&
                ui->oteView->currentIndex().row() == model_->rowCount() - 1 )
            {
                lisaaTyhjaRivi();
                ui->oteView->setCurrentIndex( model_->index( model_->rowCount(QModelIndex())-1, TilioteModel::PVM ) );
                return true;
            }

        }
    }
}

