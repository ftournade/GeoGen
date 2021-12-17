#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "ComputeNode.h"

class DEMGrabFrame;

class CGeoGenApp : public CWinAppEx
{
public:
	CGeoGenApp();
	virtual ~CGeoGenApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	DEMGrabFrame* m_pDEMGrabberFrame;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnOpenSettings();
	DECLARE_MESSAGE_MAP()

public:
	void UpdatePropertyGrid( shared_ptr<ComputeNode> pHitNode );
	void RedrawPreview();
	void RedrawNodeEditor();
	void ClearPropertyList();

	void StartIterativeSim( shared_ptr<ComputeNode> _pErosionNode );
	void StopIterativeSim();

	inline u32 GetResolution() const	{ return m_Resolution; }
	inline int GetMinAltitude() const	{ return m_MinAltitude; }
	inline int GetMaxAltitude() const	{ return m_MaxAltitude; }
	inline int GetSeaLevel() const		{ return m_SeaLevel; }
	inline u32 GetTerrainExtent() const { return m_TerrainExtent; }

private:
	CString m_Filename;

public: //stop the accesor frenzy
	u32 m_Resolution;
	int m_MinAltitude;
	int m_MaxAltitude;
	int m_SeaLevel;
	u32 m_TerrainExtent;
	float m_CamFOV, m_CamNearClip, m_CamFarClip;
public:
	afx_msg void OnFileNew();
	afx_msg void OnFileLoad();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnOpenDEMGrabber();
};

extern CGeoGenApp theApp;
