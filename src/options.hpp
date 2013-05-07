
#ifndef _OPTIONS_HPP_
#define _OPTIONS_HPP_

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


namespace ww
{

typedef ::cppu::WeakComponentImplHelper2< 
                    ::com::sun::star::awt::XContainerWindowEventHandler, 
                    ::com::sun::star::lang::XServiceInfo
            > WWOptions_Base;


class WWOptions : protected ::cppu::BaseMutex, 
                  public WWOptions_Base
{
public : 
    
    WWOptions( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&xComponentContext );
    ~WWOptions();
    
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xComponentContext);
    
    // XContainerWindowEventHandler
    virtual ::sal_Bool SAL_CALL callHandlerMethod( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xWindow, const ::com::sun::star::uno::Any& EventObject, const ::rtl::OUString& MethodName ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedMethodNames(  ) throw (::com::sun::star::uno::RuntimeException);
    
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
    
    static ::rtl::OUString SAL_CALL getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();

private :
    
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    
    ::sal_Bool loadData( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & rWindow, const ::rtl::OUString & rEventName );
    ::sal_Bool saveData( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & rWindow, const ::rtl::OUString & rEventName );
    
    bool m_bUseInputLine;
    //bool m_bStoreWatches;
    //int m_nWarnCells;
};


} // namespace ww

#endif
