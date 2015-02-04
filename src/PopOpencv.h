#pragma once
#include <ofxSoylent.h>
#include <SoyApp.h>
#include <TJob.h>
#include <TChannel.h>




//	opengl
namespace Soy
{
	class TGraphicsDevice;	//	replace TUnityDevice
	class TOpenglDevice;
};

//	make this more copyable (avoid the ref pointer)
//	and maybe a little more multipurpose
//	basically for overloading "do I have a context, and am I in the right thread for it" for safety
class SoyContextLock
{
public:
	explicit SoyContextLock()	{}		//	invalid
	explicit SoyContextLock(std::shared_ptr<Soy::TScopeCall> Scope);
	explicit SoyContextLock(std::mutex& Mutex);
	
	bool		IsValid() const	{	return mScope != nullptr;	}
	operator	bool() const	{	return IsValid();	}

private:
	std::shared_ptr<Soy::TScopeCall>	mScope;	//	unlocks when last ContextLock is released
};


class Soy::TGraphicsDevice
{
public:
	virtual SoyContextLock		GetContext()
	{
		//	generic, free lock which does nothing
		std::shared_ptr<Soy::TScopeCall> Scope( new Soy::TScopeCall(nullptr,nullptr) );
		return SoyContextLock( Scope );
	}
};

class Soy::TOpenglDevice : public TGraphicsDevice
{
public:
	bool		Test();
};

bool Soy::TOpenglDevice::Test()
{
	auto Context = GetContext();
	if ( !Context )
	{
		std::Debug << "failed to grab opengl context" << std::endl;
		return false;
	}
	
	return true;
}



class TPopOpencv : public TJobHandler, public TChannelManager
{
public:
	TPopOpencv();
	
	virtual void	AddChannel(std::shared_ptr<TChannel> Channel) override;

	void			OnExit(TJobAndChannel& JobAndChannel);
	
public:
	Soy::Platform::TConsoleApp	mConsoleApp;
};



