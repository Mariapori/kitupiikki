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
#include "tallennettuwidget.h"
#include "ui_tallennettuwidget.h"

#include "db/kirjanpito.h"

#include <QTimer>


TallennettuWidget::TallennettuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TallennettuWidget),
    timer_(new QTimer(this))
{
    ui->setupUi(this);
    setVisible(false);

    setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    timer_->setSingleShot(true);
    connect( timer_, &QTimer::timeout, this, &TallennettuWidget::hide );
}

TallennettuWidget::~TallennettuWidget()
{
    delete ui;
}

void TallennettuWidget::nayta(int tunnus, const QDate &paiva, const QString &sarja, int tila, int aika)
{

    timer_->start(aika);

    ui->tunnisteellaLabel->setVisible( tunnus );
    ui->tunnisteLabel->setVisible( tunnus );
    ui->kausiLabel->setVisible( tunnus );
    ui->luonnosLabel->setVisible( !tunnus );

    if( tunnus ) {
        ui->tunnisteLabel->setText( QString("%2 %1 ")
                                .arg(tunnus)                                
                                .arg(sarja));
        ui->kausiLabel->setText("/ " + kp()->tilikausiPaivalle(paiva).pitkakausitunnus() );
    } else if(tila == Tosite::MALLIPOHJA) {
        ui->luonnosLabel->setText(tr("Tosite tallennettu \nmallipohjana\n"));
    } else {
        ui->luonnosLabel->setText(tr("Tosite tallennettu tilassa\n") + Tosite::tilateksti(tila));
    }

    setVisible(true);

    QWidget* isi = qobject_cast<QWidget*>( parent() );

    move( isi->width() / 2 - width() / 2,
          isi->height() - height() + 5 );
    raise();

}

void TallennettuWidget::piiloon()
{
    if( timer_->remainingTime() < 59500) {
        timer_->stop();
        hide();
    }
}



void TallennettuWidget::mousePressEvent(QMouseEvent *event)
{
    piiloon();
    QWidget::mousePressEvent(event);
}
