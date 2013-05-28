
#include "view.hpp"
#include "defs.hpp"
#include "model.hpp"
#include "listeners.hpp"
#include "config.hpp"
#include "tools.hpp"

#include <osl/thread.hxx>
#include <cppuhelper/weakref.hxx>

#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/XStyleSettingsSupplier.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/awt/grid/XGridControl.hpp>
#include <com/sun/star/awt/XUserInputInterception.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/XDialogProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/io/XTextInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/style/HorizontalAlignment.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/view/SelectionType.hpp>

#ifndef TASKPANE
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#endif

#define A2S( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )


namespace ww
{

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::deployment;
using namespace ::com::sun::star::ucb;

using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiComponentFactory;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::frame::XDispatchHelper;
using ::com::sun::star::frame::XDispatchProvider;

using ::rtl::OUString;

// tools.cpp
Reference< XInterface > getConfig( 
            const Reference< XComponentContext > & xContext, 
            const OUString & sNodeName ) throw ( RuntimeException );


class SetFocusThread : public osl::Thread
{
public:
    SetFocusThread( const Reference< XWindow > & rWindow );
    virtual ~SetFocusThread();
    
    virtual void SAL_CALL run();
    
    virtual void SAL_CALL onTerminated();
private:
    ::com::sun::star::uno::WeakReferenceHelper * pWeakRefHelper;
};


SetFocusThread::SetFocusThread( 
        const Reference< XWindow > & rWindow )
: pWeakRefHelper( new ::com::sun::star::uno::WeakReferenceHelper( rWindow ) )
{
}

SetFocusThread::~SetFocusThread()
{
    delete pWeakRefHelper;
    pWeakRefHelper = NULL;
}

void SAL_CALL SetFocusThread::run()
{
    // ToDo wait few hundred seconds?
    try
    {
        if ( pWeakRefHelper )
        {
            Reference< XWindow > xWindow( pWeakRefHelper->get(), UNO_QUERY_THROW );
            xWindow->setFocus();
        }
    }
    catch (...)
    {
        onTerminated();
    }
}

void SAL_CALL SetFocusThread::onTerminated()
{
    delete this;
}


Sequence< OUString > WWView::m_aResource;


WWView::WWView( const Reference< XComponentContext > & xContext, 
                const Reference< XFrame > & xFrame, 
                const Reference< XWindow > & xParentWindow )
 : m_pModel( NULL )
 , m_xContext( xContext )
 , m_xFrame( xFrame )
 , m_xParentWindow( xParentWindow )
 , m_bUseInputLine( true )
 , m_nRowHeight( -1 )
{
    try
    {
        ConfigReader aConfig( m_xContext );
        m_bUseInputLine = aConfig.getBoolValue( A2S( PROP_USE_INPUT_LINE ) );
        loadResource();
    }
    catch ( Exception & )
    {
    }
    
    createWindow();
    SetFocusThread * thread = new SetFocusThread( m_xFrame->getContainerWindow() );
    if ( thread->createSuspended() )
        thread->resume();
}


WWView::~WWView()
{
    m_pModel = NULL;
    try
    {
        if ( m_xFocusListener.is() )
            m_xFocusListener.clear();
        if ( m_xKeyHandler.is() )
        {
            controlFocusLost( Edit );
            m_xKeyHandler.clear();
        }
        if ( m_xGridKeyHandler.is() )
        {
            controlFocusLost( Grid );
            m_xGridKeyHandler.clear();
        }
        if ( m_xPopupMenu.is() )
            m_xPopupMenu.clear();
        
        if ( m_xWindowListener.is() )
        {
            m_xParentWindow->removeWindowListener( m_xWindowListener );
            m_xWindowListener.clear();
        }
        
        m_xGridRowSelection.clear();
        
        if ( m_xActionListener.is() )
            m_xActionListener.clear();
        
        Reference< XComponent > xComp;
        xComp.set( m_xContainerWindow, UNO_QUERY );
        m_xContainerWindow.clear();
        if ( xComp.is() )
            xComp->dispose();
    }
    catch ( Exception & )
    {
    }
    m_xContext.clear();
    m_xFrame.clear();
}

Reference< XWindow > WWView::getWindow() const
{
    return m_xContainerWindow;
}


void WWView::setModel( WWModel * pModel )
{
    m_pModel = pModel;
}


void WWView::disconnectModel()
{
    m_pModel = NULL;
}

void WWView::focusToDoc() const
{
    Reference< XWindow > xWindow( m_xFrame->getContainerWindow() );
    if ( xWindow.is() )
        xWindow->setFocus();
}


Reference< XMutableGridDataModel > WWView::getGridDataModel() const
{
    Reference< XControlContainer > xContainer( m_xContainerWindow, UNO_QUERY );
    if ( xContainer.is() )
    {
        Reference< XControl > xControl( xContainer->getControl( A2S( CTL_GRID ) ), UNO_QUERY );
        if ( xControl.is() )
        {
            Reference< XPropertySet > xPropSet( xControl->getModel(), UNO_QUERY );
            if ( xPropSet.is() )
            {
                 Reference< XMutableGridDataModel > xGridDataModel( 
                        xPropSet->getPropertyValue( A2S( "GridDataModel" ) ), UNO_QUERY );
                return xGridDataModel;
             }
        }
    }
    return Reference< XMutableGridDataModel >();
}


Reference< XModel > WWView::getDocument() const throw ( RuntimeException )
{
    if ( m_xFrame.is() )
    {
        Reference< XController > xController( m_xFrame->getController() );
        if ( xController.is() )
        {
            Reference< XModel > xModel( xController->getModel() );
            if ( xModel.is() )
                return xController->getModel();
        }
    }
    throw RuntimeException();
}


Reference< XInterface > WWView::getSelection() const throw ( RuntimeException )
{
    return getDocument()->getCurrentSelection();
}


Sequence< sal_Int32 > WWView::getSelectedEntryIndex() const
{
    if ( m_xGridRowSelection.is() )
        return m_xGridRowSelection->getSelectedRows();
    return Sequence< sal_Int32 >();
}

bool WWView::hasGridSelection() const
{
    if ( m_xGridRowSelection.is() )
        return m_xGridRowSelection->hasSelectedRows();
    return false;
}


void WWView::selectRow( const sal_Int32 nIndex, const bool bGotoRow ) const
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        if ( m_xGridRowSelection.is() && m_pModel && nIndex >= 0 )
        {
            if ( nIndex < m_pModel->getRowCount() )
                m_xGridRowSelection->selectRow( nIndex );
            if ( bGotoRow )
            {
                Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY_THROW );
                xGridControl->goToCell( 0, nIndex );
            }
        }
    }
    catch ( Exception & )
    {
    }
}

void WWView::selectRow( const ::sal_Int32 nX, const ::sal_Int32 nY, const ::sal_Int16 nModifires ) const
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY_THROW );
        const sal_Int32 nIndex = xGridControl->getRowAtPoint( nX, nY );
        if ( nIndex >= 0 )
        {
            if ( !( nModifires & ::com::sun::star::awt::KeyModifier::MOD1 ) )
                m_xGridRowSelection->deselectAllRows();
            m_xGridRowSelection->selectRow( nIndex );
        }
    }
    catch ( Exception & )
    {
    }
}


int WWView::getRowAt( const ::sal_Int32 nX, const ::sal_Int32 nY ) const
{
    Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY );
    if ( xGridControl.is() )
        return xGridControl->getRowAtPoint( nX, nY );
    return -1;
}

int WWView::getRowHeight()
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_nRowHeight < 0 )
    {
        Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY );
        if ( xGridControl.is() )
        {
            sal_Int32 nRow = xGridControl->getRowAtPoint( 0, 0 );
            sal_Int32 nY = 0;
            for ( ; nY < 50; ++nY )
            {
                if ( xGridControl->getRowAtPoint( 0, nY ) != nRow )
                    break;
            }
            const sal_Int32 nStartY = nY;
            ++nRow;
            const sal_Int32 nNextEnd = nY + 50;
            for ( ; nY < nNextEnd; ++nY )
            {
                if ( xGridControl->getRowAtPoint( 0, nY ) != nRow )
                    break;
            }
            m_nRowHeight = nY - nStartY;
        }
    }
    return m_nRowHeight;
}


void WWView::controlFocusGained( const ControlType nType ) const
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_xFrame.is() )
    {
        try
        {
#ifndef TASKPANE
            // If the panel is there in the sidebar, 
            // keyhandler for the document controller does not take 
            // any events, so events like to push enter key can not be consumed.
            // For workaround the problem, to set the keyhandler to the toolkit.
            // ToDo XExtendedToolkit is deprecated to use
            Reference< XWindowPeer > xWindowPeer(
                    m_xFrame->getContainerWindow(), UNO_QUERY_THROW );
            Reference< XExtendedToolkit > xUserInputInterception(
                    xWindowPeer->getToolkit(), UNO_QUERY_THROW );
#else
            Reference< XUserInputInterception > xUserInputInterception( 
                    m_xFrame->getController(), UNO_QUERY_THROW );
#endif
            xUserInputInterception->addKeyHandler( 
                nType == Edit ? m_xKeyHandler : m_xGridKeyHandler );
        }
        catch ( Exception & )
        {
        }
    }
}

void WWView::controlFocusLost( const ControlType nType ) const
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_xFrame.is() )
    {
        try
        {
#ifndef TASKPANE
            Reference< XWindowPeer > xWindowPeer(
                    m_xFrame->getContainerWindow(), UNO_QUERY_THROW );
            Reference< XExtendedToolkit > xUserInputInterception(
                    xWindowPeer->getToolkit(), UNO_QUERY_THROW );
#else
            Reference< XUserInputInterception > xUserInputInterception( 
                    m_xFrame->getController(), UNO_QUERY_THROW );
#endif
            xUserInputInterception->removeKeyHandler( 
                nType == Edit ? m_xKeyHandler : m_xGridKeyHandler );
        }
        catch ( Exception & )
        {
        }
    }
}


OUString WWView::getInputLineText() const
{
    static const OUString sPropText = A2S( "Text" );
    
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XControl > xControl( xContainerControl->getControl( A2S( CTL_EDIT ) ), UNO_SET_THROW );
        Reference< XPropertySet > xPropSet( xControl->getModel(), UNO_QUERY_THROW );
        OUString s;
        xPropSet->getPropertyValue( sPropText ) >>= s;
        return s;
    }
    catch ( Exception & )
    {
    }
    return OUString();
}

void WWView::setInputLine( const ::rtl::OUString & rText ) const
{
    static const OUString sPropText = A2S( "Text" );
    
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XControl > xControl( xContainerControl->getControl( A2S( CTL_EDIT ) ), UNO_SET_THROW );
        Reference< XPropertySet > xPropSet( xControl->getModel(), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sPropText, makeAny( rText ) );
    }
    catch ( Exception & )
    {
    }
}

void WWView::updateButtonsState() const
{
    try
    {
        if ( m_xGridRowSelection.is() )
        {
            const sal_Bool bState = m_xGridRowSelection->hasSelectedRows();
            Reference< XWindow > xWindow;
            Reference< XControlContainer > xContainer( m_xContainerWindow, UNO_QUERY_THROW );
            xWindow.set( xContainer->getControl( A2S( CTL_BTN_DELETE ) ), UNO_QUERY_THROW );
            xWindow->setEnable( bState );
            xWindow.set( xContainer->getControl( A2S( CTL_BTN_GOTO ) ), UNO_QUERY_THROW );
            xWindow->setEnable( bState );
        }
    }
    catch ( Exception & )
    {
    }
}

void WWView::gridSelectionChanged() const
{
    // ToDo update button state
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_pModel )
    {
        const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
        if ( aIndex.getLength() == 1 )
            setInputLine( m_pModel->getFormula( aIndex[0] ) );
        updateButtonsState();
    }
}

void WWView::updateRowFormula() const
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_pModel )
    {
        // ToDo multiple selection support?
        const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
        if ( aIndex.getLength() == 1 )
            m_pModel->setFormula( aIndex[0], getInputLineText() );
    }
}


void WWView::executeCommand( const int nCommand )
{
    if ( !m_pModel )
        return;
    switch ( nCommand )
    {
        case CMD_ADD:
            m_pModel->addEntry();
            break;
        case CMD_REMOVE:
        {
            const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
            if ( aIndex.getLength() )
            {
                m_pModel->removeEntries( aIndex );
                executeGridCommand( CMD_SELECT_CURRENT, 0 );
                //updateButtonsState();
            }
            break;
        }
        case CMD_UPDATE:
            m_pModel->updateAll();
            break;
        case CMD_GOTO:
        {
            const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
            if ( aIndex.getLength() == 1 )
            {
                const OUString sAddress = m_pModel->getAddress( aIndex[0] );
                if ( sAddress.getLength() )
                    gotoPoint( sAddress );
            }
            break;
        }
        case CMD_OPTION:
            showOptionPopup();
            break;
        case CMD_CLEAR:
            m_pModel->removeAllEntries();
            //updateButtonsState();
            break;
        case CMD_INPUT_LINE:
            switchInputLine( !m_bUseInputLine );
            break;
        case CMD_SETTINGS:
            openOption();
            break;
        case CMD_ABOUT:
        {
            Reference< XInputStream > xInputStream;
            Reference< XDialog > xDialog;
            try
            {
                Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_SET_THROW );
                const OUString sExtId = A2S( EXT_ID );
                
                Reference< XExtensionManager > xExtManager (
                    ::com::sun::star::deployment::ExtensionManager::get( m_xContext ) );
                Reference< XPackage > xPackage;
                xPackage.set( xExtManager->getDeployedExtension( 
                    A2S( "user" ), sExtId, OUString(), Reference< XCommandEnvironment >() ) );
                if ( !xPackage.is() )
                    xPackage.set( xExtManager->getDeployedExtension( 
                        A2S( "shared" ), sExtId, OUString(), Reference< XCommandEnvironment >() ) );
                if ( !xPackage.is() )
                    xPackage.set( xExtManager->getDeployedExtension( 
                        A2S( "bundle" ), sExtId, OUString(), Reference< XCommandEnvironment >() ) );
                if ( !xPackage.is() )
                    return; // not installed?
                
                Reference< XSimpleFileAccess > xSfa ( 
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.ucb.SimpleFileAccess" ), m_xContext ), UNO_QUERY_THROW );
                xInputStream.set( xSfa->openFileRead( A2S( LICENSE_FILE ) ), UNO_QUERY_THROW );
                
                Reference< XTextInputStream > xTextInputStream( 
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.io.TextInputStream" ), m_xContext ), UNO_QUERY_THROW );
                Reference< XActiveDataSink > xActiveDataSink( xTextInputStream, UNO_QUERY_THROW );
                xActiveDataSink->setInputStream( xInputStream );
                const OUString sLicense = xTextInputStream->readString( Sequence< sal_Unicode >(), sal_False );
                xInputStream->closeInput();
                
                Reference< XDialogProvider > xDialogProvider( 
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.awt.DialogProvider" ), m_xContext ), UNO_QUERY_THROW );
                xDialog.set( xDialogProvider->createDialog( 
                                        A2S( ABOUT_DIALOG ) ), UNO_SET_THROW );
                
                Reference< XControl > xControl;
                Reference< XPropertySet > xPropSet;
                Reference< XControlContainer > xContainer( xDialog, UNO_QUERY_THROW );
                xControl.set( xContainer->getControl( A2S( "label_name" ) ), UNO_SET_THROW );
                xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
                xPropSet->setPropertyValue( A2S( "Label" ), makeAny( xPackage->getName() ) );
                // ToDo display name?
                
                xControl.set( xContainer->getControl( A2S( "label_version" ) ), UNO_SET_THROW );
                xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
                xPropSet->setPropertyValue( A2S( "Label" ), makeAny( xPackage->getVersion() ) );
                
                xControl.set( xContainer->getControl( A2S( "edit_text" ) ), UNO_SET_THROW );
                xPropSet.set( xControl->getModel(), UNO_QUERY_THROW );
                xPropSet->setPropertyValue( A2S( "Text" ), makeAny( sLicense ) );
                
                xDialog->execute();
                Reference< XComponent > xComponent( xDialog, UNO_QUERY_THROW );
                xDialog.clear();
                xComponent->dispose();
            }
            catch ( Exception & )
            {
                if ( xInputStream.is() )
                    xInputStream->closeInput();
                if ( xDialog.is() )
                {
                    Reference< XComponent > xComponent( xDialog, UNO_QUERY );
                    if ( xComponent.is() )
                        xComponent->dispose();
                }
            }
            break;
        }
        default:
            break;
    }
}


void WWView::executeGridCommand( const int nCommand, const int nModifires )
{
    if ( !m_pModel )
        return;
    try
    {
        Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY_THROW );
        const sal_Int32 nCurrent = xGridControl->getCurrentRow();
        
        switch ( nCommand )
        {
            case CMD_HOME:
            case CMD_END:
            case CMD_UP:
            case CMD_DOWN:
            {
                if ( nCommand == CMD_UP && nCurrent < 0 ||
                     nCommand == CMD_DOWN && nCurrent > ( m_pModel->getRowCount() - 1 ) )
                    break;
                sal_Int32 nNextRow = 0;
                switch ( nCommand )
                {
                    case CMD_UP:
                        nNextRow = nCurrent - 1;
                        break;
                    case CMD_DOWN:
                        nNextRow = nCurrent + 1;
                        break;
                    case CMD_HOME:
                        nNextRow = 0;
                        break;
                    case CMD_END:
                        nNextRow = m_pModel->getRowCount() - 1;
                    default:
                        break;
                }
                switch ( nModifires )
                {
                    case KeyModifier::SHIFT:
                    {
                        sal_Int32 nStart = 0;
                        sal_Int32 nEnd = 0;
                        switch ( nCommand )
                        {
                            case CMD_UP:
                                nStart = nNextRow;
                                nEnd = nCurrent;
                                break;
                            case CMD_DOWN:
                                nStart = nCurrent;
                                nEnd = nNextRow;
                                break;
                            case CMD_HOME:
                                nStart = 0;
                                nEnd = nCurrent;
                                break;
                            case CMD_END:
                                nStart = nCurrent;
                                nEnd = nNextRow;
                            default:
                                break;
                        }
                        m_xGridRowSelection->deselectAllRows();
                        xGridControl->goToCell( 0, nNextRow );
                        for ( sal_Int32 nPos = nStart; nPos <= nEnd; ++nPos )
                                m_xGridRowSelection->selectRow( nPos );
                        break;
                    }
                    case KeyModifier::MOD1:
                        xGridControl->goToCell( 0, nNextRow );
                        break;
                    case KeyModifier::SHIFT | KeyModifier::MOD1:
                        break;
                    default:
                        m_xGridRowSelection->deselectRow( nCurrent );
                        xGridControl->goToCell( 0, nNextRow );
                        m_xGridRowSelection->selectRow( nNextRow );
                        break;
                }
                break;
            }
            case CMD_SELECT_CURRENT:
            {
                xGridControl->goToCell( 0, nCurrent );
                m_xGridRowSelection->selectRow( nCurrent );
                break;
            }
            case CMD_SPACE:
                if ( m_xGridRowSelection->isRowSelected( nCurrent ) )
                    m_xGridRowSelection->deselectRow( nCurrent );
                else
                    m_xGridRowSelection->selectRow( nCurrent );
                break;
            default:
                break;
        }
    }
    catch ( Exception & )
    {
    }
}


void WWView::openOption()
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XPropertySet > xPropSet( 
                getConfig( m_xContext, A2S( CONFIG_OPTIONS ) ), UNO_QUERY_THROW );
        OUString sOptionsPageURL;
        xPropSet->getPropertyValue( A2S( "OptionsPage" ) ) >>= sOptionsPageURL;
        
        Reference< ::com::sun::star::util::XMacroExpander > xMacroExpander( 
            m_xContext->getValueByName( 
                A2S( "/singletons/com.sun.star.util.theMacroExpander" ) ), UNO_QUERY_THROW );
        sOptionsPageURL = xMacroExpander->expandMacros( sOptionsPageURL );
        
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0] = PropertyValue( 
            A2S( "OptionsPageURL" ), 
            0, 
            makeAny( sOptionsPageURL.copy( 20 ) ), 
            ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
        executeDispatch( A2S( ".uno:OptionsTreeDialog" ), aArgs );
    }
    catch ( Exception & )
    {
    }
}


void WWView::switchInputLine( bool bState )
{
    ::osl::MutexGuard const g( m_aMutex );
    m_bUseInputLine = bState;
    try
    {
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XControl > xControl( xContainerControl->getControl( A2S( CTL_EDIT ) ) );
        Reference< XWindow > xWindow( xControl, UNO_QUERY_THROW );
        if ( m_bUseInputLine )
            xWindow->addFocusListener( m_xFocusListener );
        else
            xWindow->removeFocusListener( m_xFocusListener );
        
        xWindow->setVisible( m_bUseInputLine ? sal_True : sal_False );
        if ( m_xParentWindow.is() )
        {
            Rectangle aRect = m_xParentWindow->getPosSize();
            resizeWindow( aRect.Width, aRect.Height );
        }
    }
    catch ( Exception & )
    {
    }
}

void WWView::executeContextMenu( const ::sal_Int32 nX, const ::sal_Int32 nY )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
    if ( !aIndex.getLength() )
        return;
    try
    {
        const OUString * pRes = m_aResource.getConstArray();
        const sal_Int16 nGotoId = 1000;
        
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XControl > xControl( xContainerControl->getControl( A2S( CTL_GRID ) ), UNO_QUERY_THROW );
        Reference< XWindowPeer > xPeer( xControl->getPeer(), UNO_SET_THROW );
        
        if ( !m_xPopupMenu.is() )
        {
            Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_QUERY_THROW );
            m_xPopupMenu.set(
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.awt.PopupMenu" ), m_xContext ), UNO_QUERY_THROW );
            
            m_xPopupMenu->hideDisabledEntries( sal_True );
            
            m_xPopupMenu->insertItem( (sal_Int16)CMD_GOTO, pRes[6], (sal_Int16)0, 0 );
            m_xPopupMenu->insertItem( nGotoId, pRes[13], (sal_Int16)0, 1 );
            m_xPopupMenu->insertItem( (sal_Int16)CMD_REMOVE, pRes[5], (sal_Int16)0, 2 );
            
#ifdef UNIFIED_MENU_API
            Reference< XPopupMenu > xPopup( 
#else
            Reference< XPopupMenuExtended > xPopup( 
#endif
                xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.awt.PopupMenu" ), m_xContext ), UNO_QUERY_THROW );
            m_xPopupMenu->setPopupMenu( nGotoId, xPopup );
        }
        
        Sequence< OUString > aRefs;
        sal_Bool bGotoCellState = sal_False;
        sal_Bool bGotoState = sal_False;
        
        if ( aIndex.getLength() == 1 )
        {
            bGotoCellState = sal_True;
            
            // add formula references
#ifdef UNIFIED_MENU_API
            Reference< XPopupMenu > xPopup( m_xPopupMenu->getPopupMenu( nGotoId ) );
#else
            Reference< XPopupMenuExtended > xPopup( m_xPopupMenu->getPopupMenu( nGotoId ), UNO_QUERY_THROW );
#endif
            if ( xPopup->getItemCount() )
                xPopup->clear();
            
            if ( m_pModel )
            {
                aRefs = m_pModel->getCellReferences( aIndex[0] );
                if ( aRefs.getLength() )
                {
                    sal_Int32 nIndex = 0;
                    const OUString * pRefs = aRefs.getConstArray();
                    const OUString * pRefsEnd = aRefs.getConstArray() + aRefs.getLength();
                    for ( ; pRefs < pRefsEnd; ++pRefs, ++nIndex )
                        xPopup->insertItem( nGotoId + nIndex, *pRefs, (sal_Int16)0, nIndex );
                    
                    bGotoState = sal_True;
                }
            }
        }
        m_xPopupMenu->enableItem( (sal_Int16)CMD_GOTO, bGotoCellState );
        m_xPopupMenu->enableItem( nGotoId, bGotoState );
        
        const sal_Int16 nId = m_xPopupMenu->execute( xPeer, 
                ::com::sun::star::awt::Rectangle( nX, nY, 0, 0 ), 
                ::com::sun::star::awt::PopupMenuDirection::EXECUTE_DEFAULT );
        if ( nId >= nGotoId )
        {
            const sal_Int32 n = nId - nGotoId;
            if ( 0 <= n && n < aRefs.getLength() )
                gotoPoint( aRefs[n] );
        }
        else if ( nId > 0 )
            executeCommand( (int)nId );
    }
    catch ( Exception & )
    {
    }
}


void WWView::showOptionPopup()
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        const OUString * pRes = m_aResource.getConstArray();
        
        Reference< XControl > xControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XWindowPeer > xPeer( xControl->getPeer(), UNO_SET_THROW );
        
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        Reference< XWindow > xWindow( xContainerControl->getControl( A2S( CTL_BTN_OPTION ) ), UNO_QUERY_THROW );
        Rectangle aRect = xWindow->getPosSize();
        
        Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_QUERY_THROW );
#ifdef UNIFIED_MENU_API
        Reference< XPopupMenu > xPopupMenu(
#else
        Reference< ::com::sun::star::awt::XPopupMenuExtended > xPopupMenu(
#endif
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.awt.PopupMenu" ), m_xContext ), UNO_QUERY_THROW );
        
        xPopupMenu->hideDisabledEntries( sal_True );
        
        xPopupMenu->insertItem( (sal_Int16)CMD_CLEAR, pRes[9], (sal_Int16)0, 0 );
        xPopupMenu->insertSeparator( 1 );
        xPopupMenu->insertItem( (sal_Int16)CMD_INPUT_LINE, pRes[10], ::com::sun::star::awt::MenuItemStyle::CHECKABLE, 2 );
        //xPopupMenu->insertItem( (sal_Int16)CMD_STORE_WATCHES, A2S( "Store watches" ), ::com::sun::star::awt::MenuItemStyle::CHECKABLE, 3 );
        xPopupMenu->insertSeparator( 4 );
        // ToDo move to options
        xPopupMenu->insertItem( (sal_Int16)CMD_SETTINGS, pRes[11], (sal_Int16)0, 5 );
        xPopupMenu->insertItem( (sal_Int16)CMD_ABOUT, pRes[12], (sal_Int16)0, 6 );
        
        xPopupMenu->checkItem( (sal_Int16)CMD_INPUT_LINE, m_bUseInputLine );
        //xPopupMenu->checkItem( (sal_Int16)CMD_STORE_WATCHES, m_bStoreWatches );
        
        const sal_Int16 nId = xPopupMenu->execute( xPeer, 
                ::com::sun::star::awt::Rectangle( aRect.X, aRect.Y + aRect.Height, 0, 0 ), 
                ::com::sun::star::awt::PopupMenuDirection::EXECUTE_DEFAULT );
        if ( nId > 0 )
            executeCommand( (int)nId );
    }
    catch ( Exception & )
    {
    }
}


void WWView::gotoPoint( const OUString & rDesc )
{
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0] = PropertyValue( A2S( "ToPoint" ), 0, makeAny( rDesc ), 
                    ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
    
    executeDispatch( A2S( ".uno:GoToCell" ), aArgs );
    
    focusToDoc();
}


void WWView::executeDispatch( const OUString & rCommand, const Sequence< PropertyValue > & rArgs )
{
    if ( !m_xDispatchHelper.is() )
    {
        try
        {
            Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_QUERY_THROW );
            m_xDispatchHelper.set(
                    xMcf->createInstanceWithContext( 
                        A2S( "com.sun.star.frame.DispatchHelper" ), m_xContext ), UNO_QUERY_THROW );
        }
        catch (...)
        {
            return;
        }
    }
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
        m_xDispatchHelper->executeDispatch( 
                xDispatchProvider, rCommand, A2S( "_self" ), 0, rArgs );
}


bool WWView::startDragging( const ::sal_Int32 nX, const ::sal_Int32 nY ) const
{
    if ( hasGridSelection() )
    {
        try
        {
            // ignore on the vscroll bar, ToDo width of the vscroll
            Reference< XWindow > xWindow( m_xGridRowSelection, UNO_QUERY_THROW );
            if ( nX < ( xWindow->getPosSize().Width - 20 ) )
            {
                Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY_THROW );
                const sal_Int32 nIndex = xGridControl->getRowAtPoint( nX, nY );
                if ( nIndex >= 0 )
                {
                    const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
                    const sal_Int32 * pIndex = aIndex.getConstArray();
                    const sal_Int32 * pIndexEnd = aIndex.getConstArray() + aIndex.getLength();
                    for ( ; pIndex < pIndexEnd; ++pIndex )
                    {
                        if ( *pIndex == nIndex )
                            return true;
                    }
                }
            }
        }
        catch ( Exception & )
        {
        }
    }
    return false;
}

void WWView::endDragging( const ::sal_Int32 nX, const ::sal_Int32 nY )
{
    const Sequence< sal_Int32 > aIndex = getSelectedEntryIndex();
    if ( aIndex.getLength() && m_pModel )
    {
        try
        {
            Reference< XGridControl > xGridControl( m_xGridRowSelection, UNO_QUERY_THROW );
            sal_Int32 nIndex = xGridControl->getRowAtPoint( nX, nY );
            if ( nIndex < 0 )
            {
                if ( nY < getRowHeight() )
                    nIndex = 0;
                else
                    nIndex = m_pModel->getRowCount();
            }
            if ( nIndex >= 0 )
                m_pModel->moveEntries( nIndex, aIndex );
        }
        catch ( Exception & )
        {
        }
    }
}

void WWView::drawDestination( const int nNewRow, const int nOldRow )
{
    int nRow = nNewRow;
    if ( nNewRow < 0 )
    {
        if ( nOldRow > 0 )
            nRow = nOldRow + 1;
        else
            return;
    }
    const int nRowHeight = getRowHeight();
    try
    {
        Reference< XControl > xControl( m_xGridRowSelection, UNO_QUERY_THROW );
        Reference< XWindowPeer > xWindowPeer( xControl->getPeer(), UNO_SET_THROW );
        Reference< XView > xView( m_xGridRowSelection, UNO_QUERY_THROW );
        Reference< XGraphics > xGraphics( xView->getGraphics() );
        if ( !xGraphics.is() )
        {
            Reference< XDevice > xDevice( xWindowPeer, UNO_QUERY_THROW );
            xGraphics.set( xDevice->createGraphics(), UNO_SET_THROW );
        }
        // erase older indicator
        const sal_Int32 nOldY = nRowHeight * ( nOldRow + 1 );
        xWindowPeer->invalidateRect( Rectangle( 1, nOldY - 1, 45, nOldY + 1 ), 
                        ::com::sun::star::awt::InvalidateStyle::UPDATE );
        
        xGraphics->push();
        xGraphics->setLineColor( 0x000000 ); // ToDo get color from style settings
        const sal_Int32 nY = nRowHeight * ( nRow + 1 ); // including the header
        xGraphics->drawRect( 1, nY, 45, 2 );
        xGraphics->pop();
    }
    catch ( Exception & )
    {
    }
}


void WWView::setPointer( const sal_Int32 nPointer )
{
    // ToDo not working well on the grid control, issue 
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        if ( !m_xPointer.is() )
        {
            Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_SET_THROW );
            m_xPointer.set( 
                xMcf->createInstanceWithContext( 
                    A2S( "com.sun.star.awt.Pointer" ), m_xContext ), UNO_QUERY_THROW );
        }
        m_xPointer->setType( nPointer );
        Reference< XControl > xControl( m_xGridRowSelection, UNO_QUERY_THROW );
        Reference< XWindowPeer > xWindowPeer( xControl->getPeer(), UNO_QUERY_THROW );
        xWindowPeer->setPointer( m_xPointer );
    }
    catch ( Exception & )
    {
    }
}


void WWView::loadResource()
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( m_aResource.getLength() == 0 )
    {
        Sequence< OUString > aRes( 15 );
        OUString * pRes = aRes.getArray();
        
        // ToDo map en-US to terms of specific language
        Reference< ::com::sun::star::resource::XStringResourceWithLocation > xRes( 
                getResourceLoader( m_xContext, A2S( RES_DIR ), A2S( RES_NAME ) ) );
        if ( xRes.is() )
        {
            pRes[0] = xRes->resolveString( A2S( "id.sheet" ) );
            pRes[1] = xRes->resolveString( A2S( "id.cell" ) );
            pRes[2] = xRes->resolveString( A2S( "id.value" ) );
            pRes[3] = xRes->resolveString( A2S( "id.formula" ) );
            pRes[4] = xRes->resolveString( A2S( "id.add" ) );
            pRes[5] = xRes->resolveString( A2S( "id.remove" ) );
            pRes[6] = xRes->resolveString( A2S( "id.gotocell" ) );
            pRes[7] = xRes->resolveString( A2S( "id.updateall" ) );
            pRes[8] = xRes->resolveString( A2S( "id.option" ) );
            pRes[9] = xRes->resolveString( A2S( "id.clear" ) );
            pRes[10] = xRes->resolveString( A2S( "id.inputline" ) );
            pRes[11] = xRes->resolveString( A2S( "id.settings" ) );
            pRes[12] = xRes->resolveString( A2S( "id.label.about" ) );
            pRes[13] = xRes->resolveString( A2S( "id.goto" ) );
            pRes[14] = xRes->resolveString( A2S( "id.label.store" ) );
        }
        else
        {
            pRes[0] = A2S( "Sheet" );
            pRes[1] = A2S( "Cell" );
            pRes[2] = A2S( "Value" );
            pRes[3] = A2S( "Formula" );
            pRes[4] = A2S( "Add" );
            pRes[5] = A2S( "Remove" );
            pRes[6] = A2S( "Go to cell" );
            pRes[7] = A2S( "Update" );
            pRes[8] = A2S( "Options" );
            pRes[9] = A2S( "Clear all" );
            pRes[10] = A2S( "Input line" );
            pRes[11] = A2S( "Settings..." );
            pRes[12] = A2S( "About" );
            pRes[13] = A2S( "Go to" );
            pRes[14] = A2S( "Store watches" );
        }
        m_aResource = aRes;
    }
}


Sequence< ::rtl::OUString > WWView::getLabels()
{
    if ( m_aResource.getLength() > 14 )
    {
        const OUString *pResource = m_aResource.getConstArray();
        Sequence< OUString > aRes( 4 );
        OUString * pRes = aRes.getArray();
        pRes[0] = pResource[0];
        pRes[1] = pResource[1];
        pRes[2] = pResource[2];
        pRes[3] = pResource[3];
        return aRes;
    }
    return Sequence< OUString >();
}


#define LEFT_MARGIN        (sal_Int32)3
#define RIGHT_MARGIN       (sal_Int32)3
#define TOP_MARGIN         (sal_Int32)3
#define BUTTON_SEP         (sal_Int32)2
#define BUTTON_WIDTH      (sal_Int32)28
#define BUTTON_HEIGHT     (sal_Int32)28
#define INPUT_LINE_HEIGHT (sal_Int32)23


void WWView::resizeWindow( const sal_Int32 nWidth, const sal_Int32 nHeight )
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
        Reference< XWindow > xWindow;
        Reference< XControlContainer > xContainerControl( m_xContainerWindow, UNO_QUERY_THROW );
        
        xWindow.set( xContainerControl->getControl( A2S( CTL_BTN_OPTION ) ), UNO_QUERY_THROW );
        xWindow->setPosSize( nWidth - BUTTON_WIDTH - RIGHT_MARGIN, 0, 0, 0, 
                            ::com::sun::star::awt::PosSize::X );
        
        xWindow.set( xContainerControl->getControl( A2S( CTL_BTN_UPDATE ) ), UNO_QUERY_THROW );
        xWindow->setPosSize( nWidth - BUTTON_WIDTH - BUTTON_WIDTH - RIGHT_MARGIN * 2, 0, 0, 0, 
                            ::com::sun::star::awt::PosSize::X );
        
        xWindow.set( xContainerControl->getControl( A2S( CTL_GRID ) ), UNO_QUERY_THROW );
        xWindow->setPosSize( 
            0, 
            TOP_MARGIN + BUTTON_HEIGHT + BUTTON_SEP + ( m_bUseInputLine ? INPUT_LINE_HEIGHT + BUTTON_SEP : 0 ), 
            nWidth - LEFT_MARGIN - RIGHT_MARGIN, 
            nHeight - TOP_MARGIN - BUTTON_HEIGHT - BUTTON_SEP * 2 - 
                                    ( m_bUseInputLine ? ( INPUT_LINE_HEIGHT + BUTTON_SEP ) : 0 ), 
            ::com::sun::star::awt::PosSize::SIZE | ::com::sun::star::awt::PosSize::Y );
        
        if ( m_bUseInputLine )
        {
            xWindow.set( xContainerControl->getControl( A2S( CTL_EDIT ) ), UNO_QUERY_THROW );
            xWindow->setPosSize( 0, 0, nWidth - LEFT_MARGIN - RIGHT_MARGIN, 0, 
                                ::com::sun::star::awt::PosSize::WIDTH );
        }
    }
    catch ( Exception & )
    {
    }
}


void WWView::createButton( 
                    const Reference< XControlContainer > & xControlContainer, 
                    const OUString & sName, 
                    const OUString & sHelpText, 
                    const OUString & sImageURL, 
                    const OUString & sActionCommand, 
                    const sal_Int32 nLeft, const sal_Int32 nTop, 
                    const sal_Int32 nWidth, const sal_Int32 nHeight ) throw (RuntimeException)
{
    try
    {
        Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_SET_THROW );
        
        Reference< XControl > xControl(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlButton" ), m_xContext ), UNO_QUERY_THROW );
        Reference< XControlModel > xControlModel(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlButtonModel" ), m_xContext ), UNO_QUERY_THROW );
        Reference< XButton > xButton( xControl, UNO_QUERY_THROW );
        xButton->setActionCommand( sActionCommand );
        xButton->addActionListener( m_xActionListener );
        
        Reference< XPropertySet > xPropSet( xControlModel, UNO_QUERY_THROW );
        xPropSet->setPropertyValue( A2S( "HelpText" ), makeAny( sHelpText ) );
        xPropSet->setPropertyValue( A2S( "ImageURL" ), makeAny( sImageURL ) );
        
        xControl->setModel( xControlModel );
        xControlContainer->addControl( sName, xControl );
        
        Reference< XWindow > xWindow( xControl, UNO_QUERY_THROW );
        xWindow->setPosSize( nLeft, nTop, nWidth, nHeight, 
                             ::com::sun::star::awt::PosSize::POSSIZE );
    }
    catch ( Exception & )
    {
    }
}


void WWView::createWindow()
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( m_aResource.getLength() < 14 )
        return;
    try
    {
        const OUString * pRes = m_aResource.getConstArray();
        
        Reference< XMultiComponentFactory > xMcf( m_xContext->getServiceManager(), UNO_SET_THROW );
        
        Reference< XWindowPeer > xParentWindowPeer( m_xParentWindow, UNO_QUERY_THROW );
        Reference< XToolkit > xToolkit( xParentWindowPeer->getToolkit(), UNO_SET_THROW );
        
        Reference< XControl > xContainerControl(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlContainer" ), m_xContext ), UNO_QUERY_THROW );
        Reference< XControlModel > xContainerControlModel(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlContainerModel" ), m_xContext ), UNO_QUERY_THROW );
        xContainerControl->setModel( xContainerControlModel );
        xContainerControl->createPeer( xToolkit, xParentWindowPeer );
        m_xContainerWindow.set( xContainerControl, UNO_QUERY );
        
        Reference< XStyleSettingsSupplier > xStyleSettingsSupp( xContainerControl, UNO_QUERY_THROW );
        Reference< XStyleSettings > xStyleSettings( xStyleSettingsSupp->getStyleSettings(), UNO_SET_THROW );
        const sal_Int32 nBackgroundColor = xStyleSettings->getDialogColor();
        
        Reference< XControlContainer > xControlContainer( xContainerControl, UNO_QUERY_THROW );
        
        m_xActionListener.set( new ActionListener( this ) );
        const OUString sIconDir = xStyleSettings->getHighContrastMode() == sal_True ? A2S( IMGH_DIR ) : A2S( IMG_DIR );
        // buttons
        createButton( xControlContainer, A2S( CTL_BTN_ADD ), 
                      pRes[4], sIconDir.concat( A2S( IMG_ADD ) ), A2S( "A" ), 
                      LEFT_MARGIN, TOP_MARGIN, 
                      BUTTON_WIDTH, BUTTON_HEIGHT );
        createButton( xControlContainer, A2S( CTL_BTN_DELETE ), 
                      pRes[5], sIconDir.concat( A2S( IMG_DEL ) ), A2S( "B" ), 
                      LEFT_MARGIN + BUTTON_SEP + BUTTON_WIDTH, 
                      TOP_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT );
        createButton( xControlContainer, A2S( CTL_BTN_GOTO ), 
                      pRes[6], sIconDir.concat( A2S( IMG_GOTO ) ), A2S( "D" ), 
                      LEFT_MARGIN + BUTTON_SEP * 2 + BUTTON_WIDTH * 2, 
                      TOP_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT );
        
        createButton( xControlContainer, A2S( CTL_BTN_UPDATE ), 
                      pRes[7], sIconDir.concat( A2S( IMG_UPDATE ) ), A2S( "C" ), 
                      LEFT_MARGIN + BUTTON_SEP * 5 + BUTTON_WIDTH * 5, 
                      TOP_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT );
        createButton( xControlContainer, A2S( CTL_BTN_OPTION ), 
                      pRes[8], sIconDir.concat( A2S( IMG_TUNE ) ), A2S( "G" ), 
                      LEFT_MARGIN + BUTTON_SEP * 6 + BUTTON_WIDTH * 6, 
                      TOP_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT );
        
        // ToDo throbber to show the state and to stop the iteration?
        
        // input line
        Reference< XControl > xEditControl(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlEdit" ), m_xContext ), UNO_QUERY_THROW );
        Reference< XControlModel > xEditControlModel(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.UnoControlEditModel" ), m_xContext ), UNO_QUERY_THROW );
        xEditControl->setModel( xEditControlModel );
        xControlContainer->addControl( A2S( CTL_EDIT ), xEditControl );
        
        // ToDo helptext and help url
        Reference< XWindow > xEditWindow( xEditControl, UNO_QUERY_THROW );
        xEditWindow->setPosSize( LEFT_MARGIN, TOP_MARGIN * 2 + BUTTON_HEIGHT, 
                                 (sal_Int32)100, INPUT_LINE_HEIGHT, 
                                 ::com::sun::star::awt::PosSize::POSSIZE );
        m_xFocusListener.set( new FocusListener( this, Edit ) );
        xEditWindow->addFocusListener( m_xFocusListener );
        xEditWindow->setVisible( m_bUseInputLine ? sal_True : sal_False );
        
        // grid
        Reference< XControl > xGridControl(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.grid.UnoControlGrid" ), m_xContext ), UNO_QUERY_THROW );
        Reference< XControlModel > xGridControlModel(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.grid.UnoControlGridModel" ), m_xContext ), UNO_QUERY_THROW );
        
        Reference< XGridDataModel > xGridDataModel(
            xMcf->createInstanceWithContext( 
                A2S( "com.sun.star.awt.grid.DefaultGridDataModel" ), m_xContext ), UNO_QUERY_THROW );
        
        Reference< XPropertySet > xGridPropSet( xGridControlModel, UNO_QUERY_THROW );
        // ToDo sortable or not by column tab
        xGridPropSet->setPropertyValue( A2S( "GridDataModel" ), makeAny( xGridDataModel ) );
        xGridPropSet->setPropertyValue( A2S( "Border" ), makeAny( (sal_Int16)0 ) );
        xGridPropSet->setPropertyValue( A2S( "HScroll" ), Any( sal_False, getCppuBooleanType() ) );
        xGridPropSet->setPropertyValue( A2S( "SelectionModel" ), makeAny( ::com::sun::star::view::SelectionType_MULTI ) );
        xGridPropSet->setPropertyValue( A2S( "ShowColumnHeader" ), makeAny( sal_True ) );
        xGridPropSet->setPropertyValue( A2S( "ShowRowHeader" ), makeAny( sal_False ) );
        xGridPropSet->setPropertyValue( A2S( "VScroll" ), makeAny( sal_True ) );
        xGridPropSet->setPropertyValue( A2S( "BackgroundColor" ), makeAny( nBackgroundColor ) );
        xGridPropSet->setPropertyValue( A2S( "Tabstop" ), makeAny( sal_True ) );
        
        xGridControl->setModel( xGridControlModel );
        
        Reference< XWindow > xGridWindow( xGridControl, UNO_QUERY_THROW );
        xGridWindow->setPosSize( LEFT_MARGIN, TOP_MARGIN * 3 + BUTTON_HEIGHT + INPUT_LINE_HEIGHT, 
                                 280, 400, 
                                 ::com::sun::star::awt::PosSize::POSSIZE );
        xGridWindow->addFocusListener( new FocusListener( this, Grid ) );
        
        // add columns
        Reference< XGridColumnModel > xGridColumnModel(
                xGridPropSet->getPropertyValue( A2S( "ColumnModel" ) ), UNO_QUERY_THROW );
        Reference< XGridColumn > xGridColumn;
        xGridColumn = xGridColumnModel->createColumn();
        xGridColumn->setTitle( pRes[0] );
        xGridColumnModel->addColumn( xGridColumn );
        xGridColumn = xGridColumnModel->createColumn();
        xGridColumn->setTitle( pRes[1] );
        xGridColumnModel->addColumn( xGridColumn );
        xGridColumn = xGridColumnModel->createColumn();
        xGridColumn->setTitle( pRes[2] );
        xGridColumnModel->addColumn( xGridColumn );
        xGridColumn->setHorizontalAlign( 
                    ::com::sun::star::style::HorizontalAlignment_RIGHT );
        xGridColumn = xGridColumnModel->createColumn();
        xGridColumn->setTitle( pRes[3] );
        xGridColumnModel->addColumn( xGridColumn );
        
        xControlContainer->addControl( A2S( CTL_GRID ), xGridControl );
        m_xGridRowSelection.set( xGridControl, UNO_QUERY_THROW );
        Reference< XMouseListener > xMouseListener( new MouseListener( this ) );
        xGridWindow->addMouseListener( xMouseListener );
        Reference< XMouseMotionListener > xMouseMotionListener( xMouseListener, UNO_QUERY_THROW );
        xGridWindow->addMouseMotionListener( xMouseMotionListener );
        m_xGridRowSelection->addSelectionListener( new GridSelectionListener( this ) );
        
        m_xWindowListener.set( new WindowListener( this ) );
        m_xParentWindow->addWindowListener( m_xWindowListener );
        
        m_xKeyHandler.set( new KeyHandler( this ) );
        m_xGridKeyHandler.set( new GridKeyHandler( this ) );
        
        updateButtonsState();
    }
    catch ( Exception & )
    {
    }
}

} // namespace ww
