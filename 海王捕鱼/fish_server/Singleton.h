#ifndef __ZS_Singleton_h__
#define __ZS_Singleton_h__

//µ¥ÀýÊµÀý
template <typename T>
class Singleton
{
public:
	static T& Instance()
	{
		if (s_pInstance == NULL) s_pInstance = new T;
		return *s_pInstance;
	}
	static T* getInstance()
	{
		if (s_pInstance == NULL) s_pInstance = new T;
		return s_pInstance;
	}

protected:
	Singleton() {}

private:
	Singleton(const Singleton &) {}
	Singleton& operator=(const Singleton &) {}

	// Singleton Helpers
	static void DestroySingleton()
	{
		SafeDelete(s_pInstance);
	}

	// data structure
	static T * s_pInstance;
};

template <typename T>
T * Singleton<T>::s_pInstance = NULL;


#endif // __ZS_Singleton_h__
