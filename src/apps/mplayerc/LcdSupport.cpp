/* $Id: LcdSupport.cpp,v 1.1 2006-11-04 22:41:51 wild Exp $
 *
 * Logitech LCD Support Class
 */

#include "stdafx.h"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>    // _beginthread, _endthread
#include <time.h>

#include "LcdSupport.h"

// make sure we use the library
#pragma comment(lib, "lgLcd.lib")

#define LCD_APP_NAME "Media Player Classic"
#define LCD_UPD_TIMER 40


void LCD_UpdateThread(void * Control) 
{
	_TCHAR str[40];
	__time64_t ltime;
	__time64_t otime = 0;
	struct tm  thetime;
	_tsetlocale(LC_ALL, _T(""));		// set current system locale

	while (((CMPC_Lcd *) Control)->Thread_Loop)
	{
		if (_time64(&ltime) != otime &&		// Retrieve the time
		    (ltime > ((CMPC_Lcd *) Control)->nThread_tTimeout || ltime < otime))	// message displayed, no timeupdate until timeout
		{
			otime = ltime;
			_localtime64_s(&thetime, &ltime);

			// Format the current time structure into a string
			// using %#x is the long date representation,
			// appropriate to the current locale
			if (_tcsftime(str, sizeof(str), _T("%X %#x"), (const struct tm *)&thetime))
				((CMPC_Lcd *) Control)->m_Manager.m_Text2.SetText(str);
		}

		((CMPC_Lcd *) Control)->m_Output.Update(GetTickCount());	// This invokes OnUpdate for the active screen
		((CMPC_Lcd *) Control)->m_Output.Draw();			// This invokes OnDraw for the active screen
		Sleep(LCD_UPD_TIMER);
	}

	_endthread();
}


/******************************************************************************************************
 ******************************************** CLCDMyManager ******************************************/

HRESULT CLCDMyManager::Initialize()
{
	LOGFONT lf;
	HFONT hFont;

	// max dims: 160 x 43
	// Initialize the text control (media)
	m_Text1.Initialize();
	m_Text1.SetOrigin(10, 0);
	m_Text1.SetSize(150, 13);
	m_Text1.SetAlignment(DT_CENTER);
	m_Text1.SetWordWrap(false);
	m_Text1.SetText(_T(""));
	m_Text1.SetStartDelay(5000);
	m_Text1.SetEndDelay(2000);
	m_Text1.EnableRepeat(true);
	m_Text1.SetScrollDirection(CLCDScrollingText::SCROLL_HORZ);
	m_Text1.SetSpeed(24);

	// Initialize the text control (time / mpc messages)
	m_Text2.Initialize();
	m_Text2.SetOrigin(10, 25);
	m_Text2.SetSize(150, 13);
	m_Text2.SetAlignment(DT_CENTER);
	m_Text2.SetWordWrap(false);
	m_Text2.SetText(_T(""));
	m_Text2.SetFontPointSize(7);
	hFont = m_Text2.GetFont();
	GetObject(hFont, sizeof(LOGFONT), &lf);
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Microsoft Sans Serif"));
	m_Text2.SetFont(lf);

	// Initialize the progressbar control (volume)
	m_ProgBar[0].Initialize();
	m_ProgBar[0].SetOrigin(0, 0);
	m_ProgBar[0].SetSize(7, 43);
	m_ProgBar[0].SetPos(0);
	m_ProgBar[0].SetProgressStyle(CLCDProgressBar::STYLE_FILLED_V);

	// Initialize the progressbar control (media progress)
	m_ProgBar[1].Initialize();
	m_ProgBar[1].SetOrigin(20, 15);
	m_ProgBar[1].SetSize(140, 7);
	m_ProgBar[1].SetPos(0);
	m_ProgBar[1].SetProgressStyle(CLCDProgressBar::STYLE_FILLED_H); /* fuer seeken benutzen: STYLE_DASHED_CURSOR */

	// gfx
	m_PlayState.Initialize();
	m_PlayState.SetOrigin(10, 15);
	m_PlayState.SetSize(7, 7);

	AddObject(&m_Text1);
	AddObject(&m_Text2);
	AddObject(&m_ProgBar[0]);
	AddObject(&m_ProgBar[1]);
	AddObject(&m_PlayState);

	return CLCDManager::Initialize();
}

void CLCDMyManager::OnLCDButtonUp(int nButton)
{
	switch(nButton)
	{
	case LGLCDBUTTON_BUTTON0://	break;
	{
/*		LOGFONT lf;
		HFONT hFont = m_Text2.GetFont();

		GetObject(hFont, sizeof(LOGFONT), &lf);

		CFontDialog cfd(&lf);
		if (cfd.DoModal() == IDOK)
		{
			cfd.GetCurrentFont(&lf);
			m_Text1.SetFont(lf);
		}
*/		break;
	}
	case LGLCDBUTTON_BUTTON1:	break;
	case LGLCDBUTTON_BUTTON2:	break;
	case LGLCDBUTTON_BUTTON3:	break;
	default:			break;
	}
}


/******************************************************************************************************
 ********************************************** CMPC_Lcd *********************************************/

/* attach to an available lcd */
CMPC_Lcd::CMPC_Lcd(void)
{
	BYTE bPause[] = {0x93, 0xFF,
	                 0x93, 0xFF,
	                 0x93, 0xFF,
	                 0x93, 0xFF,
	                 0x93, 0xFF,
	                 0x93, 0xFF,
	                 0x93, 0xFF};

	BYTE bStop[]  = {0x00, 0x00,
	                 0x00, 0x00,
	                 0x00, 0x00,
	                 0x00, 0x00,
	                 0x00, 0x00,
	                 0x00, 0x00,
	                 0x00, 0x00};

	BYTE bPlay[]  = {0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
	                 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	                 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	                 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	                 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	                 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	                 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F};

	hBmp[PS_PLAY]  = CreateBitmap(56, 7, 1, 1, bPlay);
	hBmp[PS_PAUSE] = CreateBitmap(14, 7, 1, 1, bPause);
	hBmp[PS_STOP]  = CreateBitmap( 8, 7, 1, 1, bStop);

	// lcd init
	ZeroMemory(&m_ConnCtx, sizeof(m_ConnCtx));

	m_ConnCtx.appFriendlyName		= _T(LCD_APP_NAME);
	m_ConnCtx.isPersistent			= FALSE;
	m_ConnCtx.isAutostartable		= FALSE;
	m_ConnCtx.onConfigure.configCallback	= NULL;		// we don't have a configuration screen
	m_ConnCtx.onConfigure.configContext	= NULL;
	m_ConnCtx.connection			= LGLCD_INVALID_CONNECTION;	// the "connection" member will be returned upon return

	if (m_Output.Initialize(&m_ConnCtx) != ERROR_SUCCESS ||	// Initialize the output object
	    m_Manager.Initialize() != ERROR_SUCCESS)
	{
		//_tperror(_T("Initialize"));
		return;
	}

	m_Manager.SetExpiration(INFINITE);	// Set the expiration on the sample screen

	// Add and lock the screen onto our output manager
	m_Output.AddScreen(&m_Manager);
	m_Output.LockScreen(&m_Manager);

	m_Output.Update(GetTickCount());	// This invokes OnUpdate for the active screen
	m_Output.Draw();			// This invokes OnDraw for the active screen

	hLCD_UpdateThread = 0;
	if (m_Output.IsOpened())
	{
		Thread_Loop = true;
		hLCD_UpdateThread = (HANDLE) _beginthread(LCD_UpdateThread, 512 /* stack */, (void*) this /* arg */);
	}
}

/* detach from lcd */
CMPC_Lcd::~CMPC_Lcd(void)
{
	Thread_Loop = false;
	WaitForSingleObject(hLCD_UpdateThread, LCD_UPD_TIMER * 2 /* timeout */);

	m_Output.Shutdown();
}

/* update title name */
void CMPC_Lcd::SetMediaTitle(const _TCHAR * text)
{
	m_Manager.m_Text1.SetText(text);
	m_Manager.m_ProgBar[1].SetPos(0);
}

/* set volume min/max */
void CMPC_Lcd::SetVolumeRange(__int64 nStart, __int64 nStop)
{
	m_Manager.m_ProgBar[0].SetRange(nStart, nStop);
}

/* update volume */
void CMPC_Lcd::SetVolume(__int64 nVol)
{
	m_Manager.m_ProgBar[0].SetPos(nVol);
}

/* set media min/max */
void CMPC_Lcd::SetMediaRange(__int64 nStart, __int64 nStop)
{
	m_Manager.m_ProgBar[1].SetRange(nStart, nStop);
}

/* update media position */
void CMPC_Lcd::SetMediaPos(__int64 nPos)
{
	m_Manager.m_ProgBar[1].SetPos(nPos);
}

/* update status message (displayed for nTimeOut milliseconds) */
void CMPC_Lcd::SetStatusMessage(const _TCHAR * text, int nTimeOut)
{
	if (!m_Output.IsOpened())
		return;

	__time64_t ltime;
	_time64(&ltime);
	if ((nTimeOut /= 1000) < 1)
		nTimeOut = 1;

	nThread_tTimeout = ltime + nTimeOut;
	m_Manager.m_Text2.SetText(text);
}

/* update play state bitmap */
void CMPC_Lcd::SetPlayState(CMPC_Lcd::PlayState ps)
{
	if (!m_Output.IsOpened())
		return;

	switch (ps)
	{
	case PS_PLAY:
		m_Output.SetAsForeground(true);
		m_Manager.m_PlayState.SetBitmap(hBmp[PS_PLAY]);
		m_Manager.m_PlayState.ResetUpdate();
		m_Manager.m_PlayState.SetSubpicWidth(7);
		m_Manager.m_PlayState.SetAnimationRate(300);
		break;

	case PS_PAUSE:
		m_Manager.m_PlayState.SetBitmap(hBmp[PS_PAUSE]);
		m_Manager.m_PlayState.ResetUpdate();
		m_Manager.m_PlayState.SetSubpicWidth(7);
		m_Manager.m_PlayState.SetAnimationRate(800);
		break;

	case PS_STOP:
		m_Output.SetAsForeground(false);
		m_Manager.m_ProgBar[1].SetPos(0);
		m_Manager.m_PlayState.SetBitmap(hBmp[PS_STOP]);
		m_Manager.m_PlayState.ResetUpdate();
		m_Manager.m_PlayState.SetSubpicWidth(7);
		m_Manager.m_PlayState.SetAnimationRate(5000);	// dummy
		break;

	default:
		break;
	}
}
