#pragma once


using namespace std;
extern Renderer g_Renderer; //tmp hack
class GridMesh;

#ifdef _DEBUG
//	#define DBG_RENDERDOC
#endif

enum class IOType
{
	Float, 
	Integer, 
	Color, 
	Bool, 
	String, 
	FloatOrColor
};

enum class IOSlotCategory
{
	Input,
	Output,
	Param
};

enum class ParamEdition
{
	Slider,
	LogSlider,
	EditBox,
	ColorPickerControl,
	CheckBox,
	ComboBox,
	FilePicker
};



class Map
{
public:
	Map() : m_Format( DXGI_FORMAT_UNKNOWN ), m_Width(0), m_Height(0) {}

	bool Init( uint32_t _resX, uint32_t _resY, DXGI_FORMAT _fmt, UINT _bind = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );
	bool Init( uint32_t _resolution, IOType _type, UINT _bind = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );
	
	void SetDebugName( const char* _name );

	inline DXGI_FORMAT GetFormat() const { return m_Format; }
	inline bool IsRGB() const { return m_bIsRGB; }

	inline uint32_t GetWidth() const { return m_Width; }
	inline uint32_t GetHeight() const { return m_Height; }

	ID3D11Texture2D*			GetTex() const { return m_pTex; }
	ID3D11ShaderResourceView*	GetSRV() const { return m_pSRV; }
	ID3D11UnorderedAccessView*	GetUAV() { return m_pUAV; }
	ID3D11RenderTargetView*		GetRTV() { return m_pRTV; }
	ID3D11DepthStencilView*		GetDSV() { return m_pDSV; }

	void CopyFromGPU( vector<float>& _data ) const;
	void CopyFromGPU( float* _data ) const;
	void CopyToGPU( const float* _pData );
	void CopyToGPU( const rgba8_t* _pData );
private:
	D3DObject< ID3D11Texture2D >			m_pTex;
	D3DObject< ID3D11ShaderResourceView >	m_pSRV;
	D3DObject< ID3D11UnorderedAccessView >	m_pUAV;
	D3DObject< ID3D11RenderTargetView >		m_pRTV;
	D3DObject< ID3D11DepthStencilView >		m_pDSV;

	uint32_t m_Width, m_Height;
	DXGI_FORMAT m_Format;
	bool m_bIsRGB;
};

#define COMPUTE_NODE_FACTORY( ClassName ) \
	virtual const char*  GetNodeClassName() const { return #ClassName; } \
	static ComputeNode* Instantiate() { return new ClassName; }

enum ResolutionReference
{
	Res_GlobalSetting,
	Res_MainInput,
	Res_Fixed
};

class ComputeNode
{
public:
	struct Slot
	{
		std::string				m_Name;
		IOType			m_DataType;
		UIRect			m_UIRect;
		bool			m_bOptional;
	};

	struct RemoteSlot
	{
		weak_ptr<ComputeNode> m_pNode;
		uint32_t	m_SlotIndex;
	};

	union ParamValue
	{
		float		f;
		int			i;
		bool		b;
		uint32_t			c;
	};
	

	struct ParamSlot : public Slot
	{
		std::string				m_CategoryName;
		ParamEdition	m_Edition;
		ParamValue		m_Value;
		std::string				m_ValueString; //only for IOType::String

		ParamValue		m_DefaultValue;
		float			m_Min, m_Max;

		vector< std::string >	m_Enum;

		RemoteSlot		m_RemoteParamSlot;

		bool			m_bCompileTimeShaderConstant;
	
		void AddEnum( const std::string& _name ) { m_Enum.push_back( _name ); }
	};


	struct InputSlot : public Slot
	{
		RemoteSlot	m_RemoteOutputSlot;
	};

	struct OutputSlot : public Slot
	{
		vector< RemoteSlot > m_RemoteInputSlots;
	};


public:
	ComputeNode();
	virtual ~ComputeNode() {}

	virtual bool OneTimeInit() { return true; }

	inline  const std::string&  GetName() const { return m_UIName; }
	virtual const char* GetNodeClassName() const = 0;

	void SetUIName( const std::string& _uiname ) { m_UIName = _uiname; }
	void SetPos( const Vec2& _pos ) { m_UIRect.Pos = _pos; }
	void SetSize( const Vec2& _size ) { m_UIRect.Size = _size;  }

	//Resolution

	inline uint32_t  GetResolution() const { return m_Resolution; }

		   bool SetFixedResolution( uint32_t _resolution );
		   bool SetResolutionReference( ResolutionReference _resolutionReference );
	inline ResolutionReference  GetResolutionReference() const { return m_ResolutionReference; }

	       bool SetResolutionModifier( int32_t _resolutionModifier );
	inline int32_t  GetResolutionModifier() const { return m_ResolutionModifier; }
	

	void UpdateInternalResolution();

	virtual void OnInputConnectionChanged( int _slot ) {}
	virtual void OnOutputConnectionChanged( int _slot ) {}

	//Misc

	inline bool IsDirty() const { return m_bIsDirty; }
	void SetDirty();

	//Load/Save

	virtual bool Load( const tinyxml2::XMLElement* _xmlNode );
	virtual tinyxml2::XMLElement* Save( tinyxml2::XMLDocument& _xmlDoc ) const;

	//Compute

	void Compute();

	//Simulation

	bool IsIterativeComputation() const { return m_bIsIterative; }
	virtual void InitSim() {}
	virtual void StepSim( bool _rebindResources, bool _unbindResourcesOnExit ) {}
	virtual bool RenderSimPreview( const GridMesh& _gridMesh ) { return false; }

	inline bool GetPreviewMode() const { return m_bPreviewAsHeightField; } //TODO enum //return true for heightfield, false for image

	//I/O

	void AddInput( const char* _name, IOType _type, bool _optional = false );
	void AddOutput( const char* _name, IOType _type );

	//Parameters

	int AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition, 
					float _defaultValue, float _min, float _max, bool _invalidatesShader = false );

	int AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
					int _defaultValue, int _min, int _max, bool _invalidatesShader = false );

	int AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
					const Color& _defaultColor, bool _invalidatesShader = false );

	int AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
					bool _defaultValue, bool _invalidatesShader = false );

	int AddParam(	const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition,
					const char* _defaultString, bool _invalidatesShader = false );

	//Misc

	virtual void OnCompileTimeShaderConstantChanged() {}
	
	const Map* GetRemoteInputMap( int _slot ) const;

	virtual const Map* GetOutput( uint32_t _idx ) const = 0;

	//UI

	virtual CDialogEx* GetCustomUI( CWnd* _pParent ) { return nullptr; }

protected:
	virtual void InternalCompute() = 0;

	virtual bool OnResolutionChanged() = 0;

	int AddParam( const char* _categoryName, const char* _name, IOType _type, ParamEdition _edition, bool _invalidatesShader );

protected:
	vector< InputSlot > m_InputSlots;
	vector< OutputSlot > m_OutputSlots;
	vector< ParamSlot > m_ParameterSlots;

	bool m_bIsIterative;

	//Resolution
	////////////

	uint32_t m_FixedResolution;
	int32_t m_ResolutionModifier;
	ResolutionReference m_ResolutionReference;

private:
	uint32_t m_Resolution;

	//Misc
	//////

	bool m_bIsDirty;

	//UI stuff
	//////////
	friend class NodeEditor;
	friend class NodeEditorView;
	friend class CPropertiesWnd;
	
	UIRect		m_UIRect;
	std::string			m_UIName;

public:
	bool m_bPreviewAsHeightField; //TODO enum //else previewed as B&W mask if single channel or image if RGB
};


