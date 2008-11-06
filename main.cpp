#include <windows.h>
#include "ncbind.hpp"

// �C�x���g���ꗗ
#define EXEV_MINIMIZE  TJS_W("onMinimize")
#define EXEV_MAXIMIZE  TJS_W("onMaximize")
#define EXEV_QUERYMAX  TJS_W("onMaximizeQuery")
#define EXEV_SHOW      TJS_W("onShow")
#define EXEV_HIDE      TJS_W("onHide")
#define EXEV_RESIZING  TJS_W("onResizing")
#define EXEV_MOVING    TJS_W("onMoving")
#define EXEV_MOVE      TJS_W("onMove")
#define EXEV_MVSZBEGIN TJS_W("onMoveSizeBegin")
#define EXEV_MVSZEND   TJS_W("onMoveSizeEnd")

////////////////////////////////////////////////////////////////

struct WindowEx
{
	//--------------------------------------------------------------
	// ���[�e�B���e�B

	// �E�B���h�E�n���h�����擾
	static HWND GetHWND(iTJSDispatch2 *obj) {
		tTJSVariant val;
		obj->PropGet(0, TJS_W("HWND"), 0, &val, obj);
		return (HWND)(tjs_int)(val);
	}

	// RECT�������ɕۑ�
	static bool SetRect(ncbDictionaryAccessor &rdic, LPCRECT lprect) {
		bool r;
		if ((r = rdic.IsValid())) {
			rdic.SetValue(TJS_W("x"), lprect->left);
			rdic.SetValue(TJS_W("y"), lprect->top);
			rdic.SetValue(TJS_W("w"), lprect->right  - lprect->left);
			rdic.SetValue(TJS_W("h"), lprect->bottom - lprect->top );
		}
		return r;
	}

	// SYSCOMMAND�𑗂�
	static tjs_error postSysCommand(iTJSDispatch2 *objthis, WPARAM param) {
		PostMessage(GetHWND(objthis), WM_SYSCOMMAND, param, 0);
		return TJS_S_OK;
	}

	//--------------------------------------------------------------
	// �N���X�ǉ����\�b�h(RawCallback�`��)

	// minimize, maximize, showRestore
	static tjs_error TJS_INTF_METHOD minimize(   tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_MINIMIZE); }
	static tjs_error TJS_INTF_METHOD maximize(   tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_MAXIMIZE); }
	static tjs_error TJS_INTF_METHOD showRestore(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_RESTORE);  }

	// getWindowRect
	static tjs_error TJS_INTF_METHOD getWindowRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		RECT rect;
		HWND hwnd = GetHWND(obj);
		r->Clear();
		if (hwnd != NULL && ::GetWindowRect(hwnd, &rect)) {
			ncbDictionaryAccessor dict;
			if (SetRect(dict, &rect)) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}

	// getClientRect
	static tjs_error TJS_INTF_METHOD getClientRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		RECT rect;
		POINT zero = { 0, 0 };
		HWND hwnd = GetHWND(obj);
		r->Clear();
		if (hwnd != NULL && ::GetClientRect(hwnd, &rect)) {
			::ClientToScreen(hwnd, &zero);
			rect.left   += zero.x; rect.top    += zero.y;
			rect.right  += zero.x; rect.bottom += zero.y;
			ncbDictionaryAccessor dict;
			if (SetRect(dict, &rect)) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}

	// getMouseCursorPos
	static tjs_error TJS_INTF_METHOD getMouseCursorPos(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		POINT pt = { 0, 0 };
		if (::GetCursorPos(&pt)) {
			ncbDictionaryAccessor dict;
			if (dict.IsValid()) {
				dict.SetValue(TJS_W("x"), pt.x);
				dict.SetValue(TJS_W("y"), pt.y);
				*r = tTJSVariant(dict, dict);
			}
		}
		return TJS_S_OK;
	}

	//--------------------------------------------------------------
	// �g���C�x���g�p

	// �����o�����݂��邩
	bool hasMember(tjs_char const *name) const {
		tTJSVariant func;
		return TJS_SUCCEEDED(self->PropGet(TJS_MEMBERMUSTEXIST, name, 0, &func, self));
	}

	// TJS���\�b�h�Ăяo��
	tjs_error funcCall(tjs_char const *name, tTJSVariant *result, tjs_int numparams = 0, tTJSVariant **params = 0) const {
		return self->FuncCall(0, name, 0, result, numparams, params, self);
	}

	// �����Ȃ��R�[���o�b�N
	bool callback(tjs_char const *name) const {
		if (!hasMember(name)) return false;
		tTJSVariant rslt;
		funcCall(name, &rslt, 0, 0);
		return !!rslt.AsInteger();
	}

	// ���W�n���R�[���o�b�N
	bool callback(tjs_char const *name, int x, int y) const {
		tTJSVariant vx(x), vy(y);
		tTJSVariant rslt, *params[] = { &vx, &vy };
		funcCall(name, &rslt, 2, params);
		return !!rslt.AsInteger();
	}

	// ��`�n���R�[���o�b�N
	bool callback(tjs_char const *name, RECT *rect, int type = 0) const {
		ncbDictionaryAccessor dict;
		if (SetRect(dict, rect)) {
			if (type != 0) dict.SetValue(TJS_W("type"), type);
			tTJSVariant pdic(dict, dict);
			tTJSVariant rslt, *params[] = { &pdic };
			funcCall(name, &rslt, 1, params);
			if (rslt.AsInteger()) {
				ncbTypedefs::Tag<LONG> LongTypeTag;
				rect->left   = dict.GetValue(TJS_W("x"), LongTypeTag);
				rect->top    = dict.GetValue(TJS_W("y"), LongTypeTag);
				rect->right  = dict.GetValue(TJS_W("w"), LongTypeTag);
				rect->bottom = dict.GetValue(TJS_W("h"), LongTypeTag);
				rect->right  += rect->left;
				rect->bottom += rect->top;
				return true;
			}
		}
		return false;
	}

	// ���b�Z�[�W����
	bool onMessage(tTVPWindowMessage *mes) {
		switch (mes->Msg) {
		case WM_SYSCOMMAND:
			if ((mes->WParam & 0xFFF0) == SC_MAXIMIZE) return callback(EXEV_QUERYMAX);
			break;
		case WM_SIZE:
			switch (mes->WParam) {
			case SIZE_MINIMIZED: callback(EXEV_MINIMIZE); break;
			case SIZE_MAXIMIZED: callback(EXEV_MAXIMIZE); break;
			}
			break;
		case WM_SHOWWINDOW:
			switch (mes->LParam) {
			case SW_PARENTOPENING: callback(EXEV_SHOW); break;
			case SW_PARENTCLOSING: callback(EXEV_HIDE); break;
			}
			break;
		case WM_ENTERSIZEMOVE: callback(EXEV_MVSZBEGIN); break;
		case WM_EXITSIZEMOVE:  callback(EXEV_MVSZEND);   break;
		case WM_SIZING: if (hasResizing) callback(EXEV_RESIZING, (RECT*)mes->LParam, mes->WParam); break;
		case WM_MOVING: if (hasMoving)   callback(EXEV_MOVING,   (RECT*)mes->LParam); break;
		case WM_MOVE:   if (hasMove)     callback(EXEV_MOVE, (int)LOWORD(mes->LParam), (int)HIWORD(mes->LParam)); break;
		}
		return false;
	}

	// ���b�Z�[�W���V�[�o
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *mes) {
		WindowEx *inst = ncbInstanceAdaptor<WindowEx>::GetNativeInstance((iTJSDispatch2*)userdata);
		return inst ? inst->onMessage(mes) : false;
	}

	// Message Receiver �o�^�E����
	void regist(bool en) {
		tTJSVariant mode(en ? wrmRegister : wrmUnregister); 
		tTJSVariant func((tTVInteger)(&receiver));
		tTJSVariant data((tTVInteger)(self));
		tTJSVariant rslt, *params[] = { &mode, &func, &data };
		self->FuncCall(0, TJS_W("registerMessageReceiver"), 0, &rslt, 3, params, self);
	}

	// �l�C�e�B�u�C���X�^���X�̐����E�j���ɂ��킹�ă��V�[�o��o�^�E��������
	WindowEx(iTJSDispatch2 *obj)
		:   self(obj),
			hasResizing(false),
			hasMoving(false),
			hasMove(false)
		{ regist(true); }

	~WindowEx() { regist(false); }

	void checkExEvents() {
		hasResizing = hasMember(EXEV_RESIZING);
		hasMoving   = hasMember(EXEV_MOVING);
		hasMove     = hasMember(EXEV_MOVE);
	}
private:
	iTJSDispatch2 *self;
	bool hasResizing, hasMoving, hasMove; //< ���\�b�h�����݂��邩�t���O
};

// Window�Ƀ��\�b�h��ǉ�
NCB_ATTACH_FUNCTION(minimize,          Window, WindowEx::minimize);
NCB_ATTACH_FUNCTION(maximize,          Window, WindowEx::maximize);
NCB_ATTACH_FUNCTION(showRestore,       Window, WindowEx::showRestore);
NCB_ATTACH_FUNCTION(getWindowRect,     Window, WindowEx::getWindowRect);
NCB_ATTACH_FUNCTION(getClientRect,     Window, WindowEx::getClientRect);
NCB_ATTACH_FUNCTION(getMouseCursorPos, Window, WindowEx::getMouseCursorPos);

// �g���C�x���g�p�l�C�e�B�u�C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WindowEx)
{
	/**/  NCB_GET_INSTANCE_HOOK_CLASS () {}
	/**/ ~NCB_GET_INSTANCE_HOOK_CLASS () {}
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) SetNativeInstance(objthis, (obj = new ClassT(objthis)));
		return obj;
	}
};
// ���\�b�h�ǉ�
NCB_ATTACH_CLASS_WITH_HOOK(WindowEx, Window) {
	Method(TJS_W("registerExEvent"),  &Class::checkExEvents);
}

////////////////////////////////////////////////////////////////
struct MenuItemEx
{
	// ���j���[���擾
	static HMENU GetHMENU(iTJSDispatch2 *obj) {
		if (!obj) return NULL;
		tTJSVariant val;
		iTJSDispatch2 *global = TVPGetScriptDispatch(), *mi;
		if (global) {
			global->PropGet(0, TJS_W("MenuItem"), 0, &val, obj);
			mi = val.AsObjectNoAddRef();
			val.Clear();
			global->Release();
		} else mi = obj;
		mi->PropGet(0, TJS_W("HMENU"), 0, &val, obj);
		return (HMENU)(tjs_int)(val);
	}
	// �e���j���[���擾
	static iTJSDispatch2* GetParentMenu(iTJSDispatch2 *obj) {
		tTJSVariant val;
		obj->PropGet(0, TJS_W("parent"), 0, &val, obj);
		return val.AsObjectNoAddRef();
	}
	
	// �C���f�b�N�X���擾
	static UINT GetIndex(iTJSDispatch2 *obj) {
		tTJSVariant val;
		obj->PropGet(0, TJS_W("index"), 0, &val, obj);
		return (UINT)val.AsInteger();
	}
	// �E�B���h�E���擾
	static HWND GetHWND(iTJSDispatch2 *obj) {
		if (!obj) return NULL;
		tTJSVariant val;
		obj->PropGet(0, TJS_W("root"), 0, &val, obj);
		obj = val.AsObjectNoAddRef();
		if (!obj) return NULL;
		val.Clear();
		obj->PropGet(0, TJS_W("window"), 0, &val, obj);
		obj = val.AsObjectNoAddRef();
		return obj ? WindowEx::GetHWND(obj) : NULL;
	}

	static bool getMenuItemInfo(iTJSDispatch2 *obj, HMENU &hmenu, UINT &index, MENUITEMINFO &mi, UINT mask) {
		hmenu = GetHMENU(GetParentMenu(obj));
		if (hmenu == NULL) TVPThrowExceptionMessage(TJS_W("Cannot get parent menu."));
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = mask;
		index = GetIndex(obj);
		return !!::GetMenuItemInfo(hmenu, index, TRUE, &mi);
	}

	// property rightJustify
	static tjs_error TJS_INTF_METHOD getRightJustify(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HMENU hmenu;
		UINT index;
		MENUITEMINFO mi;
		r->Clear();
		if (getMenuItemInfo(obj, hmenu, index, mi, MIIM_FTYPE))
			*r = !!(mi.fType & MFT_RIGHTJUSTIFY);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD setRightJustify(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HMENU hmenu;
		UINT index;
		MENUITEMINFO mi;
		if (getMenuItemInfo(obj, hmenu, index, mi, MIIM_FTYPE)) {
			if (p[0]->AsInteger()) mi.fType |= MFT_RIGHTJUSTIFY;
			else                   mi.fType &= MFT_RIGHTJUSTIFY ^ (~0L);
			::SetMenuItemInfo(hmenu, index, TRUE, &mi);
			::DrawMenuBar(GetHWND(obj));
		}
		return TJS_S_OK;
	}

	// property bmpItem
	static tjs_error TJS_INTF_METHOD getBmpItem(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HMENU hmenu;
		UINT index;
		MENUITEMINFO mi;
		r->Clear();
		if (getMenuItemInfo(obj, hmenu, index, mi, MIIM_BITMAP))
			*r = (tjs_int)(mi.hbmpItem);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD setBmpItem(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HMENU hmenu;
		UINT index;
		MENUITEMINFO mi;
		if (getMenuItemInfo(obj, hmenu, index, mi, MIIM_BITMAP)) {
			mi.hbmpItem = (HBITMAP)(p[0]->AsInteger());
			::SetMenuItemInfo(hmenu, index, TRUE, &mi);
			::DrawMenuBar(GetHWND(obj));
		}
		return TJS_S_OK;
	}

};
NCB_ATTACH_CLASS(MenuItemEx, MenuItem)
{
	Variant(TJS_W("biSystem"),           (tjs_int)HBMMENU_SYSTEM);
	Variant(TJS_W("biRestore"),          (tjs_int)HBMMENU_MBAR_RESTORE);
	Variant(TJS_W("biMinimize"),         (tjs_int)HBMMENU_MBAR_MINIMIZE);
	Variant(TJS_W("biClose"),            (tjs_int)HBMMENU_MBAR_CLOSE);
	Variant(TJS_W("biCloseDisabled"),    (tjs_int)HBMMENU_MBAR_CLOSE_D);
	Variant(TJS_W("biMinimizeDisabled"), (tjs_int)HBMMENU_MBAR_MINIMIZE_D);
	Variant(TJS_W("biPopupClose"),       (tjs_int)HBMMENU_POPUP_CLOSE);
	Variant(TJS_W("biPopupRestore"),     (tjs_int)HBMMENU_POPUP_RESTORE);
	Variant(TJS_W("biPopupMaximize"),    (tjs_int)HBMMENU_POPUP_MAXIMIZE);
	Variant(TJS_W("biPopupMinimize"),    (tjs_int)HBMMENU_POPUP_MINIMIZE);

	NCB_PROPRETY_RAW_CALLBACK(rightJustify, ClassT::getRightJustify, ClassT::setRightJustify, 0);
	NCB_PROPRETY_RAW_CALLBACK(bmpItem,      ClassT::getBmpItem,      ClassT::setBmpItem,      0);
}


////////////////////////////////////////////////////////////////
struct ConsoleEx
{
	struct SearchWork {
		ttstr name;
		HWND result;
	};
#define CLASSNAME_MAX 1024
	static BOOL CALLBACK SearchWindowClass(HWND hwnd, LPARAM lp) {
		SearchWork *wk = (SearchWork*)lp;
		tjs_char name[CLASSNAME_MAX];
		::GetClassNameW(hwnd, name, CLASSNAME_MAX);
		name[CLASSNAME_MAX-1] = 0;
		if (wk->name == ttstr(name)) {
			wk->result = hwnd;
			return FALSE;
		}
		return TRUE;
	}
	static HWND GetHWND() {
		SearchWork wk = { TJS_W("TTVPConsoleForm"), NULL };
		::EnumThreadWindows(GetCurrentThreadId(), SearchWindowClass, (LPARAM)&wk);
		return wk.result;
	}

	// showRestore
	static tjs_error TJS_INTF_METHOD showRestore(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		TVPExecuteExpression(ttstr(TJS_W("Debug.console.visible=1")));
		if (hwnd != NULL) PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		return TJS_S_OK;
	}
	// getWindowRect
	static tjs_error TJS_INTF_METHOD getWindowRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		RECT rect;
		HWND hwnd = GetHWND();
		r->Clear();
		if (hwnd != NULL && ::GetWindowRect(hwnd, &rect)) {
			ncbDictionaryAccessor dict;
			if (WindowEx::SetRect(dict, &rect)) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}
	// setPos
	static tjs_error TJS_INTF_METHOD setPos(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		if (hwnd != NULL) {
			UINT flag = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;
			int x, y, w, h;
			x = (int)p[0]->AsInteger();
			y = (int)p[1]->AsInteger();
			if (p[0]->Type() == tvtVoid &&
				p[1]->Type() == tvtVoid) flag |= SWP_NOMOVE;
			if (n >= 4) {
				w = (int)p[2]->AsInteger();
				h = (int)p[3]->AsInteger();
				if (p[2]->Type() == tvtVoid &&
					p[3]->Type() == tvtVoid) flag |= SWP_NOSIZE;
			} else                           flag |= SWP_NOSIZE;
			::SetWindowPos(hwnd, NULL, x, y, w, h, flag);
		}
		return TJS_S_OK;
	}
};
NCB_ATTACH_FUNCTION_WITHTAG(showRestore,   Debug_console, Debug.console, ConsoleEx::showRestore);
NCB_ATTACH_FUNCTION_WITHTAG(getWindowRect, Debug_console, Debug.console, ConsoleEx::getWindowRect);
NCB_ATTACH_FUNCTION_WITHTAG(setPos,        Debug_console, Debug.console, ConsoleEx::setPos);


////////////////////////////////////////////////////////////////

struct System
{
	//--------------------------------------------------------------
	// ���[�e�B���e�B

	// RECT�������ɕۑ�
	static bool SetDictRect(ncbDictionaryAccessor &dict, tjs_char const *name, LPCRECT lprect) {
		ncbDictionaryAccessor rdic;
		if (rdic.IsValid()) {
			WindowEx::SetRect(rdic, lprect);
			dict.SetValue(name, tTJSVariant(rdic, rdic));
			return true;
		}
		return false;
	}

	// ��������RECT���擾
	static void GetDictRect(LPRECT lprect, tTJSVariant **param) {
		lprect->left   = (LONG)param[0]->AsInteger();
		lprect->top    = (LONG)param[1]->AsInteger();
		lprect->right  = (LONG)param[2]->AsInteger();
		lprect->bottom = (LONG)param[3]->AsInteger();
		lprect->right  += lprect->left;
		lprect->bottom += lprect->top;
	}

	// ���j�^��񎫏��𐶐�
	static bool SetDictMonitorInfo(HMONITOR hMonitor, ncbDictionaryAccessor &dict) {
		MONITORINFOEXW mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		if (::GetMonitorInfoW(hMonitor, &mi)) {
			dict.SetValue(TJS_W("name"),    ttstr(mi.szDevice));
			dict.SetValue(TJS_W("primary"), !!(mi.dwFlags & MONITORINFOF_PRIMARY));
			SetDictRect(dict, TJS_W("monitor"), &mi.rcMonitor);
			SetDictRect(dict, TJS_W("work"),    &mi.rcWork);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	// EnumDisplayMonitors�R�[���o�b�N

	// �z��Ƀ��j�^��񎫏���ǉ�
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		ncbArrayAccessor *array = (ncbArrayAccessor*)dwData;
		ncbDictionaryAccessor dict;
		if (dict.IsValid() && SetDictMonitorInfo(hMonitor, dict)) {
			if (lprcMonitor) SetDictRect(dict, TJS_W("intersect"), lprcMonitor);
			array->SetValue(array->GetArrayCount(), tTJSVariant(dict, dict));
		}
		return TRUE;
	}

	// �v���C�}�����j�^�n���h�����擾
	static BOOL CALLBACK PrimaryGetProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		if (::GetMonitorInfo(hMonitor, &mi) && mi.dwFlags & MONITORINFOF_PRIMARY) {
			*(HMONITOR*)dwData = hMonitor;
			return FALSE;
		}
		return TRUE;
	}

	//--------------------------------------------------------------
	// �N���X�ǉ����\�b�h

	// System.getDisplayMonitors
	static tjs_error TJS_INTF_METHOD getDisplayMonitors(
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		RECT rect;
		bool useRect = false;

		// �p�����[�^���� 0 �܂��� 4�̂�
		switch (numparams) {
		case 0: break;
		case 4:
			GetDictRect(&rect, param);
			useRect = true;
			break;
		default: return TJS_E_BADPARAMCOUNT;
		}

		result->Clear();
		ncbArrayAccessor array;
		if (array.IsValid()) {
			if (::EnumDisplayMonitors(NULL, useRect ? &rect : NULL, &MonitorEnumProc, (LPARAM)&array))
				*result = tTJSVariant(array, array);
		}
		return TJS_S_OK;
	}

	// System.getMonitorInfo
	static tjs_error TJS_INTF_METHOD getMonitorInfo(
		tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		HMONITOR mon = NULL;
		DWORD flag = (numparams > 0 && param[0]->AsInteger()) ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTONULL;

		// �p�����[�^���ɂ���ē���ύX
		switch (numparams) {
		case 0: // �ȗ���
			::EnumDisplayMonitors(NULL, NULL, &PrimaryGetProc, (LPARAM)&mon);
			break;
		case 2: // window�w��
			if (param[1]->Type() == tvtObject && param[1]->IsInstanceOf(TJS_W("Window"))) {
				iTJSDispatch2 *obj = param[1]->AsObjectNoAddRef();
				if (!obj) return TJS_E_INVALIDPARAM;
				HWND hwnd = WindowEx::GetHWND(obj);
				mon = ::MonitorFromWindow(hwnd, flag);
			} else return TJS_E_INVALIDPARAM;
			break;
		case 3: // ���W�w��
			POINT pt;
			pt.x = (LONG)param[0]->AsInteger();
			pt.y = (LONG)param[1]->AsInteger();
			mon = ::MonitorFromPoint(pt, flag);
			break;
		case 5: // �͈͎w��
			RECT rect;
			GetDictRect(&rect, param+1);
			mon = ::MonitorFromRect(&rect, flag);
			break;
		default: return TJS_E_BADPARAMCOUNT;
		}

		result->Clear();
		if (mon != NULL) {
			ncbDictionaryAccessor dict;
			if (dict.IsValid() && SetDictMonitorInfo(mon, dict))
				*result = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}
};

// System�Ɋ֐���ǉ�
NCB_ATTACH_FUNCTION(getDisplayMonitors, System, System::getDisplayMonitors);
NCB_ATTACH_FUNCTION(getMonitorInfo,     System, System::getMonitorInfo);
