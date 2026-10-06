#pragma once

//#include "ComputeNode.h"
#include "NormalNode.h" //TODO remove dependency

class NodeEditor //TODO merge NodeEditor into NodeEditorView
{
public:
	NodeEditor();
	~NodeEditor();

	bool Init();

	bool Load( const char* _filename );
	bool Save( const char* _filename ) const;

	shared_ptr<ComputeNode> MouseHitTest( const UIRect& _screenRect, const Vec2& _pos, int& _inputSlot, int& _outputSlot, int& _paramSlot ) const;

	void SetPreviewNode( shared_ptr<ComputeNode> _pNode );
	void LockUnlockPreview();

	void AddNode( shared_ptr<ComputeNode> _pNode );

	void SelectNode( shared_ptr<ComputeNode> _pNode, bool _addToSelection = false );
	void DeleteSelectedNodes();
	void DeleteAllNodes();

	weak_ptr<ComputeNode> GetPreviewedNode() { return m_pPreviewedNode; }

	bool IsPreviewingColorMap() const;

	ID3D11ShaderResourceView* GetPreviewHeight() const;
	ID3D11ShaderResourceView* GetPreviewNormal();
	ID3D11ShaderResourceView* GetPreviewAlbedo() const;
	ID3D11ShaderResourceView* GetPreviewWater() const;

	inline float GetZoom() const { return m_viewZoom; }
	inline void SetZoom( float _zoom ) { m_viewZoom = _zoom; ResizeFont(); }

	inline const Vec2& GetViewCenter() const { return m_ViewCenter;  }
	inline void SetViewCenter( const Vec2& _c ) { m_ViewCenter = _c; }

	Vec2 ScreenToLogicalPoint( const UIRect& _screenRect, const Vec2& _p ) const;
	Vec2 LogicalToScreenPoint( const UIRect& _screenRect, const Vec2& _p ) const;

	void DrawWithMFC( const CRect& _wndRect, CDC* dc );
	void DrawWithD3D11( const CRect& _wndRect );
//private:
	bool CreateLink(	shared_ptr<ComputeNode> _nodeA, uint32_t _slotIndexA, //source
						shared_ptr<ComputeNode> _nodeB, uint32_t _slotIndexB );//dest

	void OnResolutionChange();
protected:
	void ResizeFont();
	void DrawLink( CDC* _dc, const UIRect& _screenRect, const Vec2& _linkA, const Vec2& _linkB );

public: //Hate doing accessors for the sake of doing accessors

private:
	vector< shared_ptr<ComputeNode> > m_AllNodes;

	weak_ptr<ComputeNode> m_pPreviewedNode;
	vector<weak_ptr<ComputeNode>> m_SelectedNodes;

	shared_ptr<NormalNode> m_pNormalNode;

	bool m_bLockPreview;

	Vec2 m_ViewCenter;
	float m_viewZoom;

	//MFC display data

	CPen m_LinksPen, m_RGBLinksPen, m_NodesPen;
	CFont m_Font;

	//D3D11 display data
	
	struct D3D11Data
	{
		IDXGISwapChain* m_pSwapChain;
		RenderTarget* m_pBackBuffer;
		D3DObject< ID3D11Buffer > m_pDynamicVB;

		ID3D11PixelShader* m_pCurrentPS;

	} m_D3D11Data;
};

extern NodeEditor g_NodeEditor;

