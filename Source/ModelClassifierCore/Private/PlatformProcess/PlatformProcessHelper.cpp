#include "PlatformProcess/PlatformProcessHelper.h"

FProcessResult FPlatformProcessHelper::ExecCommand(const FString& Command, const FString& Args)
{
	FProcessResult Result;

	FPlatformProcess::ExecProcess(
		*Command,
		*Args,
		&Result.ReturnCode,
		&Result.StdOut,
		&Result.StdErr
	);

	Result.Output = Result.StdOut + TEXT("\n") + Result.StdErr;
	Result.bIsSuccess = Result.ReturnCode == 0;
	return Result;
}

FProcessResult FPlatformProcessHelper::WaitCommand(const FString& Command, const FString& Args)
{
	void* PipeRead = nullptr;
	void* PipeWrite = nullptr;
	FProcessResult Result;

	FPlatformProcess::CreatePipe(PipeRead, PipeWrite, true);

	FPlatformMisc::SetEnvironmentVar(TEXT("PYTHONUTF8"), TEXT("1"));
	FPlatformMisc::SetEnvironmentVar(TEXT("PYTHONIOENCODING"), TEXT("utf-8"));

	FString FinalArgs = TEXT("-X utf8 ") + Args;

	FProcHandle Proc = FPlatformProcess::CreateProc(
		*Command,
		*FinalArgs,
		true,
		false,
		false,
		nullptr,
		0,
		nullptr,
		PipeWrite
	);
	
	FPlatformProcess::ClosePipe(nullptr, PipeWrite);
	PipeWrite = nullptr;

	if (!Proc.IsValid())
	{
		Result.Output = TEXT("Failed to start Python process.");
		Result.bIsSuccess = false;
		return Result;
	}

	while (FPlatformProcess::IsProcRunning(Proc))
	{
		Result.Output += FPlatformProcess::ReadPipe(PipeRead);
		FPlatformProcess::Sleep(0.01f);
	}

	Result.Output += FPlatformProcess::ReadPipe(PipeRead);

	FPlatformProcess::ClosePipe(PipeRead, nullptr);

	Result.bIsSuccess = true;
	return Result;
}
