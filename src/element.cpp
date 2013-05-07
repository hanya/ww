
#include "element.hpp"
#include "defs.hpp"
#include "model.hpp"
#include "view.hpp"

#include <com/sun/star/ui/UIElementType.hpp>

namespace ww
{

using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;

using ::rtl::OUString;


WWUIElement::WWUIElement( const Reference< XComponentContext > & xContext, 
                      const Reference< XFrame > & xFrame, 
                      const Reference< XWindow > & xParentWindow )
 : WWUIElement_Base( m_aMutex )
 , m_xFrame( xFrame )
 , m_pModel( new WWModel( xContext ) )
 , m_pView( new WWView( xContext, xFrame, xParentWindow ) )
{
    m_pView->setModel( m_pModel );
    m_pModel->setView( m_pView );
}


WWUIElement::~WWUIElement()
{
}

void WWUIElement::disposing()
{
    if ( m_pView )
        m_pView->disconnectModel();
    if ( m_pModel )
        m_pModel->disconnectView();
    delete m_pView;
    delete m_pModel;
}


Reference< XFrame > SAL_CALL WWUIElement::getFrame() throw (RuntimeException)
{
    ::osl::MutexGuard const g( m_aMutex );
    if ( !m_xFrame.is() )
        throw DisposedException();
    return m_xFrame;
}


::rtl::OUString SAL_CALL WWUIElement::getResourceURL() throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( RESOURCE_NAME ) );
}


::sal_Int16 SAL_CALL WWUIElement::getType() throw (RuntimeException)
{
    return ::com::sun::star::ui::UIElementType::TOOLPANEL;
}


Reference< XInterface > SAL_CALL WWUIElement::getRealInterface(  ) throw (RuntimeException)
{
    ::osl::MutexGuard const g( m_aMutex );
    return static_cast< ::cppu::OWeakObject * >( this );
}

// XTooPanel
Reference< XWindow > SAL_CALL WWUIElement::getWindow() 
    throw (RuntimeException)
{
    if ( !m_pView )
        throw DisposedException();
    return m_pView->getWindow();
}


Reference< XAccessible > SAL_CALL WWUIElement::createAccessible( const Reference< XAccessible >& ParentAccessible ) 
    throw (RuntimeException)
{
}


#ifndef TASKPANE
// XSidebarPanel
::com::sun::star::ui::LayoutSize SAL_CALL WWUIElement::getHeightForWidth( ::sal_Int32 nWidth ) throw (RuntimeException)
{
    return ::com::sun::star::ui::LayoutSize( 0, -1, 0 );
}
#endif


} // namespace ww
