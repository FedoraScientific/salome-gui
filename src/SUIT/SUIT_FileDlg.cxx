#include "SUIT_FileDlg.h"

#include "SUIT_Tools.h"   
#include "SUIT_Session.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_FileValidator.h"

#include <qdir.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qpalette.h>
#include <qobjectlist.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qapplication.h>

#define MIN_COMBO_SIZE 100

QString SUIT_FileDlg::myLastVisitedPath;

/*!
Constructor
*/
SUIT_FileDlg::SUIT_FileDlg( QWidget* parent, bool open, bool showQuickDir, bool modal ) :
QFileDialog( parent, 0, modal ),
myValidator( 0 ),
myQuickCombo( 0 ), myQuickButton( 0 ), myQuickLab( 0 ),
myOpen( open )
{    
  if ( parent->icon() )
    setIcon( *parent->icon() );       
  setSizeGripEnabled( true );
  
  if ( showQuickDir ) {
    // inserting quick dir combo box
    myQuickLab  = new QLabel(tr("LAB_QUICK_PATH"), this);
    myQuickCombo = new QComboBox(false, this);
    myQuickCombo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    myQuickCombo->setMinimumSize(MIN_COMBO_SIZE, 0);
    
    myQuickButton = new QPushButton(tr("BUT_ADD_PATH"), this);

    connect(myQuickCombo,  SIGNAL(activated(const QString&)), this, SLOT(quickDir(const QString&)));
    connect(myQuickButton, SIGNAL(clicked()),                 this, SLOT(addQuickDir()));
    addWidgets(myQuickLab, myQuickCombo, myQuickButton);

    // getting dir list from settings
    QString dirs;
    SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
    if ( resMgr )
      dirs = resMgr->stringValue( "FileDlg", QString( "QuickDirList" ) );

    QStringList dirList = QStringList::split(';', dirs, false);
    if (dirList.count() > 0) {
      for (unsigned i = 0; i < dirList.count(); i++)
        myQuickCombo->insertItem(dirList[i]);
    }
    else {
      myQuickCombo->insertItem(QDir::homeDirPath());
    }
  }
  setMode( myOpen ? ExistingFile : AnyFile );     
  setCaption( myOpen ? tr( "INF_DESK_DOC_OPEN" ) : tr( "INF_DESK_DOC_SAVE" ) );

  // If last visited path doesn't exist -> switch to the first preferred path
  if ( !myLastVisitedPath.isEmpty() ) {
    if ( !processPath( myLastVisitedPath ) && showQuickDir )
      processPath( myQuickCombo->text( 0 ) );
  }
  else {
    if ( showQuickDir )
      processPath(myQuickCombo->text( 0 ) );
  } 

  // set default file validator
  myValidator = new SUIT_FileValidator(this);
}

/*!
Destructor
*/
SUIT_FileDlg::~SUIT_FileDlg() 
{
  setValidator( 0 );
}

/*!
  Redefined from QFileDialog.
*/
void SUIT_FileDlg::polish()
{
  QFileDialog::polish();
  if ( myQuickButton && myQuickLab ) {
    // the following is a workaround for proper layouting of custom widgets
    QValueList<QPushButton*> buttonList;
    QValueList<QLabel*> labelList;
    const QObjectList *list = children();
    QObjectListIt it(*list);
    int maxButWidth = myQuickLab->sizeHint().width();
    int maxLabWidth = myQuickButton->sizeHint().width();
    
    for (; it.current() ; ++it) {
      if ( it.current()->isA( "QLabel" ) ) {
	int tempW = ((QLabel*)it.current())->minimumWidth();
	if ( maxLabWidth < tempW ) maxLabWidth = tempW;
	labelList.append( (QLabel*)it.current() );
      }
      else if( it.current()->isA("QPushButton") ) {
	int tempW = ((QPushButton*)it.current())->minimumWidth();
	if ( maxButWidth < tempW ) maxButWidth = tempW;
	buttonList.append( (QPushButton*)it.current() );
      }
    }
    if (maxButWidth > 0) {
      QValueList<QPushButton*>::Iterator bListIt;
      for ( bListIt = buttonList.begin(); bListIt != buttonList.end(); ++bListIt )
	(*bListIt)->setFixedWidth( maxButWidth );
    }
    if (maxLabWidth > 0) {
      QValueList<QLabel*>::Iterator lListIt;
      for ( lListIt = labelList.begin(); lListIt != labelList.end(); ++lListIt )
	(*lListIt)->setFixedWidth( maxLabWidth );
    }
  }
}

/*!
Sets validator for file names to open/save
Deletes previous validator if the dialog owns it.
*/
void SUIT_FileDlg::setValidator( SUIT_FileValidator* v )
{
  if ( myValidator && myValidator->parent() == this )
    delete myValidator;
  myValidator = v;
}

/*!
Returns the selected file
*/
QString SUIT_FileDlg::selectedFile() const
{
  return mySelectedFile;
}

/*!
Returns 'true' if this is 'Open File' dialog 
and 'false' if 'Save File' dialog
*/
bool SUIT_FileDlg::isOpenDlg() const
{
  return myOpen;
}

/*!
Closes this dialog and sets the return code to 'Accepted'
if the selected name is valid ( see 'acceptData()' )
*/
void SUIT_FileDlg::accept()
{
//  mySelectedFile = QFileDialog::selectedFile().simplifyWhiteSpace(); //VSR- 06/12/02
  if ( mode() != ExistingFiles ) {
    mySelectedFile = QFileDialog::selectedFile(); //VSR+ 06/12/02
    addExtension();
  }
//  mySelectedFile = mySelectedFile.simplifyWhiteSpace(); //VSR- 06/12/02

  /* Qt 2.2.2 BUG: accept() is called twice if you validate 
  the selected file name by pressing 'Return' key in file 
  name editor but this name is not acceptable for acceptData()
  */
  if ( acceptData() ) {
    myLastVisitedPath = dirPath();
    QFileDialog::accept();        
  }
}

/*!
Closes this dialog and sets the return code to 'Rejected'    
*/
void SUIT_FileDlg::reject()
{
  mySelectedFile = QString::null;
  QFileDialog::reject();        
}

/*!
Returns 'true' if selected file is valid.
The validity is checked by a file validator, 
if there is no validator the file is always
considered as valid    
*/
bool SUIT_FileDlg::acceptData()
{    
  if ( myValidator )
  {
    if ( isOpenDlg() )
      if ( mode() == ExistingFiles ) {
	QStringList fileNames = selectedFiles();
	for ( int i = 0; i < fileNames.count(); i++ ) {
	  if ( !myValidator->canOpen( fileNames[i] ) )
	    return false;
	}
	return true;
      }
      else {
	return myValidator->canOpen( selectedFile() );
      }
    else 
      return myValidator->canSave( selectedFile() );
  }
  return true;
}

/*!
Adds an extension to the selected file name
if the file has not it.
The extension is extracted from the active filter.
*/
void SUIT_FileDlg::addExtension()
{
//  mySelectedFile.stripWhiteSpace();//VSR- 06/12/02
//  if ( mySelectedFile.isEmpty() )//VSR- 06/12/02
  if ( mySelectedFile.stripWhiteSpace().isEmpty() )//VSR+ 06/12/02
    return;

//  if ( SUIT_Tools::getFileExtensionFromPath( mySelectedFile ).isEmpty() ) //VSR- 06/12/02
//ota :   16/12/03  if ( SUIT_Tools::getFileExtensionFromPath( mySelectedFile ).isEmpty() ) //VSR+ 06/12/02
//  {

#if QT_VERSION < 0x030000
    QRegExp r( QString::fromLatin1("([a-zA-Z0-9.*? +;#]*)$") );
    int len, index = r.match( selectedFilter(), 0, &len );
#else
    QRegExp r( QString::fromLatin1("\\([a-zA-Z0-9.*? +;#]*\\)$") );
    int index = r.search(selectedFilter());
#endif
    if ( index >= 0 ) 
    {            
#if QT_VERSION < 0x030000
//      QString wildcard = selectedFilter().mid( index + 1, len-2 ); //VSR- 06/12/02
      QString wildcard = selectedFilter().mid( index + 1, len-2 ).stripWhiteSpace(); //VSR+ 06/12/02
#else
//      QString wildcard = selectedFilter().mid( index + 1, r.matchedLength()-2 ); //VSR- 06/12/02
      QString wildcard = selectedFilter().mid( index + 1, r.matchedLength()-2 ).stripWhiteSpace(); //VSR+ 06/12/02
#endif
    int aLen = mySelectedFile.length();
    if ( mySelectedFile[aLen - 1] == '.')
	  //if the file name ends with the point remove it
	    mySelectedFile.truncate(aLen - 1);
    QString anExt = "." + SUIT_Tools::extension( mySelectedFile ).stripWhiteSpace();
      // From the filters list make a pattern to validate a file extension
      // Due to transformations from the filter list (*.txt *.*xx *.c++ SUIT*.* ) we 
      // will have the pattern (\.txt|\..*xx|\.c\+\+|\..*) (as we validate extension only we remove
      // stay extension mask only in the pattern
      QString aPattern(wildcard);
      QRegExp anExtRExp("("+aPattern.replace(QRegExp("(^| )[0-9a-zA-Z*_?]*\\."), " \\.").
			stripWhiteSpace().replace(QRegExp("\\s+"), "|").
			replace(QRegExp("[*]"),".*").replace(QRegExp("[+]"),"\\+") + ")");
      
      if ( anExtRExp.match(anExt) == -1 ) //if a selected file extension does not match to filter's list
	{ //remove a point if it is at the word end
          int aExtLen = anExt.length();
	  if (anExt[ aExtLen - 1 ] == '.')  anExt.truncate( aExtLen - 1 );
	  index = wildcard.findRev( '.' );    
	  if ( index >= 0 ) 
	    mySelectedFile += wildcard.mid( index ); //add the extension
	}
    }
  //  }
}

/*!
  Processes selection : tries to set given path or filename as selection
*/
bool SUIT_FileDlg::processPath( const QString& path )
{
  if ( !path.isNull() ) {
    QFileInfo fi( path );
    if ( fi.exists() ) {
      if ( fi.isFile() )
	setSelection( path );
      else if ( fi.isDir() )
	setDir( path );
      return true;
    }
    else {
      if ( QFileInfo( fi.dirPath() ).exists() ) {
	setDir( fi.dirPath() );
	setSelection( path );
	return true;
      }
    }
  }
  return false;
}
/*!
  Called when user selects item from "Quick Dir" combo box
*/
void SUIT_FileDlg::quickDir(const QString& dirPath)
{
  if ( !QDir(dirPath).exists() ) {
    SUIT_MessageBox::error1(this, 
			   tr("ERR_ERROR"),
			   tr("ERR_DIR_NOT_EXIST").arg(dirPath), 
			   tr("BUT_OK"));
    
  }
  else {
    processPath(dirPath);
  }
}
/*!
  Called when user presses "Add" button - adds current directory to quick directory
  list and to the preferences
*/
void SUIT_FileDlg::addQuickDir()
{
  QString dp = dirPath();
  if ( !dp.isEmpty() ) {
    QDir dir( dp );
    // getting dir list from settings
    QString dirs;
    SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
    if ( resMgr )
      dirs = resMgr->stringValue( "FileDlg", QString( "QuickDirList" ) );
    QStringList dirList = QStringList::split(';', dirs, false);
    bool found = false;
    bool emptyAndHome = false;
    if ( dirList.count() > 0 ) {
      for ( unsigned i = 0; i < dirList.count(); i++ ) {
	QDir aDir( dirList[i] );
	if ( aDir.canonicalPath().isNull() && dirList[i] == dir.absPath() ||
	    !aDir.canonicalPath().isNull() && aDir.exists() && aDir.canonicalPath() == dir.canonicalPath() ) {
	  found = true;
	  break;
	}
      }
    }
    else {
      emptyAndHome = dir.canonicalPath() == QDir(QDir::homeDirPath()).canonicalPath();
    }
    if ( !found ) {
      dirList.append( dp );
      resMgr->setValue( "FileDlg", QString( "QuickDirList" ), dirList.join(";") );
      if ( !emptyAndHome )
	myQuickCombo->insertItem( dp );
    }
  }
}
/*!
  Returns the file name for Open/Save [ static ]
*/
QString SUIT_FileDlg::getFileName( QWidget*            parent, 
				   const QString&      initial, 
                                   const QStringList&  filters, 
                                   const QString&      caption,
                                   bool                open,
				   bool                showQuickDir, 
				   SUIT_FileValidator* validator )
{            
  SUIT_FileDlg* fd = new SUIT_FileDlg( parent, open, showQuickDir, true );    
  if ( !caption.isEmpty() )
    fd->setCaption( caption );
  if ( !initial.isEmpty() ) { 
    fd->processPath( initial ); // VSR 24/03/03 check for existing of directory has been added to avoid QFileDialog's bug
  }
  fd->setFilters( filters );        
  if ( validator )
    fd->setValidator( validator );
  fd->exec();
  QString filename = fd->selectedFile();
  delete fd;
  qApp->processEvents();
  return filename;
}


/*!
  Returns the list of files to be opened [ static ]
*/
QStringList SUIT_FileDlg::getOpenFileNames( QWidget*            parent, 
					    const QString&      initial, 
					    const QStringList&  filters, 
					    const QString&      caption,
					    bool                showQuickDir, 
					    SUIT_FileValidator* validator )
{            
  SUIT_FileDlg* fd = new SUIT_FileDlg( parent, true, showQuickDir, true );    
  fd->setMode( ExistingFiles );     
  if ( !caption.isEmpty() )
    fd->setCaption( caption );
  if ( !initial.isEmpty() ) { 
    fd->processPath( initial ); // VSR 24/03/03 check for existing of directory has been added to avoid QFileDialog's bug
  }
  fd->setFilters( filters );        
  if ( validator )
    fd->setValidator( validator );
  fd->exec();
  QStringList filenames = fd->selectedFiles();
  delete fd;
  qApp->processEvents();
  return filenames;
}

/*!
  Existing directory selection dialog [ static ]
*/
QString SUIT_FileDlg::getExistingDirectory( QWidget*       parent,
					    const QString& initial,
					    const QString& caption, 
					    bool           showQuickDir )
{
  SUIT_FileDlg* fd = new SUIT_FileDlg( parent, true, showQuickDir, true);
  if ( !caption.isEmpty() )
    fd->setCaption( caption );
  if ( !initial.isEmpty() ) {
    fd->processPath( initial ); // VSR 24/03/03 check for existing of directory has been added to avoid QFileDialog's bug
  }
  fd->setMode( DirectoryOnly );
  fd->setFilters(tr("INF_DIRECTORIES_FILTER"));
  fd->exec();
  QString dirname = fd->selectedFile();
  delete fd;
  qApp->processEvents();
  return dirname;
  
}
