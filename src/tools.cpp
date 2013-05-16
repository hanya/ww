
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/resource/StringResourceWithLocation.hpp>


#define OUSTR( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )

namespace ww
{

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;


using ::rtl::OUString;


Reference< XInterface > getConfig( 
            const Reference< XComponentContext > & xContext, 
            const OUString & sNodeName ) throw ( RuntimeException )
{
    Reference< XInterface > xAccess;
    try
    {
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xMsf(
            xContext->getServiceManager()->createInstanceWithContext( 
                OUSTR( "com.sun.star.configuration.ConfigurationProvider" ), xContext ), UNO_QUERY );
        Sequence< Any > aArgs( 1 );
        aArgs[0] = makeAny(
                    PropertyValue( OUSTR( "nodepath" ), 0, 
                        makeAny( sNodeName ), 
                        ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) );
        xAccess.set( xMsf->createInstanceWithArguments(
            OUSTR( "com.sun.star.configuration.ConfigurationAccess" ), aArgs ), UNO_QUERY );
    }
    catch ( Exception & )
    {
    }
    return xAccess;
}


Reference< ::com::sun::star::resource::XStringResourceWithLocation > getResourceLoader(
        const Reference< XComponentContext > & xContext, 
        const OUString & rResDir, 
        const OUString & rFileName )
{
    Reference< ::com::sun::star::resource::XStringResourceWithLocation > xStringRes;
    ::com::sun::star::lang::Locale aLocale;
    try
    {
        Reference< XPropertySet > xPropSet( 
                getConfig( xContext, OUSTR( "/org.openoffice.Setup/L10N" ) ), UNO_QUERY_THROW );
        OUString aLoc;
        xPropSet->getPropertyValue( OUSTR( "ooLocale" ) ) >>= aLoc;
        sal_Int32 nIndex = 0;
        aLocale.Language = aLoc.getToken( 0, '-', nIndex );
        aLocale.Country = aLoc.getToken( 0, '-', nIndex );
        aLocale.Variant = aLoc.getToken( 0, '-', nIndex );
    }
    catch ( Exception & )
    { }
    try
    {
        xStringRes.set( 
            ::com::sun::star::resource::StringResourceWithLocation::create( 
                    xContext, 
                    rResDir, sal_True, aLocale, rFileName, 
                    OUString(), 
                    Reference< ::com::sun::star::task::XInteractionHandler >() ) );
    }
    catch ( Exception & )
    { }
    return xStringRes;
}


} // namespace ww
