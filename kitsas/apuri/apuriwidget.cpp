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
#include "apuriwidget.h"
#include "model/tosite.h"
#include <QDebug>
#include "model/tositeviennit.h"

ApuriWidget::ApuriWidget(QWidget *parent, Tosite *pTosite) : QWidget(parent), pTosite_(pTosite)
{

}

void ApuriWidget::reset()
{
    if(!resetointiKaynnissa_) {
        resetointiKaynnissa_ = true;
        teeReset();
        resetointiKaynnissa_ = false;
    }
}

bool ApuriWidget::tositteelle()
{
    if( resetointiKaynnissa_ || tosite()->resetoidaanko())
        return false;
    return teeTositteelle();
}

void ApuriWidget::salliMuokkaus(bool sallitaanko)
{
    for( QObject* object : children()) {
        QWidget* widget = qobject_cast<QWidget*>(object);
        if(widget)
            widget->setEnabled(sallitaanko);
    }
}

void ApuriWidget::tuo(QVariantMap /*map*/)
{

}

bool ApuriWidget::resetoidaanko() const
{
    return resetointiKaynnissa_ ||
            pTosite_->resetoidaanko();
}

void ApuriWidget::asetaViennit(const QVariantList &viennit)
{
    resetointiKaynnissa_ = true;
    tosite()->viennit()->asetaViennit(viennit);
    teeReset();
    resetointiKaynnissa_ = false;
}

void ApuriWidget::aloitaResetointi()
{
    resetointiKaynnissa_ = true;
}

void ApuriWidget::lopetaResetointi()
{
    resetointiKaynnissa_ = false;
    tositteelle();
}


