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
#include "tulomenoapuri.h"
#include "ui_tulomenoapuri.h"
#include "tmrivit.h"

#include "db/kirjanpito.h"
#include "model/tosite.h"
#include "model/tositeviennit.h"
#include "model/asiakas.h"
#include "model/asiakastoimittajataydentaja.h"
#include "db/tositetyyppimodel.h"
#include "kirjaus/kohdennusproxymodel.h"
#include "rekisteri/asiakasdlg.h"
#include "rekisteri/toimittajadlg.h"

#include <QSortFilterProxyModel>
#include <QDebug>
#include <QCompleter>

TuloMenoApuri::TuloMenoApuri(QWidget *parent, Tosite *tosite) :
    ApuriWidget (parent, tosite),
    ui(new Ui::TuloMenoApuri),
    rivit_(new TmRivit),
    kohdennusProxy_( new KohdennusProxyModel(this)),
    asiakasTaydentaja_( new QCompleter(this))
{
    ui->setupUi(this);

    veroFiltteri_ = new QSortFilterProxyModel(this);
    veroFiltteri_->setFilterRole( VerotyyppiModel::KoodiTekstiRooli);
    veroFiltteri_->setSourceModel( kp()->alvTyypit());
    ui->alvCombo->setModel(veroFiltteri_);

    ui->kohdennusCombo->setModel(kohdennusProxy_);
    ui->kohdennusCombo->setModelColumn( KohdennusModel::NIMI);

    ui->alkuEdit->setNull();
    ui->loppuEdit->setNull();
    ui->erapaivaEdit->setNull();

    asiakasTaydentaja_->setCaseSensitivity(Qt::CaseInsensitive);
    ui->toimittajaEdit->setCompleter( asiakasTaydentaja_ );


    ui->tilellaView->setModel( rivit_);
    ui->tilellaView->horizontalHeader()->setSectionResizeMode(TmRivit::TILI, QHeaderView::Stretch);


    connect( ui->tiliEdit, &TilinvalintaLine::textChanged, this, &TuloMenoApuri::tiliMuuttui );
    connect( ui->maaraEdit, &KpEuroEdit::textChanged, this, &TuloMenoApuri::maaraMuuttui);
    connect( ui->verotonEdit, &KpEuroEdit::textChanged, this, &TuloMenoApuri::verotonMuuttui);
    connect( ui->alvSpin, SIGNAL( valueChanged(double) ), this, SLOT( veroprossaMuuttui()) );

    connect( ui->lisaaRiviNappi, &QPushButton::clicked, this, &TuloMenoApuri::lisaaRivi);
    connect( ui->poistaRiviNappi, &QPushButton::clicked, this, &TuloMenoApuri::poistaRivi);

    connect( ui->tilellaView->selectionModel(), &QItemSelectionModel::currentRowChanged , this, &TuloMenoApuri::haeRivi);
    connect( ui->seliteEdit, &QLineEdit::textChanged, this, &TuloMenoApuri::seliteMuuttui);
    connect( ui->alvCombo, &QComboBox::currentTextChanged, this, &TuloMenoApuri::verolajiMuuttui);
    connect( ui->vahennysCheck, &QCheckBox::stateChanged, this, &TuloMenoApuri::alvVahennettavaMuuttui);

    connect( ui->kohdennusCombo, &QComboBox::currentTextChanged, this, &TuloMenoApuri::kohdennusMuuttui);
    connect( ui->merkkauksetCC, &CheckCombo::currentTextChanged, this, &TuloMenoApuri::merkkausMuuttui );

    connect( ui->alkuEdit, &KpDateEdit::dateChanged, this, &TuloMenoApuri::jaksoAlkaaMuuttui);
    connect( ui->loppuEdit, &KpDateEdit::dateChanged, this, &TuloMenoApuri::jaksoLoppuuMuuttui);

    connect( ui->maksutapaCombo, &QComboBox::currentTextChanged, this, &TuloMenoApuri::maksutapaMuuttui);
    connect( ui->vastatiliCombo, &TiliCombo::tiliValittu, this, &TuloMenoApuri::tositteelle);

    connect( ui->viiteEdit, &QLineEdit::textChanged, [this] (const QString& text) {this->tosite()->setData(Tosite::VIITE, text);});
    connect( ui->erapaivaEdit, &KpDateEdit::dateChanged, [this] (const QDate& date) {this->tosite()->setData(Tosite::ERAPVM, date);});

    connect( ui->toimittajaEdit, &QLineEdit::editingFinished, [this] { this->tosite()->asiakas()->valitse( this->ui->toimittajaEdit->text() ); this->tositteelle(); });

    connect( tosite, &Tosite::pvmMuuttui, this, &TuloMenoApuri::haeKohdennukset );
    connect( ui->toimittajaButton, &QPushButton::clicked, this, &TuloMenoApuri::muokkaaAsiakasta);
}

TuloMenoApuri::~TuloMenoApuri()
{
    delete ui;
}

void TuloMenoApuri::otaFokus()
{
    ui->tiliEdit->setFocus();
}

void TuloMenoApuri::teeReset()
{
    // Haetaan tietoja mallista ;)
    bool menoa = tosite()->data(Tosite::TYYPPI).toInt() == TositeTyyppi::MENO;
    alusta( menoa );

    ui->viiteEdit->setText( tosite()->data(Tosite::VIITE).toString() );
    ui->erapaivaEdit->setDate( tosite()->data(Tosite::ERAPVM).toDate());

    if( menoa ) {

    } else {
        ui->toimittajaEdit->setText( tosite()->asiakas()->nimi() );
    }

    // Haetaan rivien tiedot

    QVariantList vientiLista = tosite()->viennit()->viennit().toList();
    if( vientiLista.count())
    {
        Tili* vastatili = kp()->tilit()->tiliNumerolla( vientiLista.at(0).toMap().value("tili").toInt() );
        if( vastatili ) {
            if( vastatili->onko(TiliLaji::OSTOVELKA) || vastatili->onko(TiliLaji::MYYNTISAATAVA))
                ui->maksutapaCombo->setCurrentIndex(LASKU);
            else if( vastatili->onko(TiliLaji::PANKKITILI))
                ui->maksutapaCombo->setCurrentIndex(PANKKI);
            else if( vastatili->onko(TiliLaji::KATEINEN))
                ui->maksutapaCombo->setCurrentIndex(KATEINEN);

            ui->vastatiliCombo->valitseTili( vastatili->numero() );
        }
        maksutapaMuuttui();
    }
    rivit_->clear();
    int rivi = 0;
    int i=1;

    if( vientiLista.count() && vientiLista.at(0).toMap().value("alvkoodi").toInt() == AlvKoodi::MAAHANTUONTI)
        i=0;    // Jos kirjataan maahantuonnin veroa, ei ekana ollutkaan välttis vastakirjausta

    while( i < vientiLista.count())
    {
        TositeVienti map = vientiLista.at(i).toMap();
        rivit_->lisaaRivi();
        rivit_->setTili( rivi,  map.value("tili").toInt() );
        rivit_->setSelite(rivi, map.value("selite").toString());
        rivit_->setKohdennus(rivi, map.value("kohdennus").toInt());

        rivit_->setMerkkaukset(rivi, map.value("merkkaukset").toList() );


        qlonglong maara = menoa ? qRound( map.value("debet").toDouble() * 100.0 ) :
                                  qRound( map.value("kredit").toDouble() * 100.0 );


        int verokoodi = map.data(TositeVienti::ALVKOODI).toInt();
        rivit_->setAlvKoodi( rivi, verokoodi );
        rivit_->setAlvProsentti( rivi, map.data(TositeVienti::ALVPROSENTTI).toDouble() );

        rivit_->setJaksoalkaa( rivi, map.data(TositeVienti::JAKSOALKAA).toDate());
        rivit_->setJaksoloppuu( rivi, map.data(TositeVienti::JAKSOLOPPUU).toDate() );

        i++;

        if( verokoodi == AlvKoodi::EIALV || verokoodi == AlvKoodi::ALV0 || verokoodi == AlvKoodi::MYYNNIT_MARGINAALI ||
            verokoodi == AlvKoodi::OSTOT_MARGINAALI || verokoodi == AlvKoodi::YHTEISOMYYNTI_TAVARAT ||
            verokoodi == AlvKoodi::YHTEISOMYYNTI_PALVELUT)
        {
            rivit_->setMaara(rivi, maara);
        } else {
            bool vahennyksia = false;
            qlonglong veroa = 0;

            while( i < vientiLista.count() )
            {
                TositeVienti vmap = vientiLista.at(i).toMap();
                int vkoodi = vmap.data(TositeVienti::ALVKOODI).toInt();
                if( vkoodi < AlvKoodi::ALVKIRJAUS )
                    break;
                if( vkoodi / 100 == AlvKoodi::ALVVAHENNYS / 100 || vkoodi == AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_OSTO) {
                    vahennyksia = true;
                    veroa = qRound( vmap.data(TositeVienti::DEBET).toDouble() * 100.0 );
                } else if( vkoodi / 100 == AlvKoodi::ALVKIRJAUS / 100 || vkoodi == AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_MYYNTI) {
                    veroa = qRound( vmap.data(TositeVienti::KREDIT).toDouble() * 100.0 );
                } else if( vkoodi == AlvKoodi::MAAHANTUONTI_VERO)
                    rivit_->setAlvKoodi( rivi, AlvKoodi::MAAHANTUONTI_VERO);
                i++;
            }
            rivit_->setMaara( rivi, maara + veroa );
            rivit_->setNetto( rivi, maara );

            rivit_->setEiVahennysta(rivi, !vahennyksia);

        }
        rivi++;

    }

    ui->tilellaView->setVisible( rivi > 1 );
    ui->poistaRiviNappi->setEnabled( rivi > 1 );
    if( !rivi )
    {
        rivit_->lisaaRivi();
        ui->tiliEdit->clear();      // TODO: Oletustilin hakeminen
        ui->maaraEdit->setCents(0);
        ui->seliteEdit->clear();
        tiliMuuttui();
        maksutapaMuuttui();
        verolajiMuuttui();
        haeKohdennukset();
    }
    else if( ui->tilellaView->selectionModel()->currentIndex().row() == 0)
        haeRivi( rivit_->index(0,0) );
    ui->tilellaView->selectRow(0);

}

bool TuloMenoApuri::teeTositteelle()
{
    // Lasketaan ensin summa
    qlonglong summa = 0l;
    int riveja = rivit_->rowCount();


    bool menoa = tosite()->data(Tosite::TYYPPI).toInt() == TositeTyyppi::MENO;
    QDate pvm = tosite()->data(Tosite::PVM).toDate();
    QString otsikko = tosite()->data(Tosite::OTSIKKO).toString();

    QVariantList viennit;


    for(int i=0; i < riveja; i++) {
        double maara = rivit_->maara(i) / 100.0 ;
        double netto = rivit_->netto(i) / 100.0;
        double vero = (rivit_->maara(i) - rivit_->netto(i)) / 100.0;
        double veroprosentti = rivit_->alvProsentti(i);
        int verokoodi = rivit_->alvkoodi(i);

        bool maahantuonninvero = false;
        if( verokoodi == AlvKoodi::MAAHANTUONTI_VERO) {
            maahantuonninvero = true;
            verokoodi = AlvKoodi::MAAHANTUONTI;
        }


        TositeVienti vienti;

        // Kirjataanko nettoa vai bruttoa?
        double kirjattava = ( verokoodi == AlvKoodi::MYYNNIT_NETTO  || verokoodi == AlvKoodi::OSTOT_NETTO ||
                                 verokoodi == AlvKoodi::MAKSUPERUSTEINEN_MYYNTI || verokoodi == AlvKoodi::MAKSUPERUSTEINEN_OSTO ||
                                 ((verokoodi == AlvKoodi::RAKENNUSPALVELU_OSTO || verokoodi == AlvKoodi::YHTEISOHANKINNAT_TAVARAT ||
                                 verokoodi == AlvKoodi::YHTEISOHANKINNAT_PALVELUT || verokoodi == AlvKoodi::MAAHANTUONTI )
                                  && !rivit_->eiVahennysta(i)) ) ? netto : maara;

        vienti.insert("pvm", pvm);
        vienti.insert("tili", rivit_->tili(i).numero());
        if( menoa )
            vienti.insert("debet", kirjattava);
        else
           vienti.insert("kredit", kirjattava);
        QString selite = rivit_->selite(i);
        if( selite.isEmpty())
            vienti.insert("selite", otsikko);
        else
            vienti.insert("selite", selite);

        vienti.setAlvProsentti( veroprosentti);
        vienti.setAlvKoodi( verokoodi );

        vienti.setKohdennus( rivit_->kohdennus(i) );
        vienti.setMerkkaukset( rivit_->merkkaukset(i));

        QDate alkupvm = rivit_->jaksoalkaa(i);
        if( alkupvm.isValid())
        {
            vienti.setJaksoalkaa( pvm );
            QDate loppupvm = rivit_->jaksoloppuu(i);
            if( loppupvm.isValid() )
                vienti.setJaksoloppuu( loppupvm );
        }


        viennit.append(vienti);


        // Alv-saamisten kirjaaminen
        if( verokoodi == AlvKoodi::OSTOT_NETTO || verokoodi == AlvKoodi::MAKSUPERUSTEINEN_OSTO ||
              ((verokoodi == AlvKoodi::RAKENNUSPALVELU_OSTO || verokoodi == AlvKoodi::YHTEISOHANKINNAT_TAVARAT ||
                verokoodi == AlvKoodi::YHTEISOHANKINNAT_PALVELUT || verokoodi == AlvKoodi::MAAHANTUONTI )
               && !rivit_->eiVahennysta(i)) ) {

            TositeVienti palautus;
            palautus.setPvm(pvm);
            if( verokoodi == AlvKoodi::MAKSUPERUSTEINEN_OSTO) {
                palautus.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::KOHDENTAMATONALVSAATAVA).numero() );
                palautus.setAlvKoodi( AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_OSTO );
            } else {
                palautus.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::ALVSAATAVA).numero());
                palautus.setAlvKoodi( AlvKoodi::ALVVAHENNYS + verokoodi );
            }

            palautus.setDebet( vero );
            palautus.setAlvProsentti( veroprosentti );
            palautus.setSelite( otsikko );
            viennit.append(palautus);
        }

        // Alv-veron kirjaaminen
        if( verokoodi == AlvKoodi::MYYNNIT_NETTO || verokoodi == AlvKoodi::MAKSUPERUSTEINEN_MYYNTI ||
                verokoodi == AlvKoodi::RAKENNUSPALVELU_OSTO || verokoodi == AlvKoodi::YHTEISOHANKINNAT_TAVARAT ||
                verokoodi == AlvKoodi::YHTEISOHANKINNAT_PALVELUT || verokoodi == AlvKoodi::MAAHANTUONTI )
        {
            TositeVienti verorivi;
            verorivi.setPvm(pvm);
            if( verokoodi == AlvKoodi::MAKSUPERUSTEINEN_MYYNTI) {
                verorivi.setTili( kp()->tilit()->tiliTyypilla( TiliLaji::KOHDENTAMATONALVVELKA ).numero() );
                verorivi.setAlvKoodi( AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_MYYNTI);
            } else {
                verorivi.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::ALVVELKA).numero());
                verorivi.setAlvKoodi( AlvKoodi::ALVKIRJAUS + verokoodi);
            }
            verorivi.setKredit( vero );
            verorivi.setAlvProsentti( veroprosentti);
            verorivi.setSelite(otsikko);
            viennit.append(verorivi);
        }

        // Mahdollinen maahantuonnin veron kirjaamisen vastakirjaaminen
        if( maahantuonninvero ) {
            TositeVienti tuonti;
            tuonti.setPvm(pvm);
            tuonti.setTili(rivit_->tili(i).numero());
            tuonti.setKredit(netto);
            tuonti.setSelite(otsikko);
            tuonti.setAlvKoodi(AlvKoodi::MAAHANTUONTI_VERO);
            viennit.append(tuonti);

        } else {
            if( verokoodi == AlvKoodi::RAKENNUSPALVELU_OSTO || verokoodi == AlvKoodi::YHTEISOHANKINNAT_TAVARAT ||
                    verokoodi == AlvKoodi::YHTEISOHANKINNAT_PALVELUT || verokoodi == AlvKoodi::MAAHANTUONTI )
                summa += qRound(netto * 100.0);
            else
                summa += qRound(maara * 100.0);
        }
    }

    if( summa ) {
        QVariantMap vasta;
        vasta.insert("pvm", pvm);
        vasta.insert("tili", ui->vastatiliCombo->valittuTilinumero() );
        if( menoa )
            vasta.insert("kredit", summa / 100.0);
        else
            vasta.insert("debet", summa / 100.0);
        vasta.insert("selite", otsikko);
        viennit.insert(0, vasta);
    }

    if( tosite()->asiakas()->id())
        tosite()->setData(Tosite::ASIAKAS, tosite()->asiakas()->id());

    tosite()->viennit()->asetaViennit(viennit);

    return true;
}

void TuloMenoApuri::lisaaRivi()
{
    ui->tilellaView->setVisible(true);
    ui->tilellaView->selectRow( rivit_->lisaaRivi() );
}

void TuloMenoApuri::poistaRivi()
{
    rivit_->poistaRivi( rivilla() );
    ui->poistaRiviNappi->setEnabled( rivit_->rowCount() > 1 );
}

void TuloMenoApuri::tiliMuuttui()
{
    Tili tili = ui->tiliEdit->valittuTili();
    rivit_->setTili( rivilla(), tili.numero() );

    bool tasapoisto = tili.onko(TiliLaji::TASAERAPOISTO);
    ui->poistoLabel->setVisible(tasapoisto);
    ui->poistoSpin->setVisible(tasapoisto);

    // TODO: Vero-oletusten hakeminen

    if( kp()->asetukset()->onko(AsetusModel::ALV))
    {
        ui->alvCombo->setCurrentIndex( ui->alvCombo->findData( tili.json()->luku("AlvLaji"), VerotyyppiModel::KoodiRooli ) );
        ui->alvSpin->setValue( tili.json()->variant("AlvProsentti").toDouble() );
    }

    tositteelle();

}

void TuloMenoApuri::verolajiMuuttui()
{
    int verolaji = ui->alvCombo->currentData(VerotyyppiModel::KoodiRooli).toInt();

    bool naytaMaara = !( verolaji == AlvKoodi::RAKENNUSPALVELU_OSTO || verolaji == AlvKoodi::YHTEISOHANKINNAT_TAVARAT
                                 || verolaji == AlvKoodi::YHTEISOHANKINNAT_PALVELUT || verolaji == AlvKoodi::MAAHANTUONTI
                                 || verolaji == AlvKoodi::MAAHANTUONTI_VERO)  ;

    bool naytaVeroton =  verolaji == AlvKoodi::OSTOT_NETTO || verolaji == AlvKoodi::MYYNNIT_NETTO ||
                                 verolaji == AlvKoodi::OSTOT_BRUTTO || verolaji == AlvKoodi::MYYNNIT_BRUTTO ||
                                 verolaji == AlvKoodi::MAKSUPERUSTEINEN_OSTO || verolaji == AlvKoodi::MAKSUPERUSTEINEN_MYYNTI ||
                                 !naytaMaara ;

    ui->maaraLabel->setVisible(naytaMaara);
    ui->maaraEdit->setVisible(naytaMaara);
    ui->verotonLabel->setVisible(naytaVeroton);
    ui->verotonEdit->setVisible(naytaVeroton);


    ui->alvSpin->setVisible( !ui->alvCombo->currentData(VerotyyppiModel::NollaLajiRooli).toBool() );
    ui->vahennysCheck->setVisible( verolaji == AlvKoodi::RAKENNUSPALVELU_OSTO ||
                                   verolaji == AlvKoodi::YHTEISOHANKINNAT_TAVARAT ||
                                   verolaji == AlvKoodi::YHTEISOHANKINNAT_PALVELUT ||
                                   verolaji == AlvKoodi::MAAHANTUONTI ||
                                   verolaji == AlvKoodi::MAAHANTUONTI_VERO);

    rivit_->setAlvKoodi( rivilla(), verolaji );
    if( ui->alvCombo->currentData(VerotyyppiModel::NollaLajiRooli).toBool() )
        rivit_->setAlvProsentti(rivilla(), 0.0);
    else
    {
        if( ui->alvSpin->value() == 0.0)
            ui->alvSpin->setValue(24.0);
        rivit_->setAlvProsentti(rivilla(), ui->alvSpin->value() );
    }


    tositteelle();
}

void TuloMenoApuri::maaraMuuttui()
{
    qlonglong maara = ui->maaraEdit->asCents();

    if( ui->maaraEdit->hasFocus()) {
        bruttoSnt_ = maara;
        nettoSnt_ = 0;
        double verokanta = ui->alvSpin->value();
        qlonglong vero = qRound( maara * verokanta / ( 100 + verokanta) );
        ui->verotonEdit->setCents( maara - vero);
    }

    rivit_->setMaara( rivilla(), maara);
    tositteelle();
}

void TuloMenoApuri::verotonMuuttui()
{
    qlonglong veroton = ui->verotonEdit->asCents();

    if( ui->verotonEdit->hasFocus()) {
        nettoSnt_ = veroton;
        bruttoSnt_ = 0;
        double verokanta = ui->alvSpin->value();
        ui->maaraEdit->setCents( qRound( ( 100 + verokanta) * veroton / 100.0 ) );
    }
    rivit_->setNetto(rivilla(), veroton);
    tositteelle();
}

void TuloMenoApuri::veroprossaMuuttui()
{
    double verokanta = ui->alvSpin->value();
    rivit_->setAlvProsentti(rivilla(), verokanta  );

    if( bruttoSnt_ ) {
        qlonglong vero = qRound( bruttoSnt_ * verokanta / ( 100 + verokanta) );
        ui->verotonEdit->setCents( bruttoSnt_ - vero);
    } else if( nettoSnt_) {
        ui->maaraEdit->setCents( qRound( (100 + verokanta) * nettoSnt_ / 100.0 ) );
    }
}

void TuloMenoApuri::alvVahennettavaMuuttui()
{
    rivit_->setEiVahennysta( rivilla(), ui->vahennysCheck->isChecked() );
    tositteelle();
}

void TuloMenoApuri::seliteMuuttui()
{
    rivit_->setSelite( rivilla(), ui->seliteEdit->text());
    tositteelle();
}

void TuloMenoApuri::maksutapaMuuttui()
{
    int maksutapa = ui->maksutapaCombo->currentIndex();
    bool menoa = tosite()->data(Tosite::TYYPPI).toInt() == TositeTyyppi::MENO;

    ui->viiteLabel->setVisible( maksutapa == LASKU);
    ui->viiteEdit->setVisible( maksutapa == LASKU);

    ui->erapaivaLabel->setVisible( maksutapa == LASKU);
    ui->erapaivaEdit->setVisible( maksutapa == LASKU);

    ui->eraLabel->setVisible( maksutapa == HYVITYS || maksutapa == ENNAKKO);
    ui->eraCombo->setVisible(maksutapa == HYVITYS || maksutapa == ENNAKKO);

    // TODO: Tilien valinnat järkevämmin ;)

    switch (maksutapa) {
    case LASKU:
        if(menoa)
            ui->vastatiliCombo->suodataTyypilla("BO");
        else
            ui->vastatiliCombo->suodataTyypilla("AO");
        break;
    case PANKKI:
        ui->vastatiliCombo->suodataTyypilla("ARP");
        break;
    case KATEINEN:
        ui->vastatiliCombo->suodataTyypilla("ARK");
        break;
    default:
        ui->vastatiliCombo->suodataTyypilla("[AB].*");

    }
    // Vastatilivalintaa ei tartte näyttää jos vaihtoehtoja on vain yksi
    ui->vastatiliLabel->setVisible( ui->vastatiliCombo->model()->rowCount() > 1 );
    ui->vastatiliCombo->setVisible( ui->vastatiliCombo->model()->rowCount() > 1 );
}

void TuloMenoApuri::kohdennusMuuttui()
{
    rivit_->setKohdennus( rivilla(), ui->kohdennusCombo->currentData(KohdennusModel::IdRooli).toInt());
    tositteelle();    
}

void TuloMenoApuri::merkkausMuuttui()
{

    rivit_->setMerkkaukset( rivilla(), ui->merkkauksetCC->selectedDatas());
    tositteelle();

    qDebug() << "!" << rivit_->merkkaukset( rivilla() );
}

void TuloMenoApuri::jaksoAlkaaMuuttui()
{
    QDate alkupvm = ui->alkuEdit->date();
    rivit_->setJaksoalkaa( rivilla(), alkupvm);
    ui->loppuEdit->setEnabled( alkupvm.isValid() );
    tositteelle();
}

void TuloMenoApuri::jaksoLoppuuMuuttui()
{
    rivit_->setJaksoloppuu( rivilla(), ui->loppuEdit->date());
    tositteelle();
}

void TuloMenoApuri::haeRivi(const QModelIndex &index)
{
    aloitaResetointi();
    int rivi = index.row();
    ui->tiliEdit->valitseTili( rivit_->tili(rivi));
    ui->maaraEdit->setCents( rivit_->maara(rivi) );
    ui->verotonEdit->setCents( rivit_->netto(rivi));
    ui->alvCombo->setCurrentIndex( ui->alvCombo->findData( rivit_->alvkoodi(rivi), VerotyyppiModel::KoodiRooli ) );
    ui->alvSpin->setValue( rivit_->alvProsentti(rivi) );
    ui->vahennysCheck->setChecked( rivit_->eiVahennysta(rivi) );

    ui->alkuEdit->setDate( rivit_->jaksoalkaa(rivi) );
    ui->loppuEdit->setEnabled( rivit_->jaksoalkaa(rivi).isValid());
    ui->loppuEdit->setDate( rivit_->jaksoloppuu(rivi));

    bruttoSnt_ = rivit_->maara(rivi);
    nettoSnt_ = 0;

    ui->seliteEdit->setText( rivit_->selite(rivi));

    haeKohdennukset();
    lopetaResetointi();
}

void TuloMenoApuri::haeKohdennukset()
{
    int nykyinenKohdennus = rivit_->rowCount() ? rivit_->kohdennus( rivilla() ) : 0 ;
    QVariantList merkatut =  rivit_->rowCount() ?  rivit_->merkkaukset( rivilla()) : QVariantList();
    QDate pvm = tosite()->data(Tosite::PVM).toDate();

    kohdennusProxy_->asetaKohdennus( nykyinenKohdennus );
    kohdennusProxy_->asetaPaiva( pvm );

    ui->kohdennusLabel->setVisible( kohdennusProxy_->rowCount() > 1);
    ui->kohdennusCombo->setVisible( kohdennusProxy_->rowCount() > 1);

    ui->kohdennusCombo->setCurrentIndex( ui->kohdennusCombo->findData(nykyinenKohdennus, KohdennusModel::IdRooli ));

    KohdennusProxyModel merkkausproxy(this, pvm, -1, KohdennusProxyModel::MERKKKAUKSET );
    ui->merkkauksetCC->clear();

    ui->merkkauksetLabel->setVisible( merkkausproxy.rowCount());
    ui->merkkauksetCC->setVisible( merkkausproxy.rowCount());

    for(int i=0; i < merkkausproxy.rowCount(); i++) {
        int koodi = merkkausproxy.data( merkkausproxy.index(i,0), KohdennusModel::IdRooli ).toInt();
        QString nimi = merkkausproxy.data( merkkausproxy.index(i,0), KohdennusModel::NimiRooli ).toString();

        Qt::CheckState state = merkatut.contains( koodi ) ? Qt::Checked : Qt::Unchecked;
        ui->merkkauksetCC->addItem(nimi, koodi, state);
    }

}

void TuloMenoApuri::muokkaaAsiakasta()
{
    if( tosite()->tyyppi() == TositeTyyppi::TULO) {

        AsiakasDlg dlg(this, tosite()->asiakas());
        if( dlg.exec() == QDialog::Accepted )
            ui->toimittajaEdit->setText( tosite()->asiakas()->nimi() );
    } else if ( tosite()->tyyppi() == TositeTyyppi::MENO) {
        ToimittajaDlg dlg(this);
        dlg.exec();
    }
}

void TuloMenoApuri::alusta(bool meno)
{

    if(meno) {
        ui->tiliLabel->setText( tr("Menotili") );
        ui->tiliEdit->suodataTyypilla("(AP|D).*");
        veroFiltteri_->setFilterRegExp("^(0|2[1-79]|927)");
        ui->toimittajaLabel->setText( tr("Toimittaja"));
    } else {
        ui->tiliLabel->setText( tr("Tulotili"));
        ui->tiliEdit->suodataTyypilla("(AP|C).*");
        veroFiltteri_->setFilterRegExp("^(0|1[1-79])");
        ui->toimittajaLabel->setText( tr("Asiakas"));
        asiakasTaydentaja_->setModel( tosite()->asiakas()->taydentaja() );

        qDebug() << " --- " << tosite()->asiakas()->taydentaja()->rowCount();
    }


    bool alv = kp()->asetukset()->onko( AsetusModel::ALV );
    ui->alvLabel->setVisible(alv);
    ui->alvCombo->setVisible(alv);

    ui->erapaivaEdit->setDateRange(QDate(), QDate());
    ui->loppuEdit->setDateRange( kp()->tilitpaatetty().addDays(1), QDate() );
}

int TuloMenoApuri::rivilla() const
{
    if( ui->tilellaView->currentIndex().row() < 0 )
        return 0;
    return ui->tilellaView->currentIndex().row();

}