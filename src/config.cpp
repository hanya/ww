
#include "config.hpp"
#include "defs.hpp"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#define OUSTR( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )

namespace ww
{

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


Config::Config( const Reference< XComponentContext > & xContext, const bool bWritable )
{
    try
    {
        Reference< XMultiComponentFactory > xMcf( 
                        xContext->getServiceManager(), UNO_SET_THROW );
        Reference< XMultiServiceFactory > xMsf(
            xMcf->createInstanceWithContext( 
                OUSTR( "com.sun.star.configuration.ConfigurationProvider" ), xContext), UNO_QUERY_THROW );
        Sequence< Any > aConfigArgs( 1 );
        aConfigArgs[0] = makeAny( PropertyValue( 
                OUSTR( "nodepath" ), 
                0, 
                makeAny( OUSTR( CONFIG_NODE ) ), 
                ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) );
        OUString sServiceName;
        if ( bWritable )
            sServiceName = OUSTR( "com.sun.star.configuration.ConfigurationUpdateAccess" );
        else
            sServiceName = OUSTR( "com.sun.star.configuration.ConfigurationAccess" );
        m_xPropSet.set( 
                xMsf->createInstanceWithArguments( 
                    sServiceName, aConfigArgs ), UNO_QUERY_THROW );
    }
    catch ( Exception & )
    {
    }
}

Config::~Config()
{
}


ConfigReader::ConfigReader( const Reference< XComponentContext > & xContext )
 : Config( xContext, false )
{

}

ConfigReader::~ConfigReader()
{
}

sal_Bool ConfigReader::getBoolValue( const OUString & rName ) throw ( UnknownPropertyException, RuntimeException )
{
    if ( m_xPropSet.is() )
    {
        sal_Bool b;
        m_xPropSet->getPropertyValue( rName ) >>= b;
        return b;
    }
    throw RuntimeException();
}

sal_Int32 ConfigReader::getIntValue( const OUString & rName ) 
                        throw ( UnknownPropertyException, RuntimeException )
{
    if ( m_xPropSet.is() )
    {
        sal_Int32 n;
        m_xPropSet->getPropertyValue( rName ) >>= n;
        return n;
    }
    throw RuntimeException();
}


ConfigWriter::ConfigWriter( const Reference< XComponentContext > & xContext )
 : Config( xContext, true )
{
}

ConfigWriter::~ConfigWriter()
{
}


void ConfigWriter::setBoolValue( const OUString & rName, const sal_Bool bState ) 
                    throw ( UnknownPropertyException, RuntimeException )
{
    if ( !m_xPropSet.is() )
        throw RuntimeException();
    m_xPropSet->setPropertyValue( rName, makeAny( bState ) );
}

void ConfigWriter::setIntValue( const OUString & rName, const sal_Int32 nValue ) 
                    throw ( UnknownPropertyException, RuntimeException )
{
    if ( !m_xPropSet.is() )
        throw RuntimeException();
    m_xPropSet->setPropertyValue( rName, makeAny( nValue ) );
}


void ConfigWriter::Commit() throw ( RuntimeException )
{
    Reference< ::com::sun::star::util::XChangesBatch > xChangesBatch( m_xPropSet, UNO_QUERY_THROW );
    xChangesBatch->commitChanges();
}

} // namespace ww
