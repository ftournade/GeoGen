#pragma once

#include <memory>

//Ref-counted owner of a COM (Direct3D) interface pointer, similar to ATL's CComPtr.
//operator& returns the address of the raw pointer so the object can be passed directly to
//Create*() calls (as an out parameter) and to *Set*() calls expecting an array of one pointer.
//It does NOT release the current pointer: call Release() first when re-creating the object.
template <class T>
class D3DObject
{
public:
	D3DObject() : m_p( nullptr ) {}

	D3DObject( const D3DObject& _other ) : m_p( _other.m_p )
	{
		if( m_p )
			m_p->AddRef();
	}

	D3DObject( D3DObject&& _other ) : m_p( _other.m_p )
	{
		_other.m_p = nullptr;
	}

	~D3DObject() { Release(); }

	D3DObject& operator=( const D3DObject& _other )
	{
		if( _other.m_p )
			_other.m_p->AddRef();

		Release();
		m_p = _other.m_p;

		return *this;
	}

	D3DObject& operator=( D3DObject&& _other )
	{
		if( this != std::addressof( _other ) )
		{
			Release();
			m_p = _other.m_p;
			_other.m_p = nullptr;
		}

		return *this;
	}

	void Release()
	{
		if( m_p )
		{
			m_p->Release();
			m_p = nullptr;
		}
	}

	inline T*	Get() const				{ return m_p; }
	inline		operator T*() const		{ return m_p; }
	inline T*	operator->() const		{ return m_p; }
	inline T**			operator&()			{ return &m_p; }
	inline T* const*	operator&() const	{ return &m_p; }

private:
	T* m_p;
};
