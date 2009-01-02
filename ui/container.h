
#pragma once

#ifdef __cplusplus

#include <gdiplus.h>
#include <commctrl.h>
#include <khlist.h>
#include <string>
#include <vector>

#define NIM_NOVTABLE __declspec(novtable)

using namespace Gdiplus;

namespace nim {

    class ControlWindow {
    protected:
        HWND  hwnd;

    protected:
        static ATOM window_class;

    public:
        ControlWindow(): hwnd(NULL) { }

        virtual ~ControlWindow() {
            if (hwnd) DestroyWindow(hwnd);
        }

        /* Message handlers */
    public:
        virtual BOOL OnCreate(LPVOID createParams) { return TRUE; }

        virtual void OnCommand(int id, HWND hwndCtl, UINT codeNotify) { }

        virtual void OnDestroy(void) { }

        virtual void OnPaint(Graphics& g) { }

        virtual BOOL OnPosChanging(LPWINDOWPOS lpos) { return FALSE; }

        virtual void OnPosChanged(LPWINDOWPOS lpos) { }

        virtual void OnHScroll(UINT code, int pos) { }

        virtual void OnVScroll(UINT code, int pos) { }

        virtual void OnMouseMove(const Point& p, UINT keyflags) { }

        virtual void OnMouseHover(const Point& p, UINT keyflags) { }

        virtual void OnMouseLeave() { }

        virtual void OnLButtonDown(bool doubleClick, const Point& p, UINT keyflags) { }

        virtual void OnLButtonUp(const Point& p, UINT keyflags) { }

        virtual void OnContextMenu(const Point& p) { }

        virtual void OnSetFocus(HWND hwnd_old) { }

        virtual void OnKillFocus(HWND hwnd_new) { }

        virtual LRESULT OnNotify(int id, NMHDR * pnmh) { return 0; }

#ifdef KMQ_WM_DISPATCH
        virtual khm_int32 OnWmDispatch(khm_int32 msg_type, khm_int32 msg_subtype, khm_ui_4 uparam,
                                       void * vparam) { return 0; }
#endif

        /* Styles */
    public:
        virtual DWORD GetStyle() {
            return WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                WS_HSCROLL | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL;
        }

        virtual DWORD GetStyleEx() {
            return WS_EX_CONTROLPARENT;
        }

    public:
        virtual Rect& GetClientRect(Rect *cr) {
            RECT r;
            ::GetClientRect(hwnd, &r);
            cr->X = r.left; cr->Y = r.top; cr->Width = r.right - r.left; cr->Height = r.bottom - r.top;
            return *cr;
        }

    protected:
        BOOL HandlePosChanging(HWND hwnd, LPWINDOWPOS lpos) {
            return OnPosChanging(lpos);
        }

        void HandlePosChanged(HWND hwnd, LPWINDOWPOS lpos) {
            OnPosChanged(lpos);
        }

        void HandleMouseMove(HWND hwnd, int x, int y, UINT keyflags) {
            OnMouseMove(Point(x,y), keyflags);
        }

        void HandleMouseHover(HWND hwnd, int x, int y, UINT keyflags) {
            OnMouseHover(Point(x,y), keyflags);
        }

        void HandleMouseLeave(HWND hwnd) {
            OnMouseLeave();
        }

        void HandleLButtonDown(HWND hwnd, BOOL fDblClick, int x, int y, UINT keyflags) {
            OnLButtonDown(!!fDblClick, Point(x,y), keyflags);
        }

        void HandleLButtonUp(HWND hwnd, int x, int y, UINT keyflags) {
            OnLButtonUp(Point(x,y), keyflags);
        }

        void HandleContextMenu(HWND hwnd, HWND hwnd_ctx, int x, int y) {
            OnContextMenu(Point(x, y));
        }

        void HandleCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
            OnCommand(id, hwndCtl, codeNotify);
        }

        LRESULT HandleNotify(HWND hwnd, int id, NMHDR * pnmh) {
            return OnNotify(id, pnmh);
        }

        void HandleSetFocus(HWND hwnd, HWND hwnd_old) {
            OnSetFocus(hwnd_old);
        }

        void HandleKillFocus(HWND hwnd, HWND hwnd_new) {
            OnKillFocus(hwnd_new);
        }

        void HandleOnPaint(HWND hwnd);

        void HandleOnDestroy(HWND hwnd);

        void HandleHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);

        void HandleVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);

        LRESULT HandleDispatch(LPARAM lParam);

        static BOOL HandleOnCreate(HWND hwnd, LPCREATESTRUCT lpc);

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    public:
        HWND Create(HWND parent, Rect & extents, int id, LPVOID createParams = NULL);

        BOOL ShowWindow(int nCmdShow = SW_SHOW);

        static void RegisterWindowClass(void);

        static void UnregisterWindowClass(void);

    private:
        struct CreateParams {
            ControlWindow * cw;
            LPVOID          createParams;
        };
    };

    class TimerQueueClient {
        friend class TimerQueueHost;

    private:
        HANDLE timer;
        HANDLE timerQueue;

    public:
        TimerQueueClient() {
            timer = NULL;
            timerQueue = NULL;
        }

        virtual ~TimerQueueClient() {
            CancelTimer();
        }

        virtual void OnTimer() {
            // This can't be a pure virtual function.  The timer queue
            // may run in a separate thread, in which case the timer
            // may fire after a derived class destructor has run, but
            // before the destructor for this class has run.  In this
            // case, the derived class vtable would no longer exist
            // and the callback has to be received by this function.
        }

        void CancelTimer() {
            if (timer && timerQueue) {
                DeleteTimerQueueTimer(timerQueue, timer,
                                      INVALID_HANDLE_VALUE);
                timer = NULL;
                timerQueue = NULL;
            }
        }
    };

    class TimerQueueHost {
    protected:
        HANDLE timerQueue;

        static VOID CALLBACK TimerCallback(PVOID param, BOOLEAN waitOrTimer) {
            TimerQueueClient * cb = static_cast<TimerQueueClient *>(param);
            cb->OnTimer();
        }

    public:
        TimerQueueHost() { 
            timerQueue = CreateTimerQueue();
        }

        virtual ~TimerQueueHost() {
            if (timerQueue) {
                DeleteTimerQueueEx(timerQueue, INVALID_HANDLE_VALUE);
            }
        }

        void SetTimer(TimerQueueClient * cb, DWORD milliseconds) {
            HANDLE timer;
            if (CreateTimerQueueTimer(&timer, timerQueue, TimerCallback,
                                      cb, milliseconds, 0, WT_EXECUTEONLYONCE)) {
                cb->timer = timer;
                cb->timerQueue = timerQueue;
            }
        }

        void SetTimerRepeat(TimerQueueClient * cb, DWORD ms_start, DWORD ms_repeat) {
            HANDLE timer;
            if (CreateTimerQueueTimer(&timer, timerQueue, TimerCallback,
                                      cb, ms_start, ms_repeat, 0)) {
                cb->timer = timer;
                cb->timerQueue = timerQueue;
            }
        }
    };

    class DisplayColumn {
    public:
        bool sort            : 1;
        bool sort_increasing : 1;
        bool filler          : 1;
        bool group           : 1;
        bool fixed_width     : 1;
        bool fixed_position  : 1;

        int  width;
        int  x;
        std::wstring caption;

        DisplayColumn() {
            width = 0;
            x = 0;
            sort = false; sort_increasing = false; filler = false; group = false;
            fixed_width = false; fixed_position = false;
        }

        DisplayColumn(std::wstring _caption, int _width = 0) : caption(_caption) {
            width = _width;
            x = 0;
            sort = false; sort_increasing = false; filler = false; group = false;
            fixed_width = false; fixed_position = false;
        }

        virtual ~DisplayColumn() {}

        void CheckFlags(void) {
            if (group) sort = true;
            if (fixed_width) filler = false;
        }

        HDITEM& GetHDITEM(HDITEM& hdi);
    };

    class DisplayColumnList : public std::vector<DisplayColumn *> {
    public:
        virtual ~DisplayColumnList();

    public:
        void ValidateColumns();

        void AdjustColumnPositions(int max_width);

        void AddColumnsToHeaderControl(HWND hwnd_header);

        void UpdateHeaderControl(HWND hwnd_header, UINT mask = (HDI_FORMAT | HDI_WIDTH));

        void Clear();
    };

    class DisplayContainer;

    class DisplayElement {
    public:
        Point origin;
        Size  extents;
        DisplayContainer * owner;

        bool expandable      : 1;
        bool is_outline      : 1;

        bool visible         : 1;
        bool expanded        : 1;

        bool focus           : 1;
        bool selected        : 1;
        bool highlight       : 1;

        bool recalc_extents  : 1;

        TQDCL(DisplayElement);

    public:
        DisplayElement() {
            TQINIT(this); 
            owner          = NULL;

            expandable     = false; 
            is_outline     = false;

            visible        = true; 
            expanded       = false; 

            focus          = false;
            selected       = false;
            highlight      = false;

            recalc_extents = true; 
        };

        virtual ~DisplayElement() {
            DeleteAllChildren();

#ifdef assert
            assert(TQPARENT(this) == NULL);
            assert(TQNEXTSIBLING(this) == NULL);
            assert(TQPREVSIBLING(this) == NULL);
#endif
        }

        void   InsertChildAfter(DisplayElement * e, DisplayElement * previous = NULL);

        void   InsertChildBefore(DisplayElement * e, DisplayElement * after = NULL);

        void   DeleteChild(DisplayElement * e);

        void   DeleteAllChildren();

        void   MoveChildAfter(DisplayElement * e, DisplayElement * previous);

        void   MoveChildBefore(DisplayElement * e, DisplayElement * next);

        void   SetOwner(DisplayContainer * _owner);

        void   MarkForExtentUpdate(void);

        void   MarkChildrenForExtentUpdate(void);

        Point  MapToParent(const Point & p);

        virtual Point MapToScreen(const Point & p);

        Point  MapToDescendant(const DisplayElement * e, const Point & p);

        Point  MapFromDescendant(const DisplayElement * c, const Point & p);

        DisplayElement * ChildFromPoint(const Point & p);

        DisplayElement * DescendantFromPoint(const Point & p);

        void UpdateLayout(Graphics & g, const Rect & layout);

        void Invalidate(const Rect & r);

        void Invalidate() {
            Invalidate(Rect(Point(0,0), extents));
        }

        virtual void UpdateLayoutPre(Graphics & g, Rect & layout);

        virtual void UpdateLayoutPost(Graphics & g, const Rect & layout);

        virtual void Expand(bool expand = true);

        virtual void Show(bool show = true);

        virtual void Select(bool _select = true);

        void OnPaint(Graphics& g, const Rect& bounds);

        virtual void PaintSelf(Graphics &g, const Rect& bounds) { }

        virtual void OnClick(const Point& p, UINT keyflags, bool doubleClick) {
            if (TQPARENT(this))
                TQPARENT(this)->OnClick(MapToParent(p), keyflags, doubleClick);
        }

        virtual void OnChildClick(DisplayElement * c, const Point& p, UINT keyflags, bool doubleClick) { }

        virtual void OnContextMenu(const Point& p) { }

        virtual void OnMouse(const Point& p, UINT keyflags) {
            highlight = true;
        }

        virtual void OnMouseOut(void) {
            highlight = false;
        }

        virtual bool OnShowToolTip(std::wstring& caption, Rect& align_rect) { return false; }

        virtual bool IsTabStop() { return false; }

        virtual HFONT*GetHFONT() { return NULL; }

        virtual Font *GetFont(Graphics& g) { return NULL; }

        virtual Color GetForegroundColor() { return Color(Color::Black); }
    };

    class DisplayContainer : virtual public DisplayElement, public ControlWindow {
    protected:
        Rect    scroll;
        Image * dbuffer;

        HWND    hwnd_header;
        int     header_height;

    public:
        bool    show_header  : 1;
        DisplayColumnList columns;

    public:
        DisplayElement * mouse_element;
        bool    mouse_dblclk : 1;
        bool    mouse_track  : 1;

    public:
        DisplayContainer() {
            dbuffer = 0; show_header = false; hwnd_header = NULL;
            header_height = 0;
            mouse_element = NULL; mouse_dblclk = false;
            owner = this;
        }

        virtual ~DisplayContainer() { if (dbuffer) delete dbuffer; }

        Point ClientToVirtual(const Point& p) { 
            return Point(p.X + scroll.X, p.Y + scroll.Y - header_height);
        }

        bool ValidateScrollPos(void);

        bool UpdateScrollBars(bool redraw);

        bool UpdateScrollInfo(void);

        void UpdateExtents(Graphics& g);

        void ScrollBy(const Point & delta);

        virtual BOOL OnCreate(LPVOID createParams);

        virtual void UpdateLayoutPre(Graphics & g, Rect & layout);

        virtual void UpdateLayoutPost(Graphics& g, const Rect& layout);

        virtual void OnPosChanged(LPWINDOWPOS lp);

        virtual void Invalidate(const Rect & r);

        void Invalidate() {
            ::InvalidateRect(hwnd, NULL, TRUE);
        }

        virtual Point MapToScreen(const Point & p);

        virtual void OnPaint(Graphics& g);

        virtual void OnHScroll(UINT code, int pos);

        virtual void OnVScroll(UINT code, int pos);

        virtual Rect& GetClientRect(Rect * cr);

        Rect&   GetClientRectNoScroll(Rect * cr);

        virtual void OnMouseMove(const Point& p, UINT keyflags);

        virtual void OnMouseHover(const Point& p, UINT keyflags);

        virtual void OnMouseLeave();

        virtual void OnLButtonDown(bool doubleClick, const Point& p, UINT keyflags);

        virtual void OnLButtonUp(const Point & p, UINT keyflags);

        virtual void OnContextMenu(const Point & p);

        virtual LRESULT OnNotify(int id, NMHDR * pnmh);


        LRESULT OnHeaderNotify(NMHEADER * pnmh);

        LRESULT OnHeaderBeginTrack(NMHEADER * pnmh);

        LRESULT OnHeaderEndTrack(NMHEADER * pnmh);

        LRESULT OnHeaderTrack(NMHEADER * pnmh);

        LRESULT OnHeaderBeginDrag(NMHEADER * pnmh);

        LRESULT OnHeaderEndDrag(NMHEADER * pnmh);

        LRESULT OnHeaderItemClick(NMHEADER * pnmh);

        LRESULT OnHeaderItemDblClick(NMHEADER * pnmh);

        LRESULT OnHeaderItemChanging(NMHEADER * pnmh);

        LRESULT OnHeaderItemChanged(NMHEADER * pnmh);

    public:                     // Overridables
        virtual void OnColumnSizeChanged(int idx) {}
        virtual void OnColumnPosChanged(int from, int to) {}
        virtual void OnColumnSortChanged(int idx) {}
        virtual void OnColumnContextMenu(int idx) {}
    };

    // Applies to DisplayElement
    template<class T = DisplayElement>
    class WithColumnAlignment : public T {
        int col_idx;
        int col_span;

    public:
        WithColumnAlignment() { col_idx = -1; col_span = 0; }

        WithColumnAlignment(int idx, int span = 1) {
            col_idx = idx;
            col_span = span;
        }

        ~WithColumnAlignment() { }

        virtual void UpdateLayoutPre(Graphics & g, Rect & layout) {
            if (owner == NULL || col_idx < 0 || (unsigned int) col_idx >= owner->columns.size()) {
                visible = false;
                return;
            }

            origin = owner->MapToDescendant(TQPARENT(this), Point(owner->columns[col_idx]->x, 0));
            origin.Y = 0;

            extents.Width = 0;
            extents.Height = GetSystemMetrics(SM_CYICON);

            for (int i = col_idx;
                 (col_span > 0 && i < col_idx + col_span) ||
                     (col_span <= 0 && (unsigned int) i < owner->columns.size()); i++) {
                extents.Width += owner->columns[i]->width;
            }

            layout.Width = extents.Width;
            layout.X = origin.X;
        }

        void SetColumnAlignment(int idx, int span = 1) {
            col_idx = idx;
            col_span = span;
            MarkForExtentUpdate();
        }
    };

    // Applies to DisplayContainer
    template<class T = DisplayContainer>
    class WithTooltips : public T {
        HWND hwnd_tooltip;
        std::wstring tt_text;
        Rect         tt_rect;
        bool         show_tt : 1;
        bool         shown_tt: 1;
        Point        pt_mouse;

        static const int TOOL_ID = 1;

    public:
        WithTooltips() : tt_rect(0,0,0,0) {
            show_tt = false;
            shown_tt = false;
        }

        ~WithTooltips() {
            if (hwnd_tooltip)
                DestroyWindow(hwnd_tooltip);
            hwnd_tooltip = NULL;
        }

        virtual BOOL OnCreate(LPVOID createParams) {
            hwnd_tooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS,
                                          NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
                                          CW_USEDEFAULT, CW_USEDEFAULT,
                                          CW_USEDEFAULT, CW_USEDEFAULT,
                                          hwnd, NULL, khm_hInstance, NULL);
            SetWindowPos(hwnd_tooltip, HWND_TOPMOST, 0, 0,
                         0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            TOOLINFO ti = { sizeof(TOOLINFO), TTF_ABSOLUTE | TTF_TRACK, hwnd, TOOL_ID, {0,0,0,0},
                            khm_hInstance, LPSTR_TEXTCALLBACK, 0 };
            ::GetClientRect(hwnd, &ti.rect);

            if (!SendMessage(hwnd_tooltip, TTM_ADDTOOL, 0, (LPARAM) &ti)) {
                ti.cbSize -= sizeof(void *);
                SendMessage(hwnd_tooltip, TTM_ADDTOOL, 0, (LPARAM) &ti);
            }
            return T::OnCreate(createParams);
        }

        virtual void OnMouseHover(const Point& p, UINT keyflags) {
            if (mouse_element) {
                show_tt = mouse_element->OnShowToolTip(tt_text, tt_rect);
                if (show_tt) {
                    TOOLINFO ti = { sizeof(TOOLINFO), 0, hwnd, TOOL_ID, {0,0,0,0},
                                    NULL, NULL, 0 };
                    RECT r = { tt_rect.GetLeft(), tt_rect.GetTop(),
                               tt_rect.GetRight(), tt_rect.GetBottom() };

                    SendMessage(hwnd_tooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM) &ti);
                    SendMessage(hwnd_tooltip, TTM_ADJUSTRECT, TRUE, (LPARAM) &r);
                    SendMessage(hwnd_tooltip, TTM_TRACKPOSITION, 0, MAKELONG(r.left, r.top));
                    shown_tt = true;
                    pt_mouse = p;
                }
            }
            T::OnMouseHover(p, keyflags);
        }

        virtual void OnMouseMove(const Point& pt_c, UINT keyflags) {
            if (shown_tt && !pt_mouse.Equals(pt_c)) {
                TOOLINFO ti = { sizeof(TOOLINFO), 0, hwnd, TOOL_ID, {0,0,0,0},
                                NULL, NULL, 0 };

                SendMessage(hwnd_tooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
                shown_tt = false;
            }
            T::OnMouseMove(pt_c, keyflags);
        }

        LPARAM OnTooltipGetDispInfo(NMTTDISPINFO * pdi) {
            pdi->lpszText = (LPWSTR) tt_text.c_str();
            return !show_tt;
        }

        LPARAM OnTooltipShow(NMHDR * pnmh) {
            return 0;
        }

        LPARAM OnTooltipPop(NMHDR * pnmh) {
            return 0;
        }

        LPARAM OnTooltipNotify(int id, NMHDR * pnmh) {
            switch (pnmh->code) {
                case TTN_GETDISPINFO:
                    return OnTooltipGetDispInfo(reinterpret_cast<NMTTDISPINFO *>(pnmh));

                case TTN_SHOW:
                    return OnTooltipShow(pnmh);

                case TTN_POP:
                    return OnTooltipPop(pnmh);
            }
            return 0;
        }

        virtual LPARAM OnNotify(int id, NMHDR * pnmh) {
            if (pnmh->hwndFrom == hwnd_tooltip)
                return OnTooltipNotify(id, pnmh);
            return T::OnNotify(id, pnmh);
        }
    };

    // Applies to DisplayElement
    template<class T = DisplayElement>
    class WithTextDisplay : public T {
    protected:
        Point caption_pos;
        bool  truncated;

        virtual void UpdateLayoutPost(Graphics& g, const Rect& bounds) {
            std::wstring caption = GetCaption();

            StringFormat sf;
            GetStringFormat(sf);

            SizeF sz((REAL) bounds.Width, (REAL) bounds.Height);
            SizeF szr;

            g.MeasureString(caption.c_str(), caption.length(), GetFont(g), sz, &sf, &szr);

            extents.Width = szr.Width;
            extents.Height = szr.Height;
        }

        virtual void PaintSelf(Graphics& g, const Rect& bounds) {
            std::wstring caption = GetCaption();

            StringFormat sf;
            GetStringFormat(sf);

            RectF rf((REAL) bounds.X, (REAL) bounds.Y, (REAL) bounds.Width, (REAL) bounds.Height);
            Brush br(GetForegroundColor());
            g.DrawString(caption.c_str(), caption.length(), GetFont(g), rf, &sf, &br);

            RectF bb(0,0,0,0);
            INT   chars;
            g.MeasureString(caption.c_str(), caption.length(), GetFont(g), rf, &sf, &bb, &chars);
            caption_pos.X = (INT) bb.X - bounds.X;
            caption_pos.Y = (INT) bb.Y - bounds.Y;
            truncated = ((unsigned int) chars != caption.length());
        }

        virtual bool OnShowToolTip(std::wstring& _caption, Rect& align_rect) {
            if (!truncated)
                return false;

            Point pto = MapToScreen(caption_pos);
            align_rect.X = pto.X; align_rect.Y = pto.Y;
            align_rect.Width = extents.Width; align_rect.Height = extents.Height;
            _caption = GetCaption();
            return true;
        }

    public:
        WithTextDisplay() : caption() {
            truncated = false;
        }

        virtual std::wstring GetCaption() {
            return std::wstring(L"");
        }

        virtual void GetStringFormat(StringFormat& sf) {
            sf.SetFormatFlags(StringFormatFlagsNoWrap);
            sf.SetTrimming(StringTrimmingEllipsisCharacter);
            sf.SetLineAlignment(StringAlignmentCenter);
        }
    };

    // Applies to DisplayElement
    template<class T = DisplayElement>
    class WithTabStop : public T {

        virtual bool IsTabStop() {
            return true;
        }

        virtual void OnClick(const Point& pt, UINT keyflags, bool doubleClick) {
            if (doubleClick) {
                T::OnClick(pt, keyflags, doubleClick);
                return;
            }

            DisplayElement * p;
            for (p = TQPARENT(this); p; p = TQPARENT(p)) {
                if (TQPARENT(p) == NULL)
                    break;
            }

            if (p) p->OnChildClick(this, pt, keyflags, doubleClick);
            T::OnClick(pt, keyflags, doubleClick);
        }
    };

    // Applies to DisplayElement
    template<class T = DisplayElement>
    class WithOutline : public T {
    public:
        WithOutline() { expandable = true; expanded = true; }

        void InsertOutlineAfter(DisplayElement * e, DisplayElement * previous = NULL) {
            T::InsertChildAfter(e, previous);
            e->is_outline = true;
            e->visible = expanded;
        }

        void InsertOutlineBefore(DisplayElement * e, DisplayElement * next = NULL) {
            T::InsertChildBefore(e, next);
            e->is_outline = true;
            e->visible = expanded;
        }
    };

    // Applies to DisplayContainer
    template<class T = DisplayContainer>
    class WithNavigation : public T {
        DisplayElement * focus;
        DisplayElement * anchor;

        DisplayElement * PrevElement(DisplayElement * c) {
            if (c == NULL) {
                if (TQLASTCHILD(this) == NULL)
                    return this;
                c = TQLASTCHILD(this);
                while (TQLASTCHILD(c))
                    c = TQLASTCHILD(c);
                return c;
            }
            if (TQPREVSIBLING(c)) {
                c = TQPREVSIBLING(c);
                while (TQLASTCHILD(c))
                    c = TQLASTCHILD(c);
                return c;
            } else
                return TQPARENT(c);
        }

        DisplayElement * PrevTabStop(DisplayElement * c) {
            do {
                c = PrevElement(c);
            } while (c != NULL && !c->IsTabStop());
            return c;
        }

        DisplayElement * NextElement(DisplayElement * c) {
            if (c == NULL)
                return this;
            if (TQFIRSTCHILD(c))
                return TQFIRSTCHILD(c);
            if (TQNEXTSIBLING(c))
                return TQNEXTSIBLING(c);
            if (TQPARENT(c)) {
                do {
                    c = TQPARENT(c);
                } while (c && TQNEXTSIBLING(c) == NULL);
                if (c)
                    return TQNEXTSIBLING(c);
            }
            return NULL;
        }

        DisplayElement * NextTabStop(DisplayElement * c) {
            do {
                c = NextElement(c);
            } while (c != NULL && !c->IsTabStop());
            return c;
        }

        static void SelectAllIn(DisplayElement * e, bool select) {
            e->Select(select);
            for (DisplayElement * c = TQFIRSTCHILD(e); c; c = TQNEXTSIBLING(c)) {
                SelectAllIn(c, select);
            }
        }

    public:
        WithNavigation() { focus = NULL; anchor = NULL; }
        ~WithNavigation() { }

        typedef enum FocusAction {
            FocusExclusive,
            FocusExtend,
            FocusAddToggle
        } FocusAction;

        void SetFocusElement(DisplayElement * e, FocusAction action) {
            if (e == NULL)
                return;

            switch (action) {
            case FocusAddToggle:
                e->Select(!e->selected);
                anchor = e;
                break;

            case FocusExclusive:
                SelectAllIn(this, false);
                e->Select(true);
                anchor = e;
                break;

            case FocusExtend:
                DisplayElement * c;
                for (c = e; c; c = NextTabStop(c)) {
                    if (c == anchor)
                        break;
                }
                if (c != NULL) {
                    SelectAllIn(this, false);
                    for (c = e; c; c = NextTabStop(c)) {
                        c->Select();
                        if (c == anchor)
                            break;
                    }
                    break;
                }
                for (c = e; c; c = PrevTabStop(c)) {
                    if (c == anchor)
                        break;
                }
                if (c != NULL) {
                    SelectAllIn(this, false);
                    for (c = e; c; c = PrevTabStop(c)) {
                        c->Select();
                        if (c == anchor)
                            break;
                    }
                    break;
                }
                e->Select();
                anchor = e;
                break;
            }
            focus = e;

            Rect focusRect(MapFromDescendant(focus, Point(0,0)), focus->extents);
            if (!scroll.Contains(focusRect)) {
                Rect oldScroll = scroll;

                if (focusRect.Y < scroll.Y)
                    scroll.Y = focusRect.Y;
                else if (scroll.GetBottom() < focusRect.GetBottom())
                    scroll.Y = focusRect.GetBottom() - scroll.Height;

                ValidateScrollPos();
                if (!oldScroll.Equals(scroll)) {
                    UpdateScrollBars(true);
                    ScrollBy(Point(scroll.X - oldScroll.X, scroll.Y - oldScroll.Y));
                }
            }
        }

        virtual void OnCommand(int id, HWND hwndCtl, UINT codeNotify) {
            switch (id) {
            case KHUI_PACTION_LEFT:
                if (focus && focus->expandable && focus->expanded) {
                    focus->Expand(false);
                    Invalidate();
                    break;
                }
                // fallthrough

            case KHUI_PACTION_UP:
                SetFocusElement(PrevTabStop(focus), FocusExclusive); break;

            case KHUI_PACTION_UP_EXTEND:
                SetFocusElement(PrevTabStop(focus), FocusExtend); break;

            case KHUI_PACTION_UP_TOGGLE:
                SetFocusElement(PrevTabStop(focus), FocusAddToggle); break;

            case KHUI_PACTION_PGUP_EXTEND:
            case KHUI_PACTION_PGUP:
                if (focus) {
                    Rect r;
                    Point p = MapFromDescendant(focus, Point(0,focus->extents.Height));
                    p.Y -= GetClientRect(&r).Height;
                    DisplayElement * e = DescendantFromPoint(p);
                    if (e) e = PrevTabStop(e);
                    if (e) e = NextTabStop(e);
                    SetFocusElement((e)? e : NextTabStop(NULL),
                        (id == KHUI_PACTION_PGUP)? FocusExclusive : FocusExtend);
                } else {
                    SetFocusElement(NextTabStop(NULL), 
                        (id == KHUI_PACTION_PGUP)? FocusExclusive : FocusExtend);
                }
                break;

            case KHUI_PACTION_RIGHT:
                if (focus && focus->expandable && !focus->expanded) {
                    focus->Expand(true);
                    Invalidate();
                    break;
                }
                // fallthrough

            case KHUI_PACTION_DOWN:
                SetFocusElement(NextTabStop(focus), FocusExclusive); break;

            case KHUI_PACTION_DOWN_EXTEND:
                SetFocusElement(NextTabStop(focus), FocusExtend); break;

            case KHUI_PACTION_DOWN_TOGGLE:
                SetFocusElement(NextTabStop(focus), FocusAddToggle); break;

            case KHUI_PACTION_PGDN_EXTEND:
            case KHUI_PACTION_PGDN:
                if (focus) {
                    Rect r;
                    Point p = MapFromDescendant(focus, Point(0,0));
                    p.Y += GetClientRect(&r).Height;
                    DisplayElement * e = DescendantFromPoint(p);
                    if (e) e = PrevTabStop(e);
                    if (e) e = NextTabStop(e);
                    SetFocusElement((e)? e : PrevTabStop(NULL),
                        (id == KHUI_PACTION_PGDN)? FocusExclusive : FocusExtend);
                } else {
                    SetFocusElement(PrevTabStop(NULL), 
                        (id == KHUI_PACTION_PGDN)? FocusExclusive : FocusExtend);
                }
                break;

            case KHUI_PACTION_SELALL:
                SelectAllIn(this, true);
                break;
            }

            T::OnCommand(id, hwndCtl, codeNotify);
        }

        virtual void OnChildClick(DisplayElement * e, const Point& p, UINT keyflags, bool doubleClick) {
            if (!e->IsTabStop()) return;

            if ((keyflags & MK_SHIFT) == MK_SHIFT) {
                SetFocusElement(e, FocusExtend);
            } else if ((keyflags & MK_CONTROL) == MK_CONTROL) {
                SetFocusElement(e, FocusAddToggle);
            } else {
                SetFocusElement(e, FocusExclusive);
            }
            T::OnChildClick(e, p, keyflags, doubleClick);
        }
    };

    // Applies to DisplayElement
    template <class T = DisplayElement>
    class WithVerticalLayout : public T {
        virtual void UpdateLayoutPost(Graphics & g, const Rect & layout) {
            DisplayElement * c;
            Point p(0,0);
            Size ext(0,0);

            T::UpdateLayoutPost(g, layout);

            for (c = TQFIRSTCHILD(this); c; c = TQNEXTSIBLING(c)) 
                if (c->visible) {
                    c->origin.Y = p.Y;
                    ext.Height += c->extents.Height;
                    p.Y = ext.Height;
                    if (ext.Width < c->extents.Width)
                        ext.Width = c->extents.Width;
                }

            extents = ext;
        }
    };

    // Applies to DisplayElement
    template <class T = DisplayElement>
    class WithFixedSizePos : public T {
        Point fixed_origin;
        Size  fixed_extents;
    public:
        WithFixedSizePos(const Point& p, const Size& s) : fixed_origin(p), fixed_extents(s) {}
        WithFixedSizePos() {}

        void SetSize(const Size& s) {
            fixed_extents = s;
            MarkForExtentUpdate();
        }

        void SetPosition(const Point& p) {
            fixed_origin = p;
            MarkForExtentUpdate();
        }

        virtual void UpdateLayoutPost(Graphics & g, const Rect & layout) {
            origin = fixed_origin;
            extents = fixed_extents;
        }
    };

    template <class T>
    class NIM_NOVTABLE WithCachedFont : public T {
        Font *cached_font;

        WithCachedFont() {
            cached_font = NULL;
        }

        ~WithCachedFont() {
            if (cached_font)
                delete cached_font;
            cached_font = NULL;
        }

        // Font * GetFontCreate(HDC hdc) needs to be implemented by the base class

    public:
        virtual Font* GetFont(Graphics& g) {
            if (cached_font == NULL) {
                HDC hdc = g.GetHDC();
                cached_font = GetFontCreate(hdc);
                g.ReleaseHDC(hdc);
            }

            return cached_font;
        }
    };

    class FlowLayout {
        Rect bounds;
        INT  top;
        INT  baseline;
        INT  left;
        Size margin;

        void ExtendBaseline(INT nb) {
            if (nb + top > baseline) {
                baseline = nb + top;
                if (bounds.Height < baseline)
                    bounds.Height = baseline;
            }
        }

    public:
        FlowLayout(const Rect& _bounds, const Size& _margin) {
            bounds = _bounds;
            top = 0;
            baseline = top;
            left = 0;
            margin = _margin;
        }

        Rect GetBounds() {
            return bounds;
        }

        Size GetSize() {
            return Size(bounds.Width, bounds.Height);
        }

        void AddSquishLeft(DisplayElement * e) {
            if (left != 0)
                left += margin.Width;
            e->origin.X = left + bounds.X;
            e->origin.Y = top + bounds.Y;
            e->extents.Width = __max(bounds.Width - left, e->extents.Width);
            left += e->extents.Width;

            ExtendBaseline(e->extents.Height);
        }

        void AddSquishRight(DisplayElement * e) {
            if (left != 0)
                left += margin.Width;
            e->origin.Y = top + bounds.Y;
            e->extents.Width = __max(bounds.Width - left, e->extents.Width);
            e->origin.X = (bounds.Width - e->extents.Width) + bounds.X;
            left = bounds.Width;

            ExtendBaseline(e->extents.Height);
        }

        void AddFixed(DisplayElement * e) {
            if (left != 0
                left += margin.Width;
            e->origin.X = left + bounds.X;
            e->origin.Y = top + bounds.Y;
            left += e->extents.Width;

            ExtendBaseline(e->extents.Height);
        }

        void LineBreak() {
            left = 0;
            top = baseline + margin.Height;
        }
    };
}
#endif
