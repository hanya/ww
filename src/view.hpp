
#ifndef _VIEW_HPP_
#define _VIEW_HPP_

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>


#ifdef AOO4
#define UNIFIED_MENU_API
#include <com/sun/star/awt/XPopupMenu.hpp>
#else
#undef  UNIFIED_MENU_API
#include <com/sun/star/awt/XPopupMenuExtended.hpp>
#endif

namespace ww
{

class WWModel;
class WWView;


class WWView : protected ::cppu::BaseMutex
{
public:
    
    WWView( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, 
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame, 
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xParentWindow );
    ~WWView();
    
    enum ControlType {
        Edit, 
        Grid
    };
    
    void setModel( WWModel * pModel );
    void disconnectModel();
    
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > getWindow() const;
    void resizeWindow( const sal_Int32 nWidth, const sal_Int32 nHeight );
    void executeCommand( const int nCommand );
    void executeGridCommand( const int nCommand, const int nModifires );
    
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getDocument() const throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getSelection() const throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XMutableGridDataModel > getGridDataModel() const;
    
    void executeContextMenu( const ::sal_Int32 nX, const ::sal_Int32 nY );
    void selectRow( const sal_Int32 nIndex, const bool bGoToRow ) const;
    void selectRow( const ::sal_Int32 nX, const ::sal_Int32 nY, const ::sal_Int16 nModifires ) const;
    
    void gridSelectionChanged() const;
    bool hasGridSelection() const;
    int getRowAt( const ::sal_Int32 nX, const ::sal_Int32 nY ) const;
    int getRowHeight();
    
    bool startDragging( const ::sal_Int32 nX, const ::sal_Int32 nY ) const;
    void endDragging( const ::sal_Int32 nX, const ::sal_Int32 nY );
    
    void drawDestination( const int nNewRow, const int nOldRow );
    void setPointer( const sal_Int32 nPointer );
    
    void controlFocusGained( const ControlType nType ) const;
    void controlFocusLost( const ControlType nType ) const;
    
    void updateRowFormula() const;
    
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getLabels();
private:
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aResource;
    
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    // ToDo should check frame action?
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    WWModel * m_pModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xContainerWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xParentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridRowSelection > m_xGridRowSelection;
    
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchHelper > m_xDispatchHelper;
    
#ifdef UNIFIED_MENU_API
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > m_xPopupMenu;
#else
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenuExtended > m_xPopupMenu;
#endif
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener > m_xWindowListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > m_xActionListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler > m_xKeyHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler > m_xGridKeyHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener > m_xFocusListener;
    
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer > m_xPointer;
    
    bool m_bUseInputLine;
    int m_nRow;
    int m_nRowHeight;
    
    void focusToDoc() const;
    
    ::com::sun::star::uno::Sequence< sal_Int32 > getSelectedEntryIndex() const;
    
    ::rtl::OUString getInputLineText() const;
    void setInputLine( const ::rtl::OUString & rText ) const;
    void switchInputLine( const bool bState );
    
    void updateButtonsState() const;
    void gotoPoint( const ::rtl::OUString & rDesc );
    
    void executeDispatch( const ::rtl::OUString & rCommand, 
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & rArgs );
    
    void showOptionPopup();
    void openOption();
    void loadResource();
    
    void createButton(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & xControlContainer, 
                    const ::rtl::OUString & sName, 
                    const ::rtl::OUString & sHelpText, 
                    const ::rtl::OUString & sImageURL, 
                    const ::rtl::OUString & sActionCommand, 
                    const ::sal_Int32 nLeft, const ::sal_Int32 nTop, 
                    const ::sal_Int32 nWidth, const ::sal_Int32 nHeight ) throw (::com::sun::star::uno::RuntimeException);
    void createWindow();
    
};

} // namespace ww

#endif
