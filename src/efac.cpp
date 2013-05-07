
#include "efac.hpp"
#include "defs.hpp"
#include "element.hpp"

#include <com/sun/star/awt/XWindow.hpp>

#define A2S( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )

namespace ww
{

using ::com::sun::star::awt::XWindow;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::ui::XUIElement;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::frame::XFrame;

using ::rtl::OUString;


WWUIElementFactory::WWUIElementFactory( const Reference< XComponentContext > & xContext )
 : WWUIElementFactory_Base( m_aMutex )
 , m_xContext( xContext )
{
}


WWUIElementFactory::~WWUIElementFactory()
{
}


Reference< XInterface > WWUIElementFactory::create( 
            const Reference< XComponentContext > & xContext )
{
    return static_cast< ::cppu::OWeakObject* >( new WWUIElementFactory( xContext ) );
}


// XUIElementFactory
Reference< XUIElement > SAL_CALL WWUIElementFactory::createUIElement(
            const OUString & rResourceURL, 
            const Sequence< PropertyValue > & aArgs) 
            throw (RuntimeException, NoSuchElementException, IllegalArgumentException)
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( !rResourceURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( RESOURCE_NAME ) ) )
        throw NoSuchElementException();
    
    Reference< XFrame > xFrame;
    Reference< XWindow > xParentWindow;
    
    const PropertyValue * pArgs = aArgs.getConstArray();
    const PropertyValue * pEndArgs = aArgs.getConstArray() + aArgs.getLength();
    for ( ; pArgs < pEndArgs; ++pArgs )
    {
        if ( pArgs->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Frame" ) ) )
            xFrame.set( pArgs->Value, UNO_QUERY );
        else if ( pArgs->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ParentWindow" ) ) )
            xParentWindow.set( pArgs->Value, UNO_QUERY );
    }
    if ( !xFrame.is() )
        throw IllegalArgumentException();
    if ( !xParentWindow.is() )
        throw IllegalArgumentException();
    Reference< XUIElement > xElement( new WWUIElement( m_xContext, xFrame, xParentWindow ) );
    return xElement;
}


// XServiceInfo
OUString SAL_CALL WWUIElementFactory::getImplementationName(  ) throw (RuntimeException)
{
    return WWUIElementFactory::getImplementationName_Static();
}

::sal_Bool SAL_CALL WWUIElementFactory::supportsService( const ::rtl::OUString& ServiceName ) 
        throw (RuntimeException)
{
    return ( ServiceName.equals( A2S( SERVICE_NAME ) ) );
}

Sequence< OUString > SAL_CALL WWUIElementFactory::getSupportedServiceNames(  ) 
        throw (RuntimeException)
{
    return WWUIElementFactory::getSupportedServiceNames_Static();
}


OUString WWUIElementFactory::getImplementationName_Static() 
{
    return OUString::createFromAscii( IMPLE_NAME );
}

Sequence< OUString > WWUIElementFactory::getSupportedServiceNames_Static() 
{
    Sequence< OUString > aRet( 1 );
    OUString* pArray = aRet.getArray();
    pArray[0] = A2S( SERVICE_NAME );
    return aRet;
}

} // namespace ww
