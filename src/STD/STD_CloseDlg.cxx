#include "STD_CloseDlg.h"

#include <qpushbutton.h>
#include <qhbuttongroup.h>
#include <qlayout.h> 
#include <qlabel.h> 
#include <qmessagebox.h>

#ifndef WNT
using namespace std;
#endif

//================================================================================
/*!
 * \brief creates a Close dialog box
 * \param parent a parent widget
 * \param modal bool argument, if true the dialog box is a modal dialog box
 * \param f style flags
 * 
 */
//================================================================================

STD_CloseDlg::STD_CloseDlg ( QWidget * parent, bool modal, WFlags f )
     : QDialog ( parent, "", TRUE,  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  setSizeGripEnabled ( TRUE ) ;
  setCaption( tr( "CLOSE_DLG_CAPTION" ) );

  QVBoxLayout* m_vbL = new QVBoxLayout( this );
  m_vbL->setMargin( 11 ); m_vbL->setSpacing( 6 );

  QLabel* m_lIcon  = new QLabel (this, "m_lDescr");
  QPixmap pm = QMessageBox::standardIcon( QMessageBox::Warning );
  m_lIcon->setPixmap( pm );
  m_lIcon->setScaledContents( false );
  m_lIcon->setAlignment( Qt::AlignCenter );

  QLabel* m_lDescr = new QLabel (this, "m_lDescr");
  m_lDescr->setText ( tr ("CLOSE_DLG_DESCRIPTION") );
  m_lDescr->setAlignment( Qt::AlignCenter );
  m_lDescr->setMinimumHeight( m_lDescr->sizeHint().height()*5 );
  m_lDescr->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

  QHBoxLayout* m_hl1 = new QHBoxLayout();
  m_hl1->setMargin( 0 ); m_hl1->setSpacing( 6 );
  m_hl1->addWidget( m_lIcon );
  m_hl1->addWidget( m_lDescr );

  m_pb1 = new QPushButton( tr ("CLOSE_DLG_SAVE_CLOSE"), this );
  m_pb2 = new QPushButton( tr ("CLOSE_DLG_CLOSE"),      this );
  m_pb3 = new QPushButton( tr ("CLOSE_DLG_UNLOAD"),     this );
  m_pb4 = new QPushButton( tr ("BUT_CANCEL"), this );

  QGridLayout* m_hl2 = new QGridLayout();
  m_hl2->setMargin( 0 ); m_hl2->setSpacing( 6 );
  m_hl2->addWidget( m_pb1, 0, 0 );
  m_hl2->addWidget( m_pb2, 0, 1 );
  m_hl2->addWidget( m_pb3, 0, 2 );
  m_hl2->addColSpacing( 3, 10 );
  m_hl2->setColStretch( 3, 5 );
  m_hl2->addWidget( m_pb4, 0, 4 );
  
  m_vbL->addLayout( m_hl1 );
  m_vbL->addLayout( m_hl2 );

  connect( m_pb1, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( m_pb2, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( m_pb3, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( m_pb4, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

//================================================================================
/*!
 * \brief reaction on clicked(pressed) button
 */
//================================================================================

void STD_CloseDlg::onButtonClicked()
{
  QPushButton* btn = ( QPushButton* )sender();
  if ( btn == m_pb1 )
    done( 1 );
  if ( btn == m_pb2 )
    done( 2 );
  if ( btn == m_pb3 )
    done( 3 );
}
