
#include "listeners.hpp"
#include "view.hpp"
#include "defs.hpp"

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

namespace ww
{

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::awt::grid;

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::EventObject;

using ::rtl::OUString;


WindowListener::WindowListener( WWView * pView )
 : WindowListener_Base( m_aMutex )
 , m_pView( pView )
{
}

WindowListener::~WindowListener()
{
    m_pView = NULL;
}

void SAL_CALL WindowListener::windowResized( const WindowEvent& e ) throw (RuntimeException)
{
    if ( m_pView )
    {
        Reference< XWindow > xWindow( e.Source, UNO_QUERY );
        if ( xWindow.is() )
        {
            const Rectangle aRect = xWindow->getPosSize();
            m_pView->resizeWindow( aRect.Width, aRect.Height );
        }
    }
}

void SAL_CALL WindowListener::windowMoved( const WindowEvent& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL WindowListener::windowShown( const EventObject& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL WindowListener::windowHidden( const EventObject& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL WindowListener::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}


ActionListener::ActionListener( WWView * pView )
 : m_pView( pView )
 , ActionListener_Base( m_aMutex )
{
}

ActionListener::~ActionListener()
{
    m_pView = NULL;
}

void SAL_CALL ActionListener::actionPerformed( const ActionEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard const g( m_aMutex );
    
    (void)rEvent;
    if ( m_pView )
    {
        const OUString sCommand = rEvent.ActionCommand;
        if ( sCommand.getLength() > 0 )
            m_pView->executeCommand( (int)sCommand.getStr()[0] );
    }
}

void SAL_CALL ActionListener::disposing( const EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)Source;
}

void SAL_CALL ActionListener::disposing()
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( m_pView )
        m_pView = NULL;
}


MouseListener::MouseListener( WWView * pView )
 : MouseListener_Base( m_aMutex )
 , m_pView( pView )
 , m_bAlloDragging( false )
 , m_bDragging( false )
 , m_nRow( -1 )
{
}

MouseListener::~MouseListener()
{
    m_pView = NULL;
}

void SAL_CALL MouseListener::mouseEntered( const MouseEvent& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL MouseListener::mouseExited( const MouseEvent& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL MouseListener::mousePressed( const MouseEvent& e ) throw (RuntimeException)
{
    if ( e.Buttons == ::com::sun::star::awt::MouseButton::LEFT && e.ClickCount == 1 && m_pView )
    {
        if ( m_pView->startDragging( e.X, e.Y ) )
        {
            m_nRow = m_pView->getRowAt( e.X, e.Y );
            m_bAlloDragging = true;
        }
    }
    else if ( e.Buttons == ::com::sun::star::awt::MouseButton::RIGHT && e.ClickCount == 1 && m_pView )
    {
        m_pView->selectRow( e.X, e.Y, e.Modifiers );
        m_pView->executeContextMenu( e.X, e.Y );
    }
}

void SAL_CALL MouseListener::mouseReleased( const MouseEvent& e ) throw (RuntimeException)
{
    if ( e.Buttons == ::com::sun::star::awt::MouseButton::LEFT && e.ClickCount == 2 && m_pView )
        m_pView->executeCommand( CMD_GOTO );
    else if ( e.Buttons == ::com::sun::star::awt::MouseButton::LEFT && e.ClickCount == 1 && 
              m_bAlloDragging && m_pView)
    {
        m_bAlloDragging = false;
        if ( m_bDragging )
        {
            m_pView->setPointer( ::com::sun::star::awt::SystemPointer::ARROW );
            m_bDragging = false;
            m_pView->endDragging( e.X, e.Y );
            m_nRow = -1;
        }
    }
}

void SAL_CALL MouseListener::mouseDragged( const MouseEvent& e ) throw (RuntimeException)
{
    if ( m_bAlloDragging )
    {
        if ( !m_bDragging )
            m_pView->setPointer( ::com::sun::star::awt::SystemPointer::MOVE );
        m_bDragging = true;
        const int nRow = m_pView->getRowAt( e.X, e.Y );
        if ( nRow != m_nRow )
        {
            m_pView->drawDestination( nRow, m_nRow );
            m_nRow = nRow;
        }
    }
}

void SAL_CALL MouseListener::mouseMoved( const MouseEvent& e ) throw (RuntimeException)
{
    (void)e;
}


void SAL_CALL MouseListener::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}

void SAL_CALL MouseListener::disposing()
{
}


GridSelectionListener::GridSelectionListener( WWView * pView )
 : GridSelectionListener_Base( m_aMutex )
 , m_pView( pView )
{
}

GridSelectionListener::~GridSelectionListener()
{
    m_pView = NULL;
}


void SAL_CALL GridSelectionListener::selectionChanged( const GridSelectionEvent& gridSelectionEvent ) throw (RuntimeException)
{
    (void)gridSelectionEvent;
    if ( m_pView )
        m_pView->gridSelectionChanged();
}

void SAL_CALL GridSelectionListener::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}

void SAL_CALL GridSelectionListener::disposing()
{
}


FocusListener::FocusListener( WWView * pView, const WWView::ControlType nType )
 : FocusListener_Base( m_aMutex )
 , m_pView( pView )
 , m_nControlType( nType )
{
}

FocusListener::~FocusListener()
{
    m_pView = NULL;
}

void SAL_CALL FocusListener::focusGained( const FocusEvent& e ) throw (RuntimeException)
{
    (void)e;
    if ( m_pView )
        m_pView->controlFocusGained( m_nControlType );
}

void SAL_CALL FocusListener::focusLost( const FocusEvent& e ) throw (RuntimeException)
{
    (void)e;
    if ( m_pView )
        m_pView->controlFocusLost( m_nControlType );
}

void SAL_CALL FocusListener::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}

void SAL_CALL FocusListener::disposing()
{
}

KeyHandler::KeyHandler( WWView * pView )
 : KeyHandler_Base( m_aMutex )
 , m_pView( pView )
{
}

KeyHandler::~KeyHandler()
{
    m_pView = NULL;
}

::sal_Bool SAL_CALL KeyHandler::keyPressed( const KeyEvent& aEvent ) throw (RuntimeException)
{
    if ( aEvent.KeyCode == ::com::sun::star::awt::Key::RETURN && m_pView )
    {
        m_pView->updateRowFormula();
        return sal_True;
    }
    return sal_False;
}

::sal_Bool SAL_CALL KeyHandler::keyReleased( const KeyEvent& aEvent ) throw (RuntimeException)
{
    (void)aEvent;
    return sal_True;
}

void SAL_CALL KeyHandler::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}

void SAL_CALL KeyHandler::disposing()
{
}

GridKeyHandler::GridKeyHandler( WWView * pView )
 : GridKeyHandler_Base( m_aMutex )
 , m_pView( pView )
{
}

GridKeyHandler::~GridKeyHandler()
{
    m_pView = NULL;
}

::sal_Bool SAL_CALL GridKeyHandler::keyPressed( const KeyEvent& aEvent ) throw (RuntimeException)
{
    if ( !m_pView )
        return sal_False;
    int nCommand = 0;
    switch ( aEvent.KeyCode )
    {
        case ::com::sun::star::awt::Key::UP:
            nCommand = CMD_UP;
            break;
        case ::com::sun::star::awt::Key::DOWN:
            nCommand = CMD_DOWN;
            break;
        case ::com::sun::star::awt::Key::HOME:
            nCommand = CMD_HOME;
            break;
        case ::com::sun::star::awt::Key::END:
            nCommand = CMD_END;
            break;
        case ::com::sun::star::awt::Key::SPACE:
            nCommand = CMD_SPACE;
            break;
        case ::com::sun::star::awt::Key::DELETE:
            nCommand = CMD_REMOVE;
            break;
        case ::com::sun::star::awt::Key::RETURN:
            m_pView->executeCommand( CMD_GOTO );
            return sal_True;
            break;
        default:
            break;
    }
    if ( nCommand )
    {
        m_pView->executeGridCommand( nCommand, aEvent.Modifiers & 0b11 ); // ignore MOD2, 3
        return sal_True;
    }
    return sal_False;
}

::sal_Bool SAL_CALL GridKeyHandler::keyReleased( const KeyEvent& aEvent ) throw (RuntimeException)
{
    (void)aEvent;
    return sal_True;
}

void SAL_CALL GridKeyHandler::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void)Source;
}

void SAL_CALL GridKeyHandler::disposing()
{
}

} // namespace ww
