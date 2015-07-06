#include "PopOpengl.h"
#include <TParameters.h>
#include <SoyDebug.h>
#include <TProtocolCli.h>
#include <TProtocolHttp.h>
#include <SoyApp.h>
#include <PopMain.h>
#include <TJobRelay.h>
#include <SoyPixels.h>
#include <SoyString.h>
#include <TFeatureBinRing.h>
#include <SortArray.h>
#include <TChannelLiteral.h>
#include "TTextureWindow.h"
#include <TChannelFile.h>


void Soy::TOpenglDevice::MakeTestTexture(SoyPixels& Pixels,std::stringstream& Error)
{
	Pixels.Init( 256, 256, SoyPixelsFormat::RGB );
	
	Error << "todo";
}



/*

SoyContextLock::SoyContextLock(std::shared_ptr<Soy::TScopeCall> Scope) :
	mScope	( Scope )
{
}

SoyContextLock::SoyContextLock(std::mutex& Mutex)
{
	auto LockFunc = [&Mutex]
	{
		Mutex.lock();
	};
	auto UnlockFunc = [&Mutex]
	{
		Mutex.unlock();
	};
	mScope.reset( new Soy::TScopeCall( LockFunc, UnlockFunc ) );
}
*/


TPopOpengl::TPopOpengl() :
	TJobHandler		( static_cast<TChannelManager&>(*this) ),
	TPopJobHandler	( static_cast<TJobHandler&>(*this) )
{
	AddJobHandler("exit", TParameterTraits(), *this, &TPopOpengl::OnExit );

	AddJobHandler("maketesttexture", TParameterTraits(), *this, &TPopOpengl::OnMakeTestTexture );

	TParameterTraits MakeWindowTraits;
	MakeWindowTraits.mDefaultParams.PushBack( std::make_tuple(std::string("name"),std::string("gl") ) );
	AddJobHandler("makewindow", MakeWindowTraits, *this, &TPopOpengl::OnMakeWindow );
}

bool TPopOpengl::AddChannel(std::shared_ptr<TChannel> Channel)
{
	if ( !TChannelManager::AddChannel( Channel ) )
		return false;
	if ( !Channel )
		return false;
	TJobHandler::BindToChannel( *Channel );
	return true;
}


void TPopOpengl::OnExit(TJobAndChannel& JobAndChannel)
{
	mConsoleApp.Exit();
	
	//	should probably still send a reply
	TJobReply Reply( JobAndChannel );
	Reply.mParams.AddDefaultParam(std::string("exiting..."));
	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}


void TPopOpengl::OnMakeTestTexture(TJobAndChannel& JobAndChannel)
{
	std::stringstream Error;
	SoyPixels TestTexture;
	mOpengl.MakeTestTexture( TestTexture, Error );
	
	TJobReply Reply( JobAndChannel );

	if ( !Error.str().empty() )
		Reply.mParams.AddErrorParam( Error.str() );
	
	Reply.mParams.AddDefaultParam( TestTexture );
	
	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}

void TPopOpengl::OnMakeWindow(TJobAndChannel &JobAndChannel)
{
	auto& Job = JobAndChannel.GetJob();
	auto Name = Job.mParams.GetParamAs<std::string>("name");

	vec2f Pos( 100,100 );
	vec2f Size( 400, 400 );
	std::stringstream Error;
	std::shared_ptr<TTextureWindow> pWindow( new TTextureWindow(Name,Pos,Size,Error) );
	if ( !pWindow->IsValid() )
	{
		TJobReply Reply(Job);
		Reply.mParams.AddErrorParam( Error.str() );
		auto& Channel = JobAndChannel.GetChannel();
		Channel.SendJobReply( Reply );
		return;
	}
	
	SoyPixels TestTexture;
	TestTexture.Init( 256, 256, SoyPixelsFormat::RGB );
	BufferArray<char,3> Rgb;
	Rgb.PushBack( 255 );
	Rgb.PushBack( 0 );
	Rgb.PushBack( 0 );
	TestTexture.SetColour( GetArrayBridge(Rgb) );
	pWindow->SetTexture( TestTexture );

	mWindows.PushBack( pWindow );
	TJobReply Reply(Job);
	Reply.mParams.AddDefaultParam("OK");
	if ( !Error.str().empty() )
		Reply.mParams.AddErrorParam( Error.str() );
	
	auto& Channel = JobAndChannel.GetChannel();
	Channel.SendJobReply( Reply );
}



//	keep alive after PopMain()
#if defined(TARGET_OSX_BUNDLE)
std::shared_ptr<TPopOpengl> gOpenglApp;
#endif


TPopAppError::Type PopMain(TJobParams& Params)
{
	std::cout << Params << std::endl;
	
	gOpenglApp.reset( new TPopOpengl );
	auto& App = *gOpenglApp;

	auto CommandLineChannel = std::shared_ptr<TChan<TChannelLiteral,TProtocolCli>>( new TChan<TChannelLiteral,TProtocolCli>( SoyRef("cmdline") ) );
	
	//	create stdio channel for commandline output
	auto StdioChannel = CreateChannelFromInputString("std:", SoyRef("stdio") );
	auto HttpChannel = CreateChannelFromInputString("http:8080-8090", SoyRef("http") );
	
	
	App.AddChannel( CommandLineChannel );
	App.AddChannel( StdioChannel );
	App.AddChannel( HttpChannel );

	
	
	
	
	//	bootup commands via a channel
	std::shared_ptr<TChannel> BootupChannel( new TChan<TChannelFileRead,TProtocolCli>( SoyRef("Bootup"), "bootup.txt" ) );
	/*
	//	display reply to stdout
	//	when the commandline SENDs a command (a reply), send it to stdout
	auto RelayFunc = [](TJobAndChannel& JobAndChannel)
	{
		std::Debug << JobAndChannel.GetJob().mParams << std::endl;
	};
	//BootupChannel->mOnJobRecieved.AddListener( RelayFunc );
	BootupChannel->mOnJobSent.AddListener( RelayFunc );
	BootupChannel->mOnJobLost.AddListener( RelayFunc );
	*/
	App.AddChannel( BootupChannel );
	


#if !defined(TARGET_OSX_BUNDLE)
	//	run
	App.mConsoleApp.WaitForExit();
#endif

	return TPopAppError::Success;
}




