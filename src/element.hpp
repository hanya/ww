
#ifndef _ELEMENT_HPP_
#define _ELEMENT_HPP_

#include <cppuhelper/basemutex.hxx>

#ifndef TASKPANE
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/ui/XSidebarPanel.hpp>
#else
#include <cppuhelper/compbase2.hxx>
#endif

#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace ww
{

class WWModel;
class WWView;


#ifndef TASKPANE
typedef ::cppu::WeakComponentImplHelper3< 
              ::com::sun::star::ui::XUIElement
            , ::com::sun::star::ui::XToolPanel
            , ::com::sun::star::ui::XSidebarPanel
        > WWUIElement_Base;
#else
typedef ::cppu::WeakComponentImplHelper2< 
              ::com::sun::star::ui::XUIElement
            , ::com::sun::star::ui::XToolPanel
        > WWUIElement_Base;
#endif

class WWUIElement : protected ::cppu::BaseMutex, 
                  public WWUIElement_Base
{
public:
    
    WWUIElement( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, 
               const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame, 
               const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xParentWindow );
    virtual ~WWUIElement();
    
    // XUIElement Attributes
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);

    // XUIElement Methods
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException);
    
    // XTooPanel Attributes
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getWindow() throw (::com::sun::star::uno::RuntimeException);

    // XTooPanel Methods
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL createAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& ParentAccessible ) throw (::com::sun::star::uno::RuntimeException);
    
#ifndef TASKPANE
    // XSidebarPanel @since AOO4
    virtual ::com::sun::star::ui::LayoutSize SAL_CALL getHeightForWidth( ::sal_Int32 nWidth ) throw (::com::sun::star::uno::RuntimeException);
#endif

    virtual void disposing();
private:
    
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    
    WWModel * m_pModel;
    WWView * m_pView;
};

} // namespace ww

#endif
