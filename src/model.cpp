
#include "model.hpp"
#include "view.hpp"
#include "defs.hpp"

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>

namespace ww
{

using namespace ::com::sun::star::sheet;

using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::table::XCell;
using ::com::sun::star::sheet::XSheetCellRange;
using ::com::sun::star::sheet::XSheetCellRanges;
using ::com::sun::star::sheet::XCellRangeAddressable;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::lang::XMultiServiceFactory;

using ::rtl::OUString;

#define A2S( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )


WWModel::WWModel( const Reference< XComponentContext > & xContext )
 : m_xContext( xContext )
 , m_pView( NULL )
 , m_pRows( NULL )
{
}


WWModel::~WWModel()
{
    m_pView = NULL;
    if ( m_pRows )
        delete m_pRows;
}


void WWModel::setView( WWView * pView )
{
    m_pView = pView;
    if ( m_pView )
    {
        m_pRows = new Rows( m_pView->getGridDataModel() );
        if ( !m_pRows->hasLabels() )
            m_pRows->setLabels( pView->getLabels() );
    }
}


void WWModel::disconnectView()
{
    m_pView = NULL;
}


int WWModel::getRowCount() const
{
    if ( m_pRows )
        return m_pRows->getWatchCount();
    return 0;
}

void WWModel::addEntry()
{
    static const OUString sSheetCell = A2S( "com.sun.star.sheet.SheetCell" );
    static const OUString sSheetCellRange = A2S( "com.sun.star.sheet.SheetCellRange" );
    static const OUString sSheetCellRanges = A2S( "com.sun.star.sheet.SheetCellRanges" );
    
    if ( m_pView )
    {
        Reference< XInterface > xSelection( m_pView->getSelection() );
        Reference< XServiceInfo > xServiceInfo( xSelection, UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            if ( xServiceInfo->supportsService( sSheetCell ) )
            {
                Reference< XCell > xCell( xSelection, UNO_QUERY );
                addCell( xCell );
            }
            else if ( xServiceInfo->supportsService( sSheetCellRange ) )
            {
                Reference< XSheetCellRange > xSheetCellRange( xSelection, UNO_QUERY );
                addCellRange( xSheetCellRange );
            }
            else if ( xServiceInfo->supportsService( sSheetCellRanges ) )
            {
                Reference< XSheetCellRanges > xSheetCellRanges( xSelection, UNO_QUERY );
                addCellRanges( xSheetCellRanges );
            }
        }
    }
}

void WWModel::addCell( const Reference< XCell > & rxCell )
{
    if ( m_pRows )
        m_pRows->addWatch( rxCell );
}

void WWModel::addCellRange( const Reference< XSheetCellRange > & xSheetCellRange )
{
    if ( !xSheetCellRange.is() )
        return;
    try
    {
        Reference< XCellRangeAddressable > xCellRangeAddressable( xSheetCellRange, UNO_QUERY_THROW );
        const CellRangeAddress aAddress = xCellRangeAddressable->getRangeAddress();
        const sal_Int32 nColumns = aAddress.EndColumn - aAddress.StartColumn;
        const sal_Int32 nRows = aAddress.EndRow - aAddress.StartRow;
        if ( ( nColumns + 1 ) * ( nRows + 1 ) > MAX_ADD_WATCHES )
            return;
        
        for ( sal_Int32 nColumn = 0; nColumn <= nColumns; ++nColumn )
        {
            for ( sal_Int32 nRow = 0; nRow <= nRows; ++nRow )
                addCell( xSheetCellRange->getCellByPosition( nColumn, nRow ) );
        }
    }
    catch ( Exception & )
    {
    }
}

void WWModel::addCellRanges( const Reference< XSheetCellRanges > & xSheetCellRanges )
{
    if ( !xSheetCellRanges.is() )
        return;
    try
    {
        const Sequence< CellRangeAddress > aAddresses = xSheetCellRanges->getRangeAddresses();
        const CellRangeAddress * pAddresses = aAddresses.getConstArray();
        const CellRangeAddress * pAddressesEnd = aAddresses.getConstArray() + aAddresses.getLength();
        sal_Int32 nCells = 0;
        for ( ; pAddresses < pAddressesEnd; ++pAddresses )
            nCells += ( pAddresses->EndColumn - pAddresses->StartColumn + 1 ) * 
                      ( pAddresses->EndRow - pAddresses->StartRow + 1 );
        if ( nCells > MAX_ADD_WATCHES )
            return;
        
        Reference< XEnumerationAccess > xEnumerationAcc( xSheetCellRanges, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAcc->createEnumeration(), UNO_SET_THROW );
        while ( xEnumeration->hasMoreElements() )
        {
            Reference< XSheetCellRange > xSheetCellRange( xEnumeration->nextElement(), UNO_QUERY );
            if ( xSheetCellRange.is() )
                addCellRange( xSheetCellRange );
        }
    }
    catch ( Exception & )
    {
    }
}


void WWModel::removeEntries( const Sequence< sal_Int32 > & rIndex )
{
    if ( m_pRows )
        m_pRows->removeWatches( rIndex );
}


void WWModel::removeAllEntries()
{
    if ( m_pRows )
        m_pRows->removeAllWatches();
}


void WWModel::updateAll()
{
    if ( m_pRows )
        m_pRows->updateAll();
}

void WWModel::moveEntries( const sal_Int32 nIndex, const Sequence< sal_Int32 > & aIndex )
{
    if ( m_pRows )
        m_pRows->moveWatches( nIndex, aIndex );
}


OUString WWModel::getAddress( const sal_Int32 nIndex ) const
{
    if ( m_pRows )
        return m_pRows->getRowAddress( nIndex );
    return OUString();
}

OUString WWModel::getFormula( const ::sal_Int32 nIndex ) const
{
    if ( m_pRows )
        return m_pRows->getRowFormula( nIndex );
    return OUString();
}

void WWModel::setFormula( const sal_Int32 nIndex, const ::rtl::OUString & rFormula ) const
{
    if ( m_pRows )
        m_pRows->setRowFormula( nIndex, rFormula );
}


Sequence< OUString > WWModel::getCellReferences( const sal_Int32 nIndex ) const
{
    // ToDo sorted?
    Sequence< OUString > aRefs;
    if ( m_pRows && m_pView )
    {
        try
        {
            Sequence< CellRangeAddress > aAddresses( m_pRows->getReferenceAddresses( nIndex ) );
            const sal_Int32 nLength = aAddresses.getLength();
            if ( nLength )
            {
                Reference< XMultiServiceFactory > xMsf( m_pView->getDocument(), UNO_QUERY_THROW );
                Reference< XSheetCellRangeContainer > xSheetCellRangeContainer(
                        xMsf->createInstance( A2S( "com.sun.star.sheet.SheetCellRanges" ) ), UNO_QUERY_THROW );
                
                xSheetCellRangeContainer->addRangeAddresses( aAddresses, sal_False );
                const OUString sAddress = xSheetCellRangeContainer->getRangeAddressesAsString();
                
                aRefs.realloc( nLength );
                OUString * pRefs = aRefs.getArray();
                sal_Int32 nBegin = 0;
                sal_Int32 nPos = sAddress.indexOfAsciiL( ";", 1, 0 );
                sal_Int32 i = 0;
                for ( ; i < nLength -1; ++i )
                {
                    pRefs[i] = sAddress.copy( nBegin, nPos - nBegin );
                    nBegin = nPos + 1;
                    nPos = sAddress.indexOfAsciiL( ";", 1, nBegin );
                }
                pRefs[i] = sAddress.copy( nBegin );
            }
        }
        catch ( Exception & )
        {
        }
    }
    return aRefs;
}


} // namespace ww
