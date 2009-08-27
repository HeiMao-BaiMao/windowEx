#include <windows.h>
#include "ncbind.hpp"

// �E�B���h�E�N���X���擾�p�̃o�b�t�@�T�C�Y
#define CLASSNAME_MAX 1024

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
#define EXEV_DISPCHG   TJS_W("onDisplayChanged")
#define EXEV_ENTERMENU TJS_W("onEnterMenuLoop")
#define EXEV_EXITMENU  TJS_W("onExitMenuLoop")
#define EXEV_ACTIVATE  TJS_W("onActivateChanged")
#define EXEV_SCREENSV  TJS_W("onScreenSave")
#define EXEV_MONITORPW TJS_W("onMonitorPower")

////////////////////////////////////////////////////////////////

struct WindowEx
{
	//--------------------------------------------------------------
	// ���[�e�B���e�B

	// �l�C�e�B�u�C���X�^���X�|�C���^���擾
	static inline WindowEx * GetInstance(iTJSDispatch2 *obj) {
		return ncbInstanceAdaptor<WindowEx>::GetNativeInstance(obj);
	}

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
	// ��������RECT�ɕۑ�
	static void GetRect(LPRECT rect, ncbPropAccessor &dict) {
		ncbTypedefs::Tag<LONG> LongTypeTag;
		rect->left   = dict.GetValue(TJS_W("x"), LongTypeTag);
		rect->top    = dict.GetValue(TJS_W("y"), LongTypeTag);
		rect->right  = dict.GetValue(TJS_W("w"), LongTypeTag);
		rect->bottom = dict.GetValue(TJS_W("h"), LongTypeTag);
		rect->right  += rect->left;
		rect->bottom += rect->top;
	}

	// SYSCOMMAND�𑗂�
	static tjs_error postSysCommand(iTJSDispatch2 *objthis, WPARAM param) {
		::PostMessage(GetHWND(objthis), WM_SYSCOMMAND, param, 0);
		return TJS_S_OK;
	}

	//--------------------------------------------------------------
	// �N���X�ǉ����\�b�h(RawCallback�`��)

	// minimize, maximize, showRestore
	static tjs_error TJS_INTF_METHOD minimize(   tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_MINIMIZE); }
	static tjs_error TJS_INTF_METHOD maximize(   tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_MAXIMIZE); }
	static tjs_error TJS_INTF_METHOD showRestore(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { return postSysCommand(obj, SC_RESTORE);  }

	// resetWindowIcon
	static tjs_error TJS_INTF_METHOD resetWindowIcon(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { 
		HWND hwnd = GetHWND(obj);
		if (hwnd != NULL) {
			HICON icon = ::LoadIcon(GetModuleHandle(0), IDI_APPLICATION);
			::PostMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
		}
		return TJS_S_OK;
	}

	// getWindowRect
	static tjs_error TJS_INTF_METHOD getWindowRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		RECT rect;
		HWND hwnd = GetHWND(obj);
		if (r) r->Clear();
		if (hwnd != NULL && ::GetWindowRect(hwnd, &rect)) {
			ncbDictionaryAccessor dict;
			if (SetRect(dict, &rect) && r) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}

	// getClientRect
	static tjs_error TJS_INTF_METHOD getClientRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		RECT rect;
		POINT zero = { 0, 0 };
		HWND hwnd = GetHWND(obj);
		if (r) r->Clear();
		if (hwnd != NULL && ::GetClientRect(hwnd, &rect)) {
			::ClientToScreen(hwnd, &zero);
			rect.left   += zero.x; rect.top    += zero.y;
			rect.right  += zero.x; rect.bottom += zero.y;
			ncbDictionaryAccessor dict;
			if (SetRect(dict, &rect) && r) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}

	// getNormalRect
	static tjs_error TJS_INTF_METHOD getNormalRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND(obj);
		WINDOWPLACEMENT place;
		ZeroMemory(&place, sizeof(place));
		place.length = sizeof(place);

		MONITORINFOEXW mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);

		if (r) r->Clear();
		if (hwnd != NULL && ::GetWindowPlacement(hwnd, &place)) {
			RECT *rc = &place.rcNormalPosition;
			ncbDictionaryAccessor dict;
			ncbPropAccessor   prop(obj);
			bool nofix = (n > 0 && !!p[0]->AsInteger());
			if (!nofix && ::GetMonitorInfoW(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi)) {
				LONG ox = mi.rcWork.left - mi.rcMonitor.left;
				LONG oy = mi.rcWork.top  - mi.rcMonitor.top;
				rc->left += ox, rc->right  += ox;
				rc->top  += oy, rc->bottom += oy;
			}
			if (SetRect(dict, rc) && r) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}

	// property maximized
	static bool isMaximized(iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND(obj);
		return (hwnd != NULL && ::IsZoomed(hwnd));
	}
	static tjs_error TJS_INTF_METHOD getMaximized(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (r) *r = isMaximized(obj);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD setMaximized(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		bool m = !!p[0]->AsInteger();
		if (m != isMaximized(obj)) postSysCommand(obj, m ? SC_MAXIMIZE : SC_RESTORE);
		return TJS_S_OK;
	}

	// property disableResize
	static tjs_error TJS_INTF_METHOD getDisableResize(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		if (r) *r = (self != NULL && self->disableResize);
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD setDisableResize(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		if (self == NULL) return TJS_E_ACCESSDENYED;
		self->disableResize = !!p[0]->AsInteger();
		return TJS_S_OK;
	}

	// setOverlayBitmap
	static tjs_error TJS_INTF_METHOD setOverlayBitmap(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		return (self != NULL) ? self->_setOverlayBitmap(n, p) : TJS_E_ACCESSDENYED;
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
				GetRect(rect, dict);
				return true;
			}
		}
		return false;
	}

	// ���b�Z�[�W����
	bool onMessage(tTVPWindowMessage *mes) {
		switch (mes->Msg) {
		case WM_SYSCOMMAND:
			switch (mes->WParam & 0xFFF0) {
			case SC_MAXIMIZE:     return callback(EXEV_QUERYMAX);
			case SC_SCREENSAVE:   return callback(EXEV_SCREENSV);
			case SC_MONITORPOWER: return callback(EXEV_MONITORPW, (int)mes->LParam, 0);
			}
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
			// �T�C�Y�ύX�J�[�\����}��
		case WM_NCHITTEST:
			if (disableResize) {
				HWND hwnd = GetHWND(self);
				if (hwnd != NULL) {
					LRESULT res = ::DefWindowProc(hwnd, mes->Msg, mes->WParam, mes->LParam);
					switch (res) {
						/**/             case HTLEFT:       case HTRIGHT:
					case HTTOP:       case HTTOPLEFT:    case HTTOPRIGHT:
					case HTBOTTOM: case HTBOTTOMLEFT: case HTBOTTOMRIGHT:
						res = HTBORDER;
						break;
					}
					mes->Result = res;
					return true;
				}
			}
			break;
			// �V�X�e�����j���[�T�C�Y�ύX�}��
		case WM_INITMENUPOPUP:
			if (HIWORD(mes->LParam)) {
				if (disableResize) {
					HWND hwnd = GetHWND(self);
					if (hwnd != NULL) mes->Result = ::DefWindowProc(hwnd, mes->Msg, mes->WParam, mes->LParam);
					::EnableMenuItem((HMENU)mes->WParam, SC_SIZE, MF_GRAYED | MF_BYCOMMAND);
					return (hwnd != NULL);
				}
			} else if (menuex) {
				mes->Result = ::DefWindowProc(GetHWND(self), mes->Msg, mes->WParam, mes->LParam);
				HMENU menu = (HMENU)mes->WParam;
				int cnt = ::GetMenuItemCount(menu);
				for (int i = 0; i < cnt; i++) checkUpdateMenuItem(menu, i, ::GetMenuItemID(menu, i));
				return true;
			}
			break;

			// ���j���[�J�n�I��
		case WM_ENTERMENULOOP:
			callback(EXEV_ENTERMENU);
			break;
		case WM_EXITMENULOOP:
			callback(EXEV_EXITMENU);
			break;

			// �f�B�X�v���C���[�h�ύX�ʒm
		case WM_DISPLAYCHANGE:
			callback(EXEV_DISPCHG);
			break;

		case WM_ACTIVATE:
			return callback(EXEV_ACTIVATE, (int)(mes->WParam & 0xFFFF), (int)((mes->WParam >> 16) & 0xFFFF));

		case TVP_WM_DETACH:
			deleteOverlayBitmap();
			break;
		}
		return false;
	}

	// ���j���[�X�V�����iMenuItemEx�p�j
	void checkUpdateMenuItem(HMENU, int, UINT);
	void setMenuItemID(iTJSDispatch2*, UINT, bool);

	// ���b�Z�[�W���V�[�o
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *mes) {
		WindowEx *inst = GetInstance((iTJSDispatch2*)userdata);
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
		:   self(obj), menuex(0),
			hasResizing(false),
			hasMoving(false),
			hasMove(false),
			disableResize(false),
			ovbmp(NULL)
		{ regist(true); }

	~WindowEx() {
		if (menuex) menuex->Release();
		deleteOverlayBitmap();
		regist(false);
	}

	void checkExEvents() {
		hasResizing = hasMember(EXEV_RESIZING);
		hasMoving   = hasMember(EXEV_MOVING);
		hasMove     = hasMember(EXEV_MOVE);
	}
	void deleteOverlayBitmap() {
		if (ovbmp) delete ovbmp;
		ovbmp = NULL;
	}

protected:
	tjs_error _setOverlayBitmap(tjs_int n, tTJSVariant **p) {
		if (ovbmp) ovbmp->hide();
		if (n > 0 && p[0]->Type() == tvtObject) {
			if (!ovbmp) {
				ovbmp = new OverlayBitmap();
				if (!ovbmp) return TJS_E_FAIL;
			}
			if (!ovbmp->setBitmap(self, p[0]->AsObjectNoAddRef())) {
				deleteOverlayBitmap();
				return TJS_E_FAIL;
			}
		}
		return TJS_S_OK;
	}
private:
	iTJSDispatch2 *self, *menuex;
	bool hasResizing, hasMoving, hasMove; //< ���\�b�h�����݂��邩�t���O
	bool disableResize; // �T�C�Y�ύX�֎~
public:
	struct OverlayBitmap {
		OverlayBitmap() : overlay(0), bitmap(0), bmpdc(0) {}
		~OverlayBitmap() {
			if (overlay) ::DestroyWindow(overlay);
			removeBitmap();
		}
		static BOOL CALLBACK SearchScrollBox(HWND hwnd, LPARAM lp) {
			HWND *result = (HWND*)lp;
			tjs_char name[CLASSNAME_MAX];
			::GetClassNameW(hwnd, name, CLASSNAME_MAX);
			if (ttstr(name) == TJS_W("TScrollBox")) {
				*result = hwnd;
				return FALSE;
			}
			return TRUE;
		}
		static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
			OverlayBitmap *self = reinterpret_cast<OverlayBitmap*>(::GetWindowLong(hwnd, GWL_USERDATA));
			if (self != NULL && msg == WM_PAINT) {
				self->onPaint(hwnd);
				return 0;
			}
			return DefWindowProc(hwnd, msg, wp, lp);
		}
		void drawBitmap(HDC dc) {
			if (bitmap) ::BitBlt(dc, bmpx, bmpy, bmpw, bmph, bmpdc, 0, 0, SRCCOPY);
		}
		void removeBitmap() {
			if (bmpdc)  ::DeleteDC(bmpdc);
			if (bitmap) ::DeleteObject(bitmap);
			bmpdc  = NULL;
			bitmap = NULL;
		}
		void onPaint(HWND hwnd) {
			PAINTSTRUCT ps;
			drawBitmap(::BeginPaint(hwnd, &ps));
			::EndPaint(hwnd, &ps);
		}
		static HBITMAP CopyLayerToBitmap(HDC dc, int ts, iTJSDispatch2 *lay, tjs_int &w, tjs_int &h, tjs_int *x=0, tjs_int *y=0) {
			typedef unsigned char PIX;
			ncbPropAccessor obj(lay);
			if (x) *x = obj.getIntValue(TJS_W("left"));
			if (y) *y = obj.getIntValue(TJS_W("top"));
			w = obj.getIntValue(TJS_W("imageWidth"));
			h = obj.getIntValue(TJS_W("imageHeight"));
			tjs_int ln = obj.getIntValue(TJS_W("mainImageBufferPitch"));
			PIX *pw, *pr = reinterpret_cast<unsigned char *>(obj.getIntValue(TJS_W("mainImageBuffer")));

			BITMAPINFO info;
			ZeroMemory(&info, sizeof(info));
			info.bmiHeader.biSize = sizeof(BITMAPINFO);
			info.bmiHeader.biWidth = w;
			info.bmiHeader.biHeight = h;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = (ts > 0) ? 32 : 24;

			HBITMAP bmp = ::CreateDIBSection(dc, (LPBITMAPINFO)&info, DIB_RGB_COLORS, (LPVOID*)&pw, NULL, 0);
			if (bmp && pw) {
				if (ts > 0) {
					PIX pts = (PIX)ts;
					for (int y = h-1; y >= 0; y--) {
						PIX *src = pr + (y * ln);
						PIX *dst = pw + ((h-1 - y) * ((w*4+3) & ~3L));
						for (int n = w-1; n >= 0; n--, src+=4, dst+=4) {
							dst[0] = src[0];
							dst[1] = src[1];
							dst[2] = src[2];
							dst[3] =(src[3] >= pts) ? 255: 0;
						}
					}
				} else {
					for (int y = h-1; y >= 0; y--) {
						PIX *src = pr + (y * ln);
						PIX *dst = pw + ((h-1 - y) * ((w*3+3) & ~3L));
						for (int n = w-1; n >= 0; n--, src+=4, dst+=3) {
							dst[0] = src[0];
							dst[1] = src[1];
							dst[2] = src[2];
						}
					}
				}
			}
			return bmp;
		}
		bool setBitmap(iTJSDispatch2 *win, iTJSDispatch2 *lay) {
			if (!lay || !lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay)) return false;

			if (!initOverlay()) return false;
			HWND base = GetHWND(win);
			HWND hwnd = NULL;
			::EnumChildWindows(base, SearchScrollBox, (LPARAM)&hwnd);
			if (!hwnd) return false;

			removeBitmap();
			HDC dc = ::GetDC(overlay);
			bmpdc  = ::CreateCompatibleDC(dc);
			bitmap = CopyLayerToBitmap(dc, 0, lay, bmpw, bmph, &bmpx, &bmpy);
			if (bitmap) ::SelectObject(bmpdc, bitmap);
			if (!bitmap || !bmpdc) {
				removeBitmap();
				::ReleaseDC(overlay, dc);
				return false;
			}

			RECT rect;
			::GetWindowRect(hwnd, &rect);
			::SetParent(     overlay, hwnd);
			::SetWindowPos(  overlay, HWND_TOP, 0, 0, (rect.right-rect.left), (rect.bottom-rect.top), 0); //SWP_HIDEWINDOW);

			::InvalidateRect(overlay, NULL, TRUE);
			::UpdateWindow(  overlay);
			::ShowWindow(    overlay, SW_SHOWNORMAL);
			drawBitmap(dc);

			::ReleaseDC(overlay, dc);
			return true;
		}
		bool initOverlay() {
			HINSTANCE hinst = ::GetModuleHandle(NULL);
			if (!hinst) return false;
			if (!WindowClass) {
				WNDCLASSEXW wcex = {
					/*size*/sizeof(WNDCLASSEX), /*style*/CS_PARENTDC|CS_VREDRAW|CS_HREDRAW, /*proc*/WndProc, /*extra*/0L,0L, /*hinst*/hinst,
					/*icon*/NULL, /*cursor*/::LoadCursor(NULL, IDC_ARROW), /*brush*/(HBRUSH)::GetStockObject(BLACK_BRUSH), /*menu*/NULL,
					/*class*/TJS_W("WindowEx OverlayBitmap Window Class"), /*smicon*/NULL };
				WindowClass = ::RegisterClassExW(&wcex);
				if (!WindowClass) return false;
			}
			if (!overlay) {
				overlay = ::CreateWindowExW(WS_EX_TOPMOST, (LPCWSTR)MAKELONG(WindowClass, 0), TJS_W("WindowExOverlayBitmap"),
											WS_CHILDWINDOW, 0, 0, 1, 1, TVPGetApplicationWindowHandle(), NULL, hinst, NULL);
				if (!overlay) return false;
				::SetWindowLong(overlay, GWL_USERDATA, (LONG)this);
			}
			return true;
		}
		void show() const { if (overlay) ::ShowWindow(overlay, SW_SHOWNORMAL); }
		void hide() const { if (overlay) ::ShowWindow(overlay, SW_HIDE);       }

	private:
		HWND overlay;
		HBITMAP bitmap;
		HDC bmpdc;
		tjs_int bmpx, bmpy, bmpw, bmph;
		static ATOM WindowClass;
	} *ovbmp;
};
ATOM WindowEx::OverlayBitmap::WindowClass = 0;

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
NCB_ATTACH_CLASS_WITH_HOOK(WindowEx, Window)
{
	RawCallback(TJS_W("minimize"),            &Class::minimize,          0);
	RawCallback(TJS_W("maximize"),            &Class::maximize,          0);
	RawCallback(TJS_W("maximized"),           &Class::getMaximized,      &Class::setMaximized, 0);
	RawCallback(TJS_W("showRestore"),         &Class::showRestore,       0);
	RawCallback(TJS_W("resetWindowIcon"),     &Class::resetWindowIcon,   0);
	RawCallback(TJS_W("getWindowRect"),       &Class::getWindowRect,     0);
	RawCallback(TJS_W("getClientRect"),       &Class::getClientRect,     0);
	RawCallback(TJS_W("getNormalRect"),       &Class::getNormalRect,     0);
	RawCallback(TJS_W("disableResize"),       &Class::setDisableResize,  &Class::setDisableResize, 0);
	RawCallback(TJS_W("setOverlayBitmap"),    &Class::setOverlayBitmap,  0);

	Method(     TJS_W("registerExEvent"),     &Class::checkExEvents);
}

////////////////////////////////////////////////////////////////
struct MenuItemEx
{
	enum { BMP_ITEM, BMP_CHK, BMP_UNCHK, BMP_MAX };
	enum { BMT_NONE, BMT_SYS, BMT_BMP };

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
	// ���[�g���j���[�̎q���ǂ���
	static bool IsRootChild(iTJSDispatch2 *obj) {
		tTJSVariant par, root;
		obj->PropGet(0, TJS_W("parent"), 0, &par,  obj);
		obj->PropGet(0, TJS_W("root"),   0, &root, obj);
		iTJSDispatch2 *p =  par.AsObjectNoAddRef();
		iTJSDispatch2 *r = root.AsObjectNoAddRef();
		return (p && r && p == r);
	}
	// �i�D�L����i�Łj�C���f�b�N�X���擾
	static UINT GetIndex(iTJSDispatch2 *obj, iTJSDispatch2 *parent) {
		tTJSVariant val, child;
		parent->PropGet(0, TJS_W("children"), 0, &child, parent);
		ncbPropAccessor charr(child);
		if (!charr.IsValid()) return (UINT)-1;

		obj->PropGet(0, TJS_W("index"), 0, &val, obj);
		int max = (int)val.AsInteger();
		UINT ret = (UINT)max;
		for (int i = 0; i <= max; i++) {
			tTJSVariant vitem;
			if (charr.checkVariant(i, vitem)) {
				ncbPropAccessor item(vitem);
				if (item.IsValid()) {
					// ��\���̏ꍇ�̓J�E���g����Ȃ�
					if (!item.getIntValue(TJS_W("visible"))) {
						if (i == max) return (UINT)-1;
						ret--;
					}
				}
			}
		}
		return ret;
	}
	// �E�B���h�E���擾
	static iTJSDispatch2* GetWindow(iTJSDispatch2 *obj) {
		if (!obj) return NULL;
		tTJSVariant val;
		obj->PropGet(0, TJS_W("root"), 0, &val, obj);
		obj = val.AsObjectNoAddRef();
		if (!obj) return NULL;
		val.Clear();
		obj->PropGet(0, TJS_W("window"), 0, &val, obj);
		return val.AsObjectNoAddRef();
	}
	static HWND GetHWND(iTJSDispatch2 *obj) {
		iTJSDispatch2 *win = GetWindow(obj);
		return win ? WindowEx::GetHWND(win) : NULL;
	}

	// property rightJustify
	tjs_int getRightJustify() const { return rj > 0; }
	void setRightJustify(tTJSVariant v) {
		rj = !!v.AsInteger();
		updateMenuItemInfo();
		if (IsRootChild(obj)) ::DrawMenuBar(GetHWND(obj));
	}

	// property bmpItem
	tjs_int getBmpItem() const { return getBmpSelect(BMP_ITEM); }
	void setBmpItem(tTJSVariant v) { setBmpSelect(v, BMP_ITEM); }

	// property bmpChecked
	tjs_int getBmpChecked() const { return getBmpSelect(BMP_CHK); }
	void setBmpChecked(tTJSVariant v) { setBmpSelect(v, BMP_CHK); }

	// property bmpUnchecked
	tjs_int getBmpUnchecked() const { return getBmpSelect(BMP_UNCHK); }
	void setBmpUnchecked(tTJSVariant v) { setBmpSelect(v, BMP_UNCHK); }

	tjs_int getBmpSelect(int sel) const {
		switch (bmptype[sel]) {
		case BMT_SYS: return (tjs_int)bitmap[sel];
		case BMT_BMP: return -1;
		default:      return 0;
		}
	}
	void setBmpSelect(tTJSVariant &v, int sel) {
		removeBitmap(sel);
		switch (v.Type()) {
		case tvtVoid:
		case tvtInteger:
		case tvtString:
			bmptype[sel] = BMT_SYS;
			bitmap[sel] = (HBITMAP)v.AsInteger();
			break;
		case tvtObject:
			iTJSDispatch2 *lay = v.AsObjectNoAddRef();
			if (!lay || !lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))
				TVPThrowExceptionMessage(TJS_W("no layer object."));
			tjs_int w, h;
			HWND hwnd = GetHWND(obj);
			HDC dc = ::GetDC(hwnd);
			bitmap[sel] = WindowEx::OverlayBitmap::CopyLayerToBitmap(dc, 64, lay, w, h);
			::ReleaseDC(hwnd, dc);
			bmptype[sel] = BMT_BMP;
		}
		updateMenuItemInfo();
		if (IsRootChild(obj)) ::DrawMenuBar(GetHWND(obj));
	}
	void removeBitmap(int sel) {
		if (bitmap[sel] && bmptype[sel] == BMT_BMP) ::DeleteObject(bitmap[sel]);
		bmptype[sel] = BMT_NONE;
		bitmap[ sel] = NULL;
	}

	UINT setMenuItemInfo(HMENU hmenu, int index_or_id, BOOL is_index) {
		MENUITEMINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID | MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_FTYPE;
		if (::GetMenuItemInfo(hmenu, index_or_id, is_index, &mi)) {
			if (bmptype[BMP_ITEM ]) mi.hbmpItem      = bitmap[BMP_ITEM ];
			if (bmptype[BMP_CHK  ]) mi.hbmpChecked   = bitmap[BMP_CHK  ];
			if (bmptype[BMP_UNCHK]) mi.hbmpUnchecked = bitmap[BMP_UNCHK];
			if (rj > 0)   mi.fType |= MFT_RIGHTJUSTIFY;
			else if (!rj) mi.fType &= MFT_RIGHTJUSTIFY ^ (~0L);
			::SetMenuItemInfo(hmenu, index_or_id, is_index, &mi);
			return mi.wID;
		}
		return 0;
	}
	void updateMenuItemInfo() {
		iTJSDispatch2 *parent = GetParentMenu(obj);
		HMENU hmenu = GetHMENU(parent);
		if (hmenu == NULL) TVPThrowExceptionMessage(TJS_W("Cannot get parent menu."));

		if (!id || !setMenuItemInfo(hmenu, id, FALSE)) {
			if (setMenuItemInfo(hmenu, GetIndex(obj, parent), TRUE))
				updateMenuItemID();
		}
	}

	void updateMenuItemID() {
		if (id != 0) setMenuItemID(false);
		iTJSDispatch2 *parent = GetParentMenu(obj);
		HMENU hmenu = GetHMENU(parent);
		UINT index = GetIndex(obj, parent);
		MENUITEMINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_ID | MIIM_DATA;
		if (::GetMenuItemInfo(hmenu, index, TRUE, &mi)) {
			HMENU test = GetHMENU(obj);
			DWORD check = mi.dwItemData;
			id = mi.wID;
			setMenuItemID(true);
		} else id = 0;
	}
	void setMenuItemID(bool isset) {
		iTJSDispatch2 *win = GetWindow(obj);
		if (win) {
			WindowEx *wex = WindowEx::GetInstance(win);
			if (wex)  wex->setMenuItemID(obj, id, isset);
		}
	}

	MenuItemEx(iTJSDispatch2* _obj) : obj(_obj), id(0), rj(-1) {
		for (int i = 0; i < BMP_MAX; i++) bitmap[i] = NULL, bmptype[i] = 0;
		updateMenuItemID();
	}
	~MenuItemEx() {
		for (int i = 0; i < BMP_MAX; i++) removeBitmap(i);
		setMenuItemID(false);
	}
private:
	iTJSDispatch2 *obj;
	UINT id;
	tjs_int  rj;
	int    bmptype[BMP_MAX];
	HBITMAP bitmap[BMP_MAX];
};
NCB_GET_INSTANCE_HOOK(MenuItemEx)
{
	/**/  NCB_GET_INSTANCE_HOOK_CLASS () {}
	/**/ ~NCB_GET_INSTANCE_HOOK_CLASS () {}
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) SetNativeInstance(objthis, (obj = new ClassT(objthis)));
		return obj;
	}
};
NCB_ATTACH_CLASS_WITH_HOOK(MenuItemEx, MenuItem)
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

	Property(TJS_W("rightJustify"), &Class::getRightJustify, &Class::setRightJustify);
	Property(TJS_W("bmpItem"),      &Class::getBmpItem,      &Class::setBmpItem     );
	Property(TJS_W("bmpChecked"),   &Class::getBmpChecked,   &Class::setBmpChecked  );
	Property(TJS_W("bmpUnchecked"), &Class::getBmpUnchecked, &Class::setBmpUnchecked);
}


void WindowEx::checkUpdateMenuItem(HMENU menu, int pos, UINT id) {
	if (id == 0 || id == (UINT)-1) return;

	ttstr idstr((tjs_int)(id));
	tTJSVariant var;

	tjs_error chk = menuex->PropGet(TJS_MEMBERMUSTEXIST, idstr.c_str(), idstr.GetHint(),  &var, menuex);
	if (TJS_SUCCEEDED(chk) && var.Type() == tvtObject) {
		iTJSDispatch2 *obj = var.AsObjectNoAddRef();
		MenuItemEx *ex = ncbInstanceAdaptor<MenuItemEx>::GetNativeInstance(obj);
		if (ex != NULL) ex->setMenuItemInfo(menu, pos, TRUE);
	}
}
void WindowEx::setMenuItemID(iTJSDispatch2* obj, UINT id, bool set) {
	if (id == 0 || id == (UINT)-1) return;

	ttstr idstr((tjs_int)(id));
	tTJSVariant var(obj, obj);

	if (!menuex) menuex = TJSCreateDictionaryObject();
	menuex->PropSet(TJS_MEMBERENSURE, idstr.c_str(), idstr.GetHint(),  &var, menuex);
}


////////////////////////////////////////////////////////////////
struct ConsoleEx
{
	struct SearchWork {
		ttstr name;
		HWND result;
	};
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

	// restoreMaximize
	static tjs_error TJS_INTF_METHOD restoreMaximize(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		bool hasWin = (hwnd != NULL);
		if ( hasWin && ::IsZoomed(hwnd)) PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		if (r) *r = hasWin;
		return TJS_S_OK;
	}
	static tjs_error TJS_INTF_METHOD maximize(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		bool hasWin = (hwnd != NULL);
		if ( hasWin && !::IsZoomed(hwnd)) PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		if (r) *r = hasWin;
		return TJS_S_OK;
	}
	// getPlacement
	static tjs_error TJS_INTF_METHOD getPlacement(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		WINDOWPLACEMENT place;
		ZeroMemory(&place, sizeof(place));
		place.length = sizeof(place);
		if (r) r->Clear();
		if (hwnd != NULL && ::GetWindowPlacement(hwnd, &place)) {
			ncbDictionaryAccessor dict;
			dict.SetValue(TJS_W("flags"       ), place.flags);
			dict.SetValue(TJS_W("showCmd"     ), place.showCmd);
			dict.SetValue(TJS_W("minLeft"     ), place.ptMinPosition.x);
			dict.SetValue(TJS_W("minTop"      ), place.ptMinPosition.y);
			dict.SetValue(TJS_W("maxLeft"     ), place.ptMaxPosition.x);
			dict.SetValue(TJS_W("maxTop"      ), place.ptMaxPosition.y);
			dict.SetValue(TJS_W("normalLeft"  ), place.rcNormalPosition.left);
			dict.SetValue(TJS_W("normalTop"   ), place.rcNormalPosition.top);
			dict.SetValue(TJS_W("normalRight" ), place.rcNormalPosition.right);
			dict.SetValue(TJS_W("normalBottom"), place.rcNormalPosition.bottom);
			if (r) *r = tTJSVariant(dict, dict);
		}
		return TJS_S_OK;
	}
	// setPlacement
	static tjs_error TJS_INTF_METHOD setPlacement(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (n < 1)return TJS_E_BADPARAMCOUNT;
		if (p[0]->Type() != tvtObject) return TJS_E_INVALIDPARAM;

		HWND hwnd = GetHWND();
		if (hwnd != NULL) {
			ncbPropAccessor dict(*p[0]);
			WINDOWPLACEMENT place;
			ZeroMemory(&place, sizeof(place));
			place.length                  = sizeof(place);
			place.flags                   = dict.getIntValue(TJS_W("flags"       ));
			place.showCmd                 = dict.getIntValue(TJS_W("showCmd"     ));
			place.ptMinPosition.x         = dict.getIntValue(TJS_W("minLeft"     ));
			place.ptMinPosition.y         = dict.getIntValue(TJS_W("minTop"      ));
			place.ptMaxPosition.x         = dict.getIntValue(TJS_W("maxLeft"     ));
			place.ptMaxPosition.y         = dict.getIntValue(TJS_W("maxTop"      ));
			place.rcNormalPosition.left   = dict.getIntValue(TJS_W("normalLeft"  ));
			place.rcNormalPosition.top    = dict.getIntValue(TJS_W("normalTop"   ));
			place.rcNormalPosition.right  = dict.getIntValue(TJS_W("normalRight" ));
			place.rcNormalPosition.bottom = dict.getIntValue(TJS_W("normalBottom"));
			if (r) *r = !!::SetWindowPlacement(hwnd, &place);
		} else if (r) *r = false;
		return TJS_S_OK;
	}
	// setPos
	static tjs_error TJS_INTF_METHOD setPos(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (n < 2) return TJS_E_BADPARAMCOUNT;
		HWND hwnd = GetHWND();
		if (hwnd != NULL) {
			UINT flag = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER;
			int x, y, w = 0, h = 0;
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
	static tjs_error TJS_INTF_METHOD bringAfter(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		HWND hwnd = GetHWND();
		if (hwnd != NULL) {
			HWND ins = NULL;
			if (n >= 1 && p[0]->Type() == tvtObject) {
				iTJSDispatch2 *win = p[0]->AsObjectNoAddRef();
				if (win && win->IsInstanceOf(0, 0, 0, TJS_W("Window"), win))
					ins = WindowEx::GetHWND(win);
			}
			::SetWindowPos(hwnd, ins, 0, 0, 0, 0,
						   SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		}
		return TJS_S_OK;
	}
};
NCB_ATTACH_FUNCTION_WITHTAG(restoreMaximize, Debug_console, Debug.console, ConsoleEx::restoreMaximize);
NCB_ATTACH_FUNCTION_WITHTAG(maximize,        Debug_console, Debug.console, ConsoleEx::maximize);
NCB_ATTACH_FUNCTION_WITHTAG(setPos,          Debug_console, Debug.console, ConsoleEx::setPos);
NCB_ATTACH_FUNCTION_WITHTAG(getPlacement,    Debug_console, Debug.console, ConsoleEx::getPlacement);
NCB_ATTACH_FUNCTION_WITHTAG(setPlacement,    Debug_console, Debug.console, ConsoleEx::setPlacement);
NCB_ATTACH_FUNCTION_WITHTAG(bringAfter,      Debug_console, Debug.console, ConsoleEx::bringAfter);


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
			iTJSDispatch2 *obj;
			if (param[1]->Type() != tvtObject) return TJS_E_INVALIDPARAM;
			obj = param[1]->AsObjectNoAddRef();
			if (obj && obj->IsInstanceOf(0, 0, 0, TJS_W("Window"), obj)) {
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

	// System.getMouseCursorPos
	static tjs_error TJS_INTF_METHOD getCursorPos(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		POINT pt = { 0, 0 };
		if (r) r->Clear();
		if (::GetCursorPos(&pt)) {
			ncbDictionaryAccessor dict;
			if (dict.IsValid()) {
				dict.SetValue(TJS_W("x"), pt.x);
				dict.SetValue(TJS_W("y"), pt.y);
				if (r) *r = tTJSVariant(dict, dict);
			}
		}
		return TJS_S_OK;
	}
	// System.setMouseCursorPos
	static tjs_error TJS_INTF_METHOD setCursorPos(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (n < 2) return TJS_E_BADPARAMCOUNT;
		BOOL rslt = ::SetCursorPos((int)p[0]->AsInteger(), (int)p[1]->AsInteger());
		if (r) {
			r->Clear();
			*r = rslt ? true : false;
		}
		return TJS_S_OK;
	}
	// System.getSystemMetrics
	static tjs_error TJS_INTF_METHOD getSystemMetrics(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;

		if (p[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;
		ttstr key(p[0]->AsString());
		if (key == TJS_W("")) return TJS_E_INVALIDPARAM;
		key.ToUppserCase();

		tTJSVariant tmp;
		iTJSDispatch2 *obj =  TVPGetScriptDispatch();
		bool hasval = TJS_SUCCEEDED(obj->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("System"), 0, &tmp, obj));
		obj->Release();
		if (!hasval) return TJS_E_FAIL;

		obj = tmp.AsObjectNoAddRef();
		tmp.Clear();
		if (TJS_FAILED(obj->PropGet(TJS_MEMBERMUSTEXIST, TJS_W("metrics"), 0, &tmp, obj))) {
			ncbDictionaryAccessor dict;
#define SM(key) dict.SetValue(TJS_W(# key), SM_ ## key)
			SM(ARRANGE);				SM(CLEANBOOT);				SM(CMONITORS);				SM(CMOUSEBUTTONS);
			SM(CXBORDER);				SM(CXCURSOR);				SM(CXDLGFRAME);				SM(CXDOUBLECLK);
			SM(CXDRAG);					SM(CXEDGE);					SM(CXFIXEDFRAME);			SM(CXFOCUSBORDER);
			SM(CXFRAME);				SM(CXFULLSCREEN);			SM(CXHSCROLL);				SM(CXHTHUMB);
			SM(CXICON);					SM(CXICONSPACING);			SM(CXMAXIMIZED);			SM(CXMAXTRACK);
			SM(CXMENUCHECK);			SM(CXMENUSIZE);				SM(CXMIN);					SM(CXMINIMIZED);
			SM(CXMINSPACING);			SM(CXMINTRACK);				SM(CXPADDEDBORDER);			SM(CXSCREEN);
			SM(CXSIZE);					SM(CXSIZEFRAME);			SM(CXSMICON);				SM(CXSMSIZE);
			SM(CXVIRTUALSCREEN);		SM(CXVSCROLL);				SM(CYBORDER);				SM(CYCAPTION);
			SM(CYCURSOR);				SM(CYDLGFRAME);				SM(CYDOUBLECLK);			SM(CYDRAG);
			SM(CYEDGE);					SM(CYFIXEDFRAME);			SM(CYFOCUSBORDER);			SM(CYFRAME);
			SM(CYFULLSCREEN);			SM(CYHSCROLL);				SM(CYICON);					SM(CYICONSPACING);
			SM(CYKANJIWINDOW);			SM(CYMAXIMIZED);			SM(CYMAXTRACK);				SM(CYMENU);
			SM(CYMENUCHECK);			SM(CYMENUSIZE);				SM(CYMIN);					SM(CYMINIMIZED);
			SM(CYMINSPACING);			SM(CYMINTRACK);				SM(CYSCREEN);				SM(CYSIZE);
			SM(CYSIZEFRAME);			SM(CYSMCAPTION);			SM(CYSMICON);				SM(CYSMSIZE);
			SM(CYVIRTUALSCREEN);		SM(CYVSCROLL);				SM(CYVTHUMB);				SM(DBCSENABLED);
			SM(DEBUG);					SM(IMMENABLED);				SM(MEDIACENTER);			SM(MENUDROPALIGNMENT);
			SM(MIDEASTENABLED);			SM(MOUSEPRESENT);			SM(MOUSEHORIZONTALWHEELPRESENT);	SM(MOUSEWHEELPRESENT);
			SM(NETWORK);				SM(PENWINDOWS);				SM(REMOTECONTROL);			SM(REMOTESESSION);
			SM(SAMEDISPLAYFORMAT);		SM(SECURE);					SM(SERVERR2);				SM(SHOWSOUNDS);
			SM(SHUTTINGDOWN);			SM(SLOWMACHINE);			SM(STARTER);				SM(SWAPBUTTON);
			SM(TABLETPC);				SM(XVIRTUALSCREEN);			SM(YVIRTUALSCREEN);
//			SM(DIGITIZER);
//			SM(MAXIMUMTOUCHES);
#undef SM
			tmp = dict;
			if (TJS_FAILED(obj->PropSet(TJS_MEMBERENSURE, TJS_W("metrics"), 0, &tmp, obj)))
				return TJS_E_FAIL;
		}
		ncbPropAccessor metrics(tmp);
		tjs_int num = metrics.getIntValue(key.c_str(), -1);
		if (num < 0) return TJS_E_INVALIDPARAM;
		*r = (tjs_int)::GetSystemMetrics((int)num);
		return TJS_S_OK;
	}

	// System.readEnvValue
	static tjs_error TJS_INTF_METHOD readEnvValue(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (p[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;
		ttstr name(p[0]->AsString());
		if (name == TJS_W("")) return TJS_E_INVALIDPARAM;

		r->Clear();
		DWORD len = ::GetEnvironmentVariableW(name.c_str(), NULL, 0);
		if (!len) return TJS_S_OK;

		tjs_char *tmp = new tjs_char[len];
		if (!tmp) return TJS_E_FAIL;
		ZeroMemory(tmp, len);
		DWORD res = ::GetEnvironmentVariableW(name.c_str(), tmp, len);
//		if (res != len-1) TVPAddImportantLog(TJS_W("���ϐ�������v���܂���"));
		*r = ttstr(tmp);
		delete[] tmp;
		return TJS_S_OK;
	}
	// System.expandEnvString
	static tjs_error TJS_INTF_METHOD expandEnvString(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		ttstr src(p[0]->AsString());

		r->Clear();
		DWORD len = ::ExpandEnvironmentStrings(src.c_str(), NULL, 0);
		if (!len) return TJS_E_FAIL;

		tjs_char *tmp = new tjs_char[len];
		if (!tmp) return TJS_E_FAIL;
		ZeroMemory(tmp, len);
		DWORD res = ::ExpandEnvironmentStrings(src.c_str(), tmp, len);
//		if (res != len) TVPAddImportantLog(TJS_W("�W�J������v���܂���"));
		*r = ttstr(tmp);
		delete[] tmp;
		return TJS_S_OK;
	}
};

// System�Ɋ֐���ǉ�
NCB_ATTACH_FUNCTION(getDisplayMonitors, System, System::getDisplayMonitors);
NCB_ATTACH_FUNCTION(getMonitorInfo,     System, System::getMonitorInfo);
NCB_ATTACH_FUNCTION(getCursorPos,       System, System::getCursorPos);
NCB_ATTACH_FUNCTION(setCursorPos,       System, System::setCursorPos);
NCB_ATTACH_FUNCTION(getSystemMetrics,   System, System::getSystemMetrics);
NCB_ATTACH_FUNCTION(readEnvValue,       System, System::readEnvValue);
NCB_ATTACH_FUNCTION(expandEnvString,    System, System::expandEnvString);

////////////////////////////////////////////////////////////////

struct Scripts
{
	static bool outputErrorLogOnEval;

	// property Scripts.outputErrorLogOnEval
	static bool   setEvalErrorLog(bool v) {
		bool ret = outputErrorLogOnEval;
		/**/       outputErrorLogOnEval = v;
		return ret;
	}

	// Scripts.eval �I�[�o�[���C�h
	static tjs_error TJS_INTF_METHOD eval(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *objthis) {
		if (outputErrorLogOnEval) return evalOrig->FuncCall(0, NULL, NULL, r, n, p, objthis);

		if(n < 1) return TJS_E_BADPARAMCOUNT;
		ttstr content = *p[0], name;
		tjs_int lineofs = 0;
		if(n >= 2) name    = *p[1];
		if(n >= 3) lineofs = *p[2];

		TVPExecuteExpression(content, name, lineofs, r);
		return TJS_S_OK;
	}
	// ���� Scripts.eval ��ۑ��E���A
	static void Regist() {
		tTJSVariant var;
		TVPExecuteExpression(TJS_W("Scripts.eval"), &var);
		evalOrig = var.AsObject();
	}
	static void UnRegist() {
		if (evalOrig) evalOrig->Release();
		evalOrig = NULL;
	}
	static iTJSDispatch2 *evalOrig;
};
iTJSDispatch2 * Scripts::evalOrig = NULL;  // Scripts.eval�̌��̃I�u�W�F�N�g
bool            Scripts::outputErrorLogOnEval = true; // �؂�ւ��t���O

// Scripts�Ɋ֐���ǉ�
NCB_ATTACH_FUNCTION(eval,            Scripts, Scripts::eval);
NCB_ATTACH_FUNCTION(setEvalErrorLog, Scripts, Scripts::setEvalErrorLog);

////////////////////////////////////////////////////////////////
// �R�[���o�b�N�w��

static void PreRegistCallback()
{
	Scripts::Regist();
}

static void PostUnregistCallback()
{
	Scripts::UnRegist();
}
NCB_PRE_REGIST_CALLBACK(      PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);

