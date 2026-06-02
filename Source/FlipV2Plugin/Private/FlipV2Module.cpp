#include "FlipV2Module.h"

#define LOCTEXT_NAMESPACE "FFlipV2Module"

void FFlipV2Module::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FlipV2Plugin module loaded"));
}

void FFlipV2Module::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlipV2Module, FlipV2Plugin)
