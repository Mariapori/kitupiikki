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

#include "laskutusverodelegaatti.h"

#include <QComboBox>
#include "db/verotyyppimodel.h"
#include "db/kirjanpito.h"
#include "../laskualvcombo.h"
#include "model/tositerivit.h"
#include "kantalaskudialogi.h"

LaskutusVeroDelegaatti::LaskutusVeroDelegaatti(KantaLaskuDialogi *dialogi) :
    QItemDelegate(dialogi)
{

}

QWidget *LaskutusVeroDelegaatti::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    LaskuAlvCombo::AsiakasVeroLaji aslaji = LaskuAlvCombo::EU;
    KantaLaskuDialogi* dlg = qobject_cast<KantaLaskuDialogi*>(this->parent());
    if( dlg->asiakkaanAlvTunnus().isEmpty())
        aslaji = LaskuAlvCombo::YKSITYINEN;
    else if(dlg->asiakkaanAlvTunnus().startsWith("FI"))
        aslaji = LaskuAlvCombo::KOTIMAA;
    bool ennakkolasku = dlg->maksutapa() == Lasku::ENNAKKOLASKU;

    LaskuAlvCombo *cbox = new LaskuAlvCombo(parent);
    cbox->alusta(aslaji, index.data(TositeRivit::AlvKoodiRooli).toInt(), ennakkolasku);
    return cbox;
}

void LaskutusVeroDelegaatti::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int koodi = index.data(TositeRivit::AlvProsenttiRooli).toInt() * 100 + index.data(TositeRivit::AlvKoodiRooli).toInt();
    if( index.data(TositeRivit::VoittomarginaaliRooli).toInt())
        koodi = index.data(TositeRivit::VoittomarginaaliRooli).toInt();

    LaskuAlvCombo *cbox = qobject_cast<LaskuAlvCombo*>(editor);
    cbox->setCurrentIndex( cbox->findData(koodi) );
}

void LaskutusVeroDelegaatti::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    LaskuAlvCombo *cbox = qobject_cast<LaskuAlvCombo*>(editor);
    int koodi = cbox->currentData().toInt();

    model->setData(index, koodi / 100, TositeRivit::AlvProsenttiRooli);
    model->setData(index, koodi % 100, TositeRivit::AlvKoodiRooli);

}


