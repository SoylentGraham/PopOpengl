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


TPopOpengl::TPopOpengl()
{
	AddJobHandler("exit", TParameterTraits(), *this, &TPopOpengl::OnExit );

	AddJobHandler("maketesttexture", TParameterTraits(), *this, &TPopOpengl::OnMakeTestTexture );

	TParameterTraits MakeWindowTraits;
	MakeWindowTraits.mDefaultParams.PushBack( std::make_tuple(std::string("name"),std::string("gl") ) );
	AddJobHandler("makewindow", MakeWindowTraits, *this, &TPopOpengl::OnMakeWindow );
}

void TPopOpengl::AddChannel(std::shared_ptr<TChannel> Channel)
{
	TChannelManager::AddChannel( Channel );
	if ( !Channel )
		return;
	TJobHandler::BindToChannel( *Channel );
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
	vec2f Size( 300, 200 );
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

	mWindows.PushBack( pWindow );
	TJobReply Reply(Job);
	Reply.mParams.AddDefaultParam("OK");
	if ( !Error.str().empty() )
		Reply.mParams.AddErrorParam( Error.str() );
	
	auto& Channel = JobAndChannel.GetChannel();
	Channel.SendJobReply( Reply );
}



//	horrible global for lambda
std::shared_ptr<TChannel> gStdioChannel;
std::shared_ptr<TChannel> gCaptureChannel;

#include "TOpenglWindow.h"

TPopAppError::Type PopMain(TJobParams& Params)
{
	std::stringstream error;
	new TOpenglWindow("hello", vec2f(10,10), vec2f(200,200), error );
	std::cout << Params << std::endl;
	
	TPopOpengl App;

	auto CommandLineChannel = std::shared_ptr<TChan<TChannelLiteral,TProtocolCli>>( new TChan<TChannelLiteral,TProtocolCli>( SoyRef("cmdline") ) );
	
	//	create stdio channel for commandline output
	gStdioChannel = CreateChannelFromInputString("std:", SoyRef("stdio") );
	auto HttpChannel = CreateChannelFromInputString("http:8080-8090", SoyRef("http") );
	auto WebSocketChannel = CreateChannelFromInputString("ws:json:9090-9099", SoyRef("websock") );
//	auto WebSocketChannel = CreateChannelFromInputString("ws:cli:9090-9099", SoyRef("websock") );
	auto SocksChannel = CreateChannelFromInputString("cli:7090-7099", SoyRef("socks") );
	
	
	App.AddChannel( CommandLineChannel );
	App.AddChannel( gStdioChannel );
	App.AddChannel( HttpChannel );
	App.AddChannel( WebSocketChannel );
	App.AddChannel( SocksChannel );

	//	when the commandline SENDs a command (a reply), send it to stdout
	auto RelayFunc = [](TJobAndChannel& JobAndChannel)
	{
		if ( !gStdioChannel )
			return;
		TJob Job = JobAndChannel;
		Job.mChannelMeta.mChannelRef = gStdioChannel->GetChannelRef();
		Job.mChannelMeta.mClientRef = SoyRef();
		gStdioChannel->SendCommand( Job );
	};
	CommandLineChannel->mOnJobSent.AddListener( RelayFunc );
	
	//	connect to another app, and subscribe to frames
	bool CreateCaptureChannel = false;
	if ( CreateCaptureChannel )
	{
		auto CaptureChannel = CreateChannelFromInputString("cli://localhost:7070", SoyRef("capture") );
		gCaptureChannel = CaptureChannel;
		CaptureChannel->mOnJobRecieved.AddListener( RelayFunc );
		App.AddChannel( CaptureChannel );
		
		//	send commands from stdio to new channel
		auto SendToCaptureFunc = [](TJobAndChannel& JobAndChannel)
		{
			TJob Job = JobAndChannel;
			Job.mChannelMeta.mChannelRef = gStdioChannel->GetChannelRef();
			Job.mChannelMeta.mClientRef = SoyRef();
			gCaptureChannel->SendCommand( Job );
		};
		gStdioChannel->mOnJobRecieved.AddListener( SendToCaptureFunc );
		
		auto StartSubscription = [](TChannel& Channel)
		{
			TJob GetFrameJob;
			GetFrameJob.mChannelMeta.mChannelRef = Channel.GetChannelRef();
			//GetFrameJob.mParams.mCommand = "subscribenewframe";
			//GetFrameJob.mParams.AddParam("serial", "isight" );
			GetFrameJob.mParams.mCommand = "getframe";
			GetFrameJob.mParams.AddParam("serial", "isight" );
			GetFrameJob.mParams.AddParam("memfile", "1" );
			Channel.SendCommand( GetFrameJob );
		};
		
		CaptureChannel->mOnConnected.AddListener( StartSubscription );
	}
	
	
	/*
	std::string TestFilename = "/users/grahamr/Desktop/ringo.png";
	
	//	gr: bootup commands
	auto BootupGet = [TestFilename](TChannel& Channel)
	{
		TJob GetFrameJob;
		GetFrameJob.mChannelMeta.mChannelRef = Channel.GetChannelRef();
		GetFrameJob.mParams.mCommand = "getfeature";
		GetFrameJob.mParams.AddParam("x", 120 );
		GetFrameJob.mParams.AddParam("y", 120 );
		GetFrameJob.mParams.AddParam("image", TestFilename, TJobFormat("text/file/png") );
		Channel.OnJobRecieved( GetFrameJob );
	};
	
	auto BootupMatch = [TestFilename](TChannel& Channel)
	{
		TJob GetFrameJob;
		GetFrameJob.mChannelMeta.mChannelRef = Channel.GetChannelRef();
		GetFrameJob.mParams.mCommand = "findfeature";
		GetFrameJob.mParams.AddParam("feature", "01011000000000001100100100000000" );
		GetFrameJob.mParams.AddParam("image", TestFilename, TJobFormat("text/file/png") );
		Channel.OnJobRecieved( GetFrameJob );
	};
	

	//	auto BootupFunc = BootupMatch;
	//auto BootupFunc = BootupGet;
	auto BootupFunc = BootupMatch;
	if ( CommandLineChannel->IsConnected() )
		BootupFunc( *CommandLineChannel );
	else
		CommandLineChannel->mOnConnected.AddListener( BootupFunc );
*/
	
	
#if !defined(TARGET_OSX)
	//	run
	App.mConsoleApp.WaitForExit();

	gStdioChannel.reset();
#endif
	return TPopAppError::Success;
}




