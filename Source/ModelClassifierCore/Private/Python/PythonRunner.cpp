#include "Python/PythonRunner.h"

#include "PlatformProcess/PlatformProcessHelper.h"

namespace ModelClassifierCore
{
	FPythonRunner::FPythonRunner(ModelClassifierCore::FModelClassifierCoreModule *InCoreModule) : Core(InCoreModule)
	{
		Core = InCoreModule;
		
		if (InCoreModule->GetConfig()->PythonOutputPath.IsEmpty())
		{
			InCoreModule->GetConfig()->PythonOutputPath = GetDefaultPythonWorkingDirectory();
		}
		if (InCoreModule->GetConfig()->PythonENV.IsEmpty())
		{
			InCoreModule->GetConfig()->PythonENV = GetDefaultPythonWorkingDirectory();
		}
		
		if (CreatePythonEnv() && !InCoreModule->GetConfig()->bIsSkipInstallPackages)
		{
			InstallPackages();
		}
	}

	FPythonRunner::~FPythonRunner()
	{
	}

	FString FPythonRunner::GetDefaultPythonWorkingDirectory()
	{
		FString SaveModelPath = FPaths::Combine(Core->GetPluginPath(), "Source", "Python");

		if (!FPaths::FileExists(SaveModelPath))
		{
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			bool bIsCreate = PlatformFile.CreateDirectory(*SaveModelPath);

			if (!bIsCreate)
			{
				UE_LOG(LogTemp, Error, TEXT("Create Directory Error."))
			}
		}
		
		return SaveModelPath;
	}
	
	bool FPythonRunner::EnvExists(const FString& EnvPath)
	{
		const FString PythonExe = EnvPath / TEXT("Scripts/python.exe");
		return FPaths::FileExists(PythonExe);
	}
	
	bool FPythonRunner::EnvUsable(const FString& EnvPath)
	{
		const FString PythonExe = EnvPath / TEXT("Scripts/python.exe");

		if (!FPaths::FileExists(PythonExe))
			return false;
		
		FProcessResult Result = FPlatformProcessHelper::ExecCommand(*PythonExe, TEXT("--version"));
		
		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}
	
	bool FPythonRunner::DeleteEnv(const FString& EnvPath)
	{
		return IFileManager::Get().DeleteDirectory(*EnvPath, false, true);
	}
	
	bool FPythonRunner::CreateEnv(const FString& PythonExe, const FString& EnvPath)
	{
		FString Args = FString::Printf(TEXT("-m venv \"%s\""), *EnvPath);

		FProcessResult Result = FPlatformProcessHelper::ExecCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}
	
	bool FPythonRunner::EnsurePythonEnv(const FString& PythonExe, const FString& EnvPath)
	{
		if (!EnvExists(FPaths::Combine(EnvPath, "Scripts/python.exe")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Environment not found. Creating new env..."));
			return CreateEnv(PythonExe, EnvPath);
		}

		if (EnvUsable(EnvPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Environment already exists and is working."));
			return true;
		}

		UE_LOG(LogTemp, Error, TEXT("Environment exists but is broken. Recreating..."));

		DeleteEnv(EnvPath);

		return CreateEnv(PythonExe, EnvPath);
	}


	bool FPythonRunner::CreatePythonEnv()
	{
		FString PythonExe = Core->GetConfig()->PythonExe;
		FString EnvPath = Core->GetConfig()->PythonENV;

		if (EnsurePythonEnv(PythonExe, EnvPath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Python environment is ready!"));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Python environment."));
			return false;
		}
	}
	
	void FPythonRunner::InstallPackages()
	{
		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		const TArray<FString> Packages = Core->GetConfig()->Packages;
		const TArray<FString> ExternalPackages = Core->GetConfig()->ExternalPackages;
		
		FString Update_pip_Args = FString::Printf(TEXT("-m pip install --upgrade pip"));
		FProcessResult Result_pip = FPlatformProcessHelper::ExecCommand(*PythonExe, *Update_pip_Args);
		
		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result_pip.ReturnCode, *Result_pip.Output);
		if (Result_pip.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result_pip.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result_pip.StdErr);
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Start install packages"));
		for (const FString& Pkg : Packages)
		{
			FString PackageArgs = FString::Printf(TEXT("-m pip install %s --quiet"), *Pkg);

			FProcessResult ProcessResult = FPlatformProcessHelper::ExecCommand(*PythonExe, *PackageArgs);
			if (ProcessResult.bIsSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[OK] Installed %s"), *Pkg);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[FAILED] %s\n%s"), *Pkg, *ProcessResult.Output);
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("Start install external packages"));
		for (const FString& Pkg : ExternalPackages)
		{
			FString PackageArgs = FString::Printf(TEXT("-m pip install git+%s"), *Pkg);

			FProcessResult ProcessResult = FPlatformProcessHelper::ExecCommand(*PythonExe, *PackageArgs);
			if (ProcessResult.bIsSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[OK] Installed %s"), *Pkg);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[FAILED] %s\n%s"), *Pkg, *ProcessResult.Output);
			}
		}
	}

	bool FPythonRunner::RunDownloadAndConvertModelToONNXModel()
	{
		FString ModelName = Core->GetConfig()->ModelName;
		FString SavePath = Core->GetConfig()->PythonOutputPath;
		int32 InputX = Core->GetConfig()->InputResolution.X;
		int32 InputY = Core->GetConfig()->InputResolution.Y;
		
		FString PythonScript = FString::Printf(TEXT(
		"import torch\n"
		"import numpy as np\n"
		"from transformers import CLIPModel, CLIPProcessor\n"
		"\n"
		"model_name = \"%s\"\n" //<-- Set Model Name (S1)
		"model = CLIPModel.from_pretrained(model_name)\n"
		"processor = CLIPProcessor.from_pretrained(model_name, use_fast=False)\n"
		"model.eval().to(\"cpu\")\n"
		"\n"
		"class ImageWrapper(torch.nn.Module):\n"
		"    def __init__(self, clip):\n"
		"        super().__init__()\n"
		"        self.clip = clip\n"
		"    def forward(self, pixel_values):\n"
		"        return self.clip.get_image_features(pixel_values)\n"
		"\n"
		"image_wrapper = ImageWrapper(model)\n"
		"dummy_image = torch.randn(1, 3, %d, %d, dtype=torch.float32)\n"
		"torch.onnx.export(\n"
		"    image_wrapper,\n"
		"    dummy_image,\n"
		"    \"%s/clip_image_features(%dx%d).onnx\",\n" //<-- Set Save Image ONNX Model to Path (S2)
		"    input_names=[\"pixel_values\"],\n"
		"    output_names=[\"image_features\"],\n"
		"    dynamic_axes={},\n"
		"    opset_version=17,\n"
		"    do_constant_folding=True,\n"
		")\n"
		"\n"
		"class TextWrapper(torch.nn.Module):\n"
		"    def __init__(self, clip):\n"
		"        super().__init__()\n"
		"        self.clip = clip\n"
		"    def forward(self, input_ids):\n"
		"        return self.clip.get_text_features(input_ids=input_ids)\n"
		"\n"
		"text_wrapper = TextWrapper(model)\n"
		"dummy_text = torch.randint(0, processor.tokenizer.vocab_size, (1, 77), dtype=torch.long)\n"
		"torch.onnx.export(\n"
		"    text_wrapper,\n"
		"    dummy_text,\n"
		"    \"%s/clip_text_features(%dx%d).onnx\",\n" //<-- Set Save Text ONNX Model to Path (S3)
		"    input_names=[\"input_ids\"],\n"
		"    output_names=[\"text_features\"],\n"
		"    dynamic_axes={},\n"
		"    opset_version=17,\n"
		"    do_constant_folding=True,\n"
		")\n"
		"\n"
		"labels = [\"Apple\",\"Banana\",\"Orange\"]\n"
		"inputs = processor(text=labels, padding=True, return_tensors=\"pt\")\n"
		"with torch.no_grad():\n"
		"    text_feats = model.get_text_features(**{k: v for k, v in inputs.items() if k == \"input_ids\" or k == \"attention_mask\"})\n"
		"    text_feats = text_feats / text_feats.norm(dim=-1, keepdim=True)\n"
		"    text_feats = text_feats.cpu().numpy().astype(np.float32)\n"
		"    np.save(\"text_features.npy\", text_feats)\n"
		"    logit_scale = model.logit_scale.exp().cpu().item()\n"
		"    np.save(\"logit_scale.npy\", np.array([logit_scale], dtype=np.float32))\n"
		"print(\"Exported ONNX & saved text_features.npy, logit_scale.npy\")\n"
		), *ModelName, InputX, InputY, *SavePath, InputX, InputY, *SavePath, InputX, InputY);
		
		FString GeneratedPyPath = Core->GetConfig()->PythonENV / TEXT("GeneratedClipExport.py");
		FFileHelper::SaveStringToFile(PythonScript, *GeneratedPyPath);

		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		FString Args = FString::Printf(TEXT("\"%s\""), *GeneratedPyPath);

		FProcessResult Result = FPlatformProcessHelper::WaitCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}

	bool FPythonRunner::RunDownloadTokenizer()
	{
		FString ModelName = Core->GetConfig()->ModelName;
		FString SavePath = Core->GetConfig()->PythonOutputPath;
		
		FString PythonScript = FString::Printf(TEXT(
		"from transformers import CLIPTokenizer\n"
		"tokenizer = CLIPTokenizer.from_pretrained(\"%s\")\n"
		"tokenizer.save_pretrained(\"%s\")\n"
		), *ModelName, *SavePath);
		
		FString GeneratedPyPath = Core->GetConfig()->PythonENV / TEXT("GeneratedCLIPTokenizerDownload.py");
		FFileHelper::SaveStringToFile(PythonScript, *GeneratedPyPath);

		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		FString Args = FString::Printf(TEXT("\"%s\""), *GeneratedPyPath);

		FProcessResult Result = FPlatformProcessHelper::WaitCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}

	bool FPythonRunner::RunGenerateTokenization(UTextFeaturesAsset* TextFeaturesAsset, FString& OutJsonPath)
	{
		FString ModelName = Core->GetConfig()->ModelName;
		FString SavePath = FPaths::Combine(Core->GetConfig()->PythonOutputPath, TEXT("clip_tokens.json"));
		FString Prompt;
		TArray<FString> Templates = Core->GetConfig()->PromptTemplates;
		
		for (auto Label : TextFeaturesAsset->Labels)
		{
			Prompt += FString::Printf(TEXT("        \"%s\": [\n"), *Label);
			for (const FString& Template : Templates)
			{
				FString CLIPLabel = Template.Replace(TEXT("{}"), *Label);
				Prompt += FString::Printf(TEXT("            \"%s\",\n"), *CLIPLabel);
			}
			Prompt += FString::Printf(TEXT("        ],\n"));
		}
		
		FString PythonScript = FString::Printf(TEXT(
		"from transformers import CLIPTokenizer\n"
		"import json\n"
		"import sys\n"
		"import os\n"
		"\n"
		"try:\n"
		"    print('Loading tokenizer...')\n"
		"    tokenizer = CLIPTokenizer.from_pretrained(\"%s\")\n"
		"\n"
		"    prompt_map = {\n"
		), *ModelName);
		PythonScript += Prompt;
		PythonScript += FString::Printf(TEXT(
		"    }\n"
		"\n"
		"    token_map = {}\n"
		"    total = sum(len(v) for v in prompt_map.values())\n"
		"    processed = 0\n"
		"\n"
		"    print(f'Tokenizing {total} texts...')\n"
		"\n"
		"    for label, prompts in prompt_map.items():\n"
		"        token_map[label] = {}\n"
		"        for prompt in prompts:\n"
		"            tokens = tokenizer(\n"
		"                prompt,\n"
		"                return_tensors=\"pt\",\n"
		"                padding=\"max_length\",\n"
		"                max_length=77,\n"
		"                truncation=True\n"
		"            )\n"
		"            token_map[label][prompt] = tokens.input_ids[0].tolist()\n"
		"\n"
		"            processed += 1\n"
		"            if processed %% 10 == 0:\n"
		"                print(f'Progress: {processed}/{total}')\n"
		"\n"
		"    output_path = \"%s\"\n"
		"    output_dir = os.path.dirname(output_path)\n"
		"    if output_dir and not os.path.exists(output_dir):\n"
		"        os.makedirs(output_dir)\n"
		"\n"
		"    print('Saving tokens...')\n"
		"    with open(output_path, \"w\", encoding=\"utf-8\") as f:\n"
		"        json.dump(token_map, f, indent=2, ensure_ascii=False)\n"
		"\n"
		"    print(f'Successfully saved tokenized data to: {output_path}')\n"
		"    sys.exit(0)\n"
		"\n"
		"except Exception as e:\n"
		"    print(f'Error: {str(e)}', file=sys.stderr)\n"
		"    import traceback\n"
		"    traceback.print_exc()\n"
		"    sys.exit(1)\n"
		), *SavePath);
		
		FString GeneratedPyPath = Core->GetConfig()->PythonENV / TEXT("GeneratedTokenization.py");
		FFileHelper::SaveStringToFile(PythonScript, *GeneratedPyPath);

		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		FString Args = FString::Printf(TEXT("\"%s\""), *GeneratedPyPath);

		FProcessResult Result = FPlatformProcessHelper::WaitCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("Tokenization Output: %s"), *Result.StdOut);
        
			if (FPaths::FileExists(SavePath))
			{
				UE_LOG(LogTemp, Log, TEXT("Token file created successfully at: %s"), *SavePath);
				OutJsonPath = SavePath;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Token file not found at expected path: %s"), *SavePath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Tokenization Error: %s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}

	bool FPythonRunner::RunGetTokenLength(FString OnnxPath, FString& OutTokenLengthPath)
	{
		FString SavePath = Core->GetConfig()->PythonOutputPath / TEXT("model_shape.txt");
		
		FString PythonScript = FString::Printf(TEXT(
			"import onnx\n"
			"\n"
			"model = onnx.load(r\"%s\")\n"
			"\n"
			"seq_len = None\n"
			"feat_dim = None\n"
			"\n"
			"for inp in model.graph.input:\n"
			"    dims = inp.type.tensor_type.shape.dim\n"
			"    if len(dims) == 2 and dims[1].HasField('dim_value'):\n"
			"        seq_len = dims[1].dim_value\n"
			"\n"
			"for out in model.graph.output:\n"
			"    dims = out.type.tensor_type.shape.dim\n"
			"    if len(dims) == 2 and dims[1].HasField('dim_value'):\n"
			"        feat_dim = dims[1].dim_value\n"
			"\n"
			"with open(r\"%s\", 'w') as f:\n"
			"    f.write(str(seq_len) + '\\n')\n"
			"    f.write(str(feat_dim) + '\\n')\n"
			"\n"
			"print('SeqLen:', seq_len)\n"
			"print('FeatureDim:', feat_dim)\n"
		), *OnnxPath, *SavePath);
		
		FString GeneratedPyPath = Core->GetConfig()->PythonENV / TEXT("GeneratedReadCLIPShape.py");
		FFileHelper::SaveStringToFile(PythonScript, *GeneratedPyPath);

		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		FString Args = FString::Printf(TEXT("\"%s\""), *GeneratedPyPath);

		FProcessResult Result = FPlatformProcessHelper::WaitCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			OutTokenLengthPath = SavePath;
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}

	bool FPythonRunner::RunExportCLIPImageNormalization(const FString ModelName, FString& OutSavePath)
	{
		FString SavePath = Core->GetConfig()->PythonOutputPath / TEXT("clip_image_norm.json");

		FString PythonScript = FString::Printf(TEXT(
			"import json\n"
			"import clip\n"
			"\n"
			"model_name = \"%s\"\n"
			"output_path = r\"%s\"\n"
			"\n"
			"_, preprocess = clip.load(model_name)\n"
			"normalize = preprocess.transforms[-1]\n"
			"\n"
			"data = {\n"
			"    \"mean\": list(normalize.mean),\n"
			"    \"std\": list(normalize.std)\n"
			"}\n"
			"\n"
			"with open(output_path, \"w\") as f:\n"
			"    json.dump(data, f, indent=4)\n"
			"\n"
			"print(f\"CLIP image normalization saved to: {output_path}\")\n"
		), *ModelName, *SavePath);
		
		FString GeneratedPyPath = Core->GetConfig()->PythonENV / TEXT("GeneratedCLIPImageNormExport.py");
		FFileHelper::SaveStringToFile(PythonScript, *GeneratedPyPath);

		const FString PythonExe = Core->GetConfig()->PythonENV / TEXT("Scripts/python.exe");
		FString Args = FString::Printf(TEXT("\"%s\""), *GeneratedPyPath);

		FProcessResult Result = FPlatformProcessHelper::WaitCommand(*PythonExe, *Args);

		UE_LOG(LogTemp, Log, TEXT("Return Code : %d | %s"), Result.ReturnCode, *Result.Output);
		if (Result.bIsSuccess)
		{
			OutSavePath = SavePath;
			UE_LOG(LogTemp, Log, TEXT("%s"), *Result.StdOut);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *Result.StdErr);
		}
		
		return Result.bIsSuccess;
	}
}
