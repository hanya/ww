
#ifndef _LISTENERS_HPP_
#define _LISTENERS_HPP_

#include "view.hpp"

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/grid/XMutableGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/grid/XGridRowSelection.hpp>

namespace ww
{


typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::XWindowListener
            > WindowListener_Base;

class WindowListener : protected ::cppu::BaseMutex, 
                       public WindowListener_Base
{
public:
    
    WindowListener( WWView * pView );
    ~WindowListener();
    
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
private:
    WWView * m_pView;
};


typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::XActionListener
            > ActionListener_Base;

class ActionListener : protected ::cppu::BaseMutex, 
                       public ActionListener_Base
{
public:
    
    ActionListener( WWView * pView );
    ~ActionListener();
    
    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
};


typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::XFocusListener
            > FocusListener_Base;

class FocusListener : protected ::cppu::BaseMutex, 
                      public FocusListener_Base
{
public:
    FocusListener( WWView * pView, const WWView::ControlType nType );
    ~FocusListener();
    
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
    WWView::ControlType m_nControlType;
};

typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::XKeyHandler
            > KeyHandler_Base;

class KeyHandler : protected ::cppu::BaseMutex, 
                   public KeyHandler_Base
{
public:
    KeyHandler( WWView * pView );
    ~KeyHandler();
    
    virtual ::sal_Bool SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
};

typedef ::cppu::WeakComponentImplHelper2< 
                    ::com::sun::star::awt::XMouseListener, 
                    ::com::sun::star::awt::XMouseMotionListener
            > MouseListener_Base;

class MouseListener : protected ::cppu::BaseMutex, 
                      public MouseListener_Base
{
public:
    
    MouseListener( WWView * pView );
    ~MouseListener();
    
    virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
    bool m_bAlloDragging;
    bool m_bDragging;
    int m_nRow;
};


typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::grid::XGridSelectionListener
            > GridSelectionListener_Base;

class GridSelectionListener : protected ::cppu::BaseMutex, 
                      public GridSelectionListener_Base
{
public:
    GridSelectionListener( WWView * pView );
    ~GridSelectionListener();
    
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::awt::grid::GridSelectionEvent& gridSelectionEvent ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
};

typedef ::cppu::WeakComponentImplHelper1< 
                    ::com::sun::star::awt::XKeyHandler
            > GridKeyHandler_Base;

class GridKeyHandler : protected ::cppu::BaseMutex, 
                   public GridKeyHandler_Base
{
public:
    GridKeyHandler( WWView * pView );
    ~GridKeyHandler();
    
    virtual ::sal_Bool SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL disposing();
private:
    WWView * m_pView;
};

}

#endif
