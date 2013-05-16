
#include "row.hpp"

#include <rtl/ustrbuf.hxx>
#include <algorithm>

#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>

#define A2S( str ) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( str ) )

namespace ww
{

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::awt::grid::XMutableGridDataModel;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::util::XModifyBroadcaster;

using ::rtl::OUString;


ModifyListener::ModifyListener( Row * pRow )
 : ModifyListener_Base( m_aMutex )
 , m_pRow( pRow )
{
}

ModifyListener::~ModifyListener()
{
    m_pRow = NULL;
}

void SAL_CALL ModifyListener::modified( const EventObject& /* aEvent */ ) throw (RuntimeException)
{
    if ( m_pRow )
        m_pRow->modified();
}

void SAL_CALL ModifyListener::disposing( const EventObject& /* Source */ ) throw (RuntimeException)
{
    if ( m_pRow )
        m_pRow->listenerDisposing();
}

void SAL_CALL ModifyListener::disposing()
{
    m_pRow = NULL;
}


OUString Row::m_sSheetLabel;
OUString Row::m_sCellLabel;
OUString Row::m_sValueLabel;
OUString Row::m_sFormulaLabel;


Row::Row( Rows * pRows, const Reference< XCell > & xCell )
 : m_pRows( pRows )
 , m_xCell( xCell )
 , m_nIndex( -1 )
 , m_bWatching( false )
{
    m_xModifyListener.set( new ModifyListener( this ) );
}

Row::~Row()
{
    m_pRows = NULL;
    if ( m_bWatching )
        enableWatching( false );
    if ( m_xModifyListener.is() )
        m_xModifyListener.clear();
    if ( m_xCell.is() )
        m_xCell.clear();
}


bool Row::hasLabels()
{
    return Row::m_sSheetLabel.getLength() != 0;
}

void Row::setLabels( const Sequence< OUString > & aRes )
{
    if ( Row::m_sSheetLabel.getLength() == 0 && aRes.getLength() == 4 )
    {
        const OUString * pRes = aRes.getConstArray();
        Row::m_sSheetLabel   = pRes[0];
        Row::m_sCellLabel    = pRes[1];
        Row::m_sValueLabel   = pRes[2];
        Row::m_sFormulaLabel = pRes[3];
    }
}


void Row::listenerDisposing()
{
    enableWatching( false );
    m_xModifyListener.clear();
}


void Row::modified()
{
    if ( m_pRows )
        m_pRows->updateRow( this );
}


void Row::enableWatching( const bool bState )
{
    const Reference< XModifyBroadcaster > xModifyBroadcaster( m_xCell, UNO_QUERY );
    if ( xModifyBroadcaster.is() )
    {
        m_bWatching = bState;
        if ( bState )
            xModifyBroadcaster->addModifyListener( m_xModifyListener );
        else
            xModifyBroadcaster->removeModifyListener( m_xModifyListener );
    }
}


::rtl::OUString Row::getAddress() const
{
    static const OUString sAbsName = A2S( "AbsoluteName" );
    
    Reference< XPropertySet > xPropSet( m_xCell, UNO_QUERY );
    if ( xPropSet.is() )
    {
        OUString s;
        xPropSet->getPropertyValue( sAbsName ) >>= s;
        return s;
    }
    return OUString();
}

::rtl::OUString Row::getFormula() const
{
    static const OUString sFormulaLocal = A2S( "FormulaLocal" );
    
    Reference< XPropertySet > xPropSet( m_xCell, UNO_QUERY );
    if ( xPropSet.is() )
    {
        OUString s;
        xPropSet->getPropertyValue( sFormulaLocal ) >>= s;
        return s;
    }
    return OUString();
}

void Row::setFormula( const OUString & rFormula ) const
{
    static const OUString sFormulaLocal = A2S( "FormulaLocal" );
    
    Reference< XPropertySet > xPropSet( m_xCell, UNO_QUERY );
    if ( xPropSet.is() )
        xPropSet->setPropertyValue( sFormulaLocal, makeAny( rFormula ) );
}


OUString Row::getTooltipFromData( const OUString & rSheet, const OUString & rCell, 
                    const OUString & rValue, const OUString & rFormula ) const
{
    // ToDo RTL
    static const sal_Unicode sSuffix[] = {':', ' '};
    static const sal_Unicode sLf[] = {'\n'};
    
    ::rtl::OUStringBuffer buff;
    return 
        buff.append( m_sSheetLabel ).append( sSuffix, 2 ).append( rSheet ).append( sLf, 1 )
            .append( m_sCellLabel  ).append( sSuffix, 2 ).append( rCell  ).append( sLf, 1 )
            .append( m_sValueLabel ).append( sSuffix, 2 ).append( rValue ).append( sLf, 1 )
            .append( m_sFormulaLabel ).append( sSuffix, 2 ).append( rFormula )
            .makeStringAndClear();
}


Sequence< Any > Row::getData( Any & rOutTooltip ) const
{
    static const OUString sAbsName = A2S( "AbsoluteName" );
    static const OUString sFormulaName = A2S( "FormulaLocal" );
    static const OUString sQuote = A2S( "'" );
    static const OUString sQuotes = A2S( "''" );
    static const OUString sDot = A2S( "." );
    static const OUString sAbsPos = A2S( "$" );
    static const OUString sEmpty = OUString();
    
    if ( m_xCell.is() )
    {
        Reference< XTextRange > xTextRange( m_xCell, UNO_QUERY );
        Reference< XPropertySet > xPropSet( m_xCell, UNO_QUERY );
        if ( xPropSet.is() && xTextRange.is() )
        {
            Sequence< Any > aData( 4 );
            Any * pData = aData.getArray();
            
            OUString sHeader;
            xPropSet->getPropertyValue( sAbsName ) >>= sHeader;
            
            // example: $'Sheet.1''aa'.$A$1
            const sal_Int32 nPos = sHeader.lastIndexOf( sDot );
            
            // ignore first $ and replace second $ with empty string
            const OUString sCellAddr = sHeader.copy( nPos + 2 ).replaceAt( 
                    sHeader.indexOf( sAbsPos, nPos + 2 ) - nPos - 2, 1, sEmpty );
            
            // ignore first $ and last .
            OUString sSheetName = sHeader.copy( 1, nPos - 1 );
            if ( sSheetName.match( sQuote, sSheetName.getLength() - 1 ) && 
                 sSheetName.match( sQuote, 0 ) )
            {
                // replace '' with ' and remove first and last ' if there
                sSheetName = sSheetName.copy( 1, sSheetName.getLength() - 2 );
                sal_Int32 nPos = sSheetName.indexOf( sQuotes, 0 );
                while ( nPos > 0 ) // starting with ' is illegal
                {
                    sSheetName = sSheetName.replaceAt( nPos, 2, sQuote );
                    nPos = sSheetName.indexOf( sQuotes, nPos + 1 );
                }
            }
            
            OUString sFormula;
            if ( m_xCell->getType() == ::com::sun::star::table::CellContentType_FORMULA )
                xPropSet->getPropertyValue( sFormulaName ) >>= sFormula;
            
            const OUString sValue = xTextRange->getString();
            pData[0] <<= sSheetName;
            pData[1] <<= sCellAddr;
            pData[2] <<= sValue;
            pData[3] <<= sFormula;
            rOutTooltip <<= getTooltipFromData( sSheetName, sCellAddr, sValue, sFormula );
            return aData;
        }
    }
    return Sequence< Any >();
}


Sequence< FormulaToken > Row::getTokens() const
{
    const Reference< XFormulaTokens > xFormulaTokens( m_xCell, UNO_QUERY );
    if ( xFormulaTokens.is() )
        return xFormulaTokens->getTokens();
    return Sequence< FormulaToken >();
}

#ifdef SINGLE_WATCHING
sal_Int32 Row::getIntegerAddress() const
{
    Reference< XCellAddressable > xCellAddr( m_xCell, UNO_QUERY_THROW );
    if ( xCellAddr.is() )
    {
        CellAddress aAddr = xCellAddr->getCellAddress();
        return ( aAddr.Sheet << 30 ) | ( aAddr.Row << 20 ) | aAddr.Column;
    }
    return -1;
}
#endif


Rows::Rows( const Reference< XMutableGridDataModel > & xGridDataModel )
 : m_xGridDataModel( xGridDataModel )
{
    m_aIndexes.realloc( 4 );
    sal_Int32 * pIndexes = m_aIndexes.getArray();
    pIndexes[0] = 0;
    pIndexes[1] = 1;
    pIndexes[2] = 2;
    pIndexes[3] = 3;
}


Rows::~Rows()
{
#ifdef SINGLE_WATCHING
    m_aAddresses.clear();
#endif
    RowVector::reverse_iterator rit = m_aRows.rbegin();
    const RowVector::reverse_iterator rite = m_aRows.rend();
    while ( rit != rite )
    {
        (*rit)->enableWatching( false );
        delete *rit;
        ++rit;
    }
    m_aRows.clear();
}

bool Rows::hasLabels() const
{
    return Row::hasLabels();
}

void Rows::setLabels( const Sequence< OUString > & aRes )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    Row::setLabels( aRes );
}

int Rows::getWatchCount() const
{
    return m_aRows.size();
}


void Rows::updateRow( Row * pRow ) const
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( pRow && m_xGridDataModel.is() )
    {
        Any aTooltip;
        const Sequence< Any > aData = pRow->getData( aTooltip );
        if ( aData.getLength() == 4 )
        {
            try
            {
                const sal_Int32 nIndex = (sal_Int32)pRow->getIndex();
                m_xGridDataModel->updateRowData( m_aIndexes, nIndex, aData );
                m_xGridDataModel->updateRowToolTip( nIndex, aTooltip );
            }
            catch ( Exception & )
            {
            }
        }
    }
}


void Rows::updateAll()
{
    RowVector::iterator it = m_aRows.begin();
    const RowVector::iterator ite = m_aRows.end();
    for ( ; it < ite; ++it )
        updateRow( *it );
}


void Rows::addWatch( const Reference< XCell > & rxCell ) throw ( IllegalArgumentException )
{
    ::osl::MutexGuard const g( m_aMutex );
    try
    {
#ifdef SINGLE_WATCHING
        Reference< XCellAddressable > xCellAddr( rxCell, UNO_QUERY_THROW );
        const CellAddress aAddr = xCellAddr->getCellAddress();
        const sal_Int32 nAddr = ( aAddr.Sheet << 30 ) | ( aAddr.Row << 20 ) | aAddr.Column;
        AddressHash::iterator ith = m_aAddresses.find( nAddr );
        if ( ith == m_aAddresses.end() )
#endif
        {
            Row * pRow = new Row( this, rxCell );
            m_aRows.push_back( pRow );
            const int nIndex = m_aRows.size() - 1;
            pRow->setIndex( nIndex );
#ifdef SINGLE_WATCHING
            m_aAddresses.insert( ::std::pair< sal_Int32, bool >( nAddr, true ) );
#endif
            insertRow( nIndex, pRow );
            pRow->enableWatching( true );
        }
    }
    catch ( Exception & )
    {
        throw IllegalArgumentException();
    }
}


void Rows::insertRow( const int nIndex, Row * pRow )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( pRow && m_xGridDataModel.is() )
    {
        Any aTooltip;
        const Sequence< Any > aData = pRow->getData( aTooltip );
        if ( aData.getLength() == 4 )
        {
            try
            {
                const sal_Int32 nIndex = (sal_Int32)pRow->getIndex();
                m_xGridDataModel->insertRow( 
                        nIndex, makeAny( OUString() ), aData );
                m_xGridDataModel->updateRowToolTip( nIndex, aTooltip );
            }
            catch ( Exception & )
            {
            }
        }
    }
}


void Rows::reassignIndex( const int nStart )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    int nIndex = nStart;
    RowVector::iterator it = m_aRows.begin() + nIndex;
    const RowVector::iterator endit = m_aRows.end();
    for ( ; it < endit; ++it, ++nIndex )
        (*it)->setIndex( nIndex );
}

typedef ::std::vector< sal_Int32 > IntVector;

void lcl_sortSequence( const Sequence< sal_Int32 > & rIndex, IntVector & aSortedIndex )
{
    const int nLength = rIndex.getLength();
    aSortedIndex.resize( nLength );
    
    const sal_Int32 * pIndex = rIndex.getConstArray();
    for ( int n = 0; n < nLength; n++ )
        aSortedIndex[n] = pIndex[n];
    ::std::sort( aSortedIndex.begin(), aSortedIndex.end() );
}


void Rows::removeWatches( const Sequence< sal_Int32 > & rIndex )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( rIndex.getLength() )
    {
        IntVector aSortedIndex;
        lcl_sortSequence( rIndex, aSortedIndex );
        // ToDo check first item is >= 0 and last item < size
        int nRemovedRows = 0;
        IntVector::reverse_iterator rit = aSortedIndex.rbegin();
        const IntVector::reverse_iterator rite = aSortedIndex.rend();
        for ( ; rit != rite; ++rit )
        {
            const int nIndex = (int)*rit;
            if ( 0 <= nIndex && nIndex <= m_aRows.size() )
            {
                Row * pRow = m_aRows[nIndex];
                if ( pRow )
                {
                    pRow->enableWatching( false );
#ifdef SINGLE_WATCHING
                    AddressHash::iterator ith = m_aAddresses.find( 
                                            pRow->getIntegerAddress() );
                    if ( ith != m_aAddresses.end() )
                        m_aAddresses.erase( ith );
#endif
                    delete pRow;
                }
                m_aRows.erase( m_aRows.begin() + nIndex );
                m_xGridDataModel->removeRow( (sal_Int32)nIndex );
                ++nRemovedRows;
            }
        }
        if ( nRemovedRows )
            reassignIndex( aSortedIndex[0] );
    }
}


void Rows::removeAllWatches()
{
    ::osl::MutexGuard const g( m_aMutex );
    
    if ( m_aRows.size() )
    {
        RowVector::iterator it = m_aRows.begin();
        const RowVector::iterator ite = m_aRows.end();
        for ( ; it < ite; ++it )
            delete *it;
        m_aRows.clear();
#ifdef SINGLE_WATCHING
        m_aAddresses.clear();
#endif
        m_xGridDataModel->removeAllRows();
    }
}

void Rows::moveWatches( const sal_Int32 nDest, const Sequence< sal_Int32 > & rIndex )
{
    ::osl::MutexGuard const g( m_aMutex );
    
    const int nLength = rIndex.getLength();
    if ( nLength && 0 <= nDest && nDest <= m_aRows.size() )
    {
        int nPosTransition = nDest;
        IntVector aSortedIndex;
        lcl_sortSequence( rIndex, aSortedIndex );
        // ToDo check out of range or duplicated indexes
        RowVector aRows( nLength );
        {
            int n = 0;
            IntVector::reverse_iterator rit = aSortedIndex.rbegin();
            const IntVector::reverse_iterator rite = aSortedIndex.rend();
            for ( ; rit != rite; ++rit )
            {
                const int nIndex = (int)*rit;
                if ( 0 <= nIndex && nIndex <= m_aRows.size() )
                {
                    if ( nIndex <= nDest )
                        nPosTransition--;
                    Row * pRow = m_aRows[nIndex];
                    if ( pRow )
                        aRows[n++] = pRow; // reverse order
                    m_aRows.erase( m_aRows.begin() + nIndex );
                    m_xGridDataModel->removeRow( (sal_Int32)nIndex );
                }
            }
        }
        RowVector::iterator i = m_aRows.begin() + nPosTransition;
        RowVector::reverse_iterator rit = aRows.rbegin();
        const RowVector::reverse_iterator rite = aRows.rend();
        for ( int nPos = nPosTransition; rit != rite; ++rit, ++i, ++nPos )
        {
            m_aRows.insert( i, *rit );
            insertRow( nPos, *rit );
        }
        reassignIndex( nPosTransition < aSortedIndex[0] ? nPosTransition : aSortedIndex[0] );
    }
}




OUString Rows::getRowAddress( const sal_Int32 nIndex ) const
{
    if ( nIndex >= 0 && nIndex < m_aRows.size() )
    {
        const Row * pRow = m_aRows[(int)nIndex];
        if ( pRow )
            return pRow->getAddress();
    }
    return OUString();
}

OUString Rows::getRowFormula( const sal_Int32 nIndex ) const
{
    if ( nIndex >= 0 && nIndex < m_aRows.size() )
    {
        const Row * pRow = m_aRows[(int)nIndex];
        if ( pRow )
            return pRow->getFormula();
    }
    return OUString();
}

void Rows::setRowFormula( const sal_Int32 nIndex, const OUString & rFormula ) const
{
    if ( nIndex >= 0 && nIndex < m_aRows.size() )
    {
        const Row * pRow = m_aRows[(int)nIndex];
        if ( pRow )
            return pRow->setFormula( rFormula );
    }
}


Sequence< CellRangeAddress > Rows::getReferenceAddresses( const sal_Int32 nIndex ) const
{
    Sequence< CellRangeAddress > aAddresses( 0 );
    if ( nIndex >= 0 && nIndex < m_aRows.size() )
    {
        Row * pRow = m_aRows[(int)nIndex];
        if ( pRow )
        {
            Sequence< FormulaToken > aTokens( pRow->getTokens() );
            const sal_Int32 nTokens = aTokens.getLength();
            if ( nTokens > 0 )
            {
                SingleReference aSingleRef;
                ComplexReference aComplexRef;
                
                sal_Int32 nAddrIndex = 0;
                const FormulaToken * pTokens = aTokens.getConstArray();
                const FormulaToken * pTokensEnd = aTokens.getConstArray() + nTokens;
                for ( ; pTokens < pTokensEnd; ++pTokens )
                {
                    if ( pTokens->OpCode == 0 && pTokens->Data.hasValue() )
                    {
                        if ( pTokens->Data >>= aSingleRef )
                        {
                            aAddresses.realloc( nAddrIndex + 1 );
                            aAddresses[nAddrIndex++] = CellRangeAddress( 
                                                    aSingleRef.Sheet, 
                                                    aSingleRef.Column, aSingleRef.Row, 
                                                    aSingleRef.Column, aSingleRef.Row );
                        }
                        else if ( pTokens->Data >>= aComplexRef )
                        {
                            aAddresses.realloc( nAddrIndex + 2 );
                            
                            SingleReference aSingleRef1 = aComplexRef.Reference1;
                            SingleReference aSingleRef2 = aComplexRef.Reference2;
                            
                            aAddresses[nAddrIndex++] = CellRangeAddress( 
                                                    aSingleRef1.Sheet, 
                                                    aSingleRef1.Column, aSingleRef1.Row, 
                                                    aSingleRef1.Column, aSingleRef1.Row );
                            aAddresses[nAddrIndex++] = CellRangeAddress( 
                                                    aSingleRef2.Sheet, 
                                                    aSingleRef2.Column, aSingleRef2.Row, 
                                                    aSingleRef2.Column, aSingleRef2.Row );
                        }
                    }
                }
            }
        }
    }
    return aAddresses;
}



} // namespace ww
