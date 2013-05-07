
#ifndef _EFAC_HPP_
#define _EFAC_HPP_

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>

namespace ww
{

typedef ::cppu::WeakComponentImplHelper2< 
            ::com::sun::star::ui::XUIElementFactory, 
            ::com::sun::star::lang::XServiceInfo
            > WWUIElementFactory_Base;

class WWUIElementFactory : protected ::cppu::BaseMutex, 
                         public WWUIElementFactory_Base
{
public:
    
    WWUIElementFactory( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext );
    ~WWUIElementFactory();
    
    // XUIElementFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL createUIElement(const ::rtl::OUString & ResourceURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & Args) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException);
    
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
    
    static ::rtl::OUString SAL_CALL getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static();
    
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext);

private:
    
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    
};

} // namespace ww

#endif
