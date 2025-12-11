#include "FBX/ExportFBX.h"
#include <cmath>
#include "FBX/StaticMeshCache.h"
#include "FBX/InMemoryFBXStream.h"
#include "MeshAttributes.h"
#include "StaticMeshAttributes.h"
#include <fbxsdk.h>
#include <fbxsdk/core/fbxstream.h>
#include "ProgressDebug.h"

using namespace fbxsdk;

namespace ModelClassifierCore
{
	class FExportFBXPrivate : public FExportFBX
	{
	public:
		FExportFBXPrivate(){}
	
		static void ConvertStaticMeshToFbxScene(TSharedPtr<FStaticMeshCache> MObject, TFunction<void(bool bIsValid, TSharedPtr<FInMemoryFBXStream>)> CallBack)
		{
			UE_LOG(LogTemp, Log, TEXT("Run Convert StaticMesh To FbxScene"));
		
			TUniquePtr<FbxManager, FbxManagerDeleter> FbxManager(FbxManager::Create());
			UE_LOG(LogTemp, Log, TEXT("Create FbxManager"));
		
			//Setup
			TSharedPtr<FMeshDescription> MeshDescription = MObject->GetMeshDescription();
			UE_LOG(LogTemp, Log, TEXT("Start writer FBX memory"));
		
			FbxIOSettings* IOSettings = FbxIOSettings::Create(FbxManager.Get(), IOSROOT);
			UE_LOG(LogTemp, Log, TEXT("Create FbxIOSettings"));
		
			FbxManager->SetIOSettings(IOSettings);
			UE_LOG(LogTemp, Log, TEXT("Set FbxIOSettings to FbxManager"));
		
			// Create Scene
			FbxScene* Scene = FbxScene::Create(FbxManager.Get(), "FbxScene");
			UE_LOG(LogTemp, Log, TEXT("Create Scene Complete!"));

			// Create Node
			FbxNode* MeshNode = FbxNode::Create(Scene, "MeshNode");
			UE_LOG(LogTemp, Log, TEXT("Create Node Complete!"));

			// Create Mesh
			FbxMesh* FbxMeshData = FbxMesh::Create(Scene, "FbxMesh");
			UE_LOG(LogTemp, Log, TEXT("Create Mesh Complete!"));

			// ---- Step 1: Convert Positions (ControlPoints) ----
			const int32 VertexCount = MeshDescription->Vertices().Num();
			FbxMeshData->InitControlPoints(VertexCount);
			UE_LOG(LogTemp, Log, TEXT("Init- Convert Positions (ControlPoints) Complete!"));
		
			ControlPoints(MeshDescription, FbxMeshData, MeshNode, Scene);

			UE_LOG(LogTemp, Log, TEXT("Start Convert StaticMesh To FbxScene"));
			FbxExporter* Exporter = FbxExporter::Create(FbxManager.Get(), "");
			TSharedPtr<FInMemoryFBXStream> MemoryStream = MakeShared<FInMemoryFBXStream>();
			MemoryStream->SetName(MObject->GetStaticMesh()->GetName());
			FbxStream* StreamPtr;
			if (MemoryStream->TryGetClassBridge<FbxStream*>(StreamPtr))
			{
				Exporter->SetFileExportVersion(FBX_2020_00_COMPATIBLE);
				if (!Exporter->Initialize(StreamPtr, nullptr, -1, FbxManager->GetIOSettings())) {
					UE_LOG(LogTemp, Error, TEXT("FBX Exporter Init Failed: %s"),
						   UTF8_TO_TCHAR(Exporter->GetStatus().GetErrorString()));

					if (CallBack)
					{
						CallBack(false, MemoryStream);
					}
					return;
				}

				if (!Scene)
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid Scene in Export"));
				
					if (CallBack)
					{
						CallBack(false, MemoryStream);
					}
					return;
				}
			
				Exporter->Export(Scene);
				Exporter->Destroy();

				if (CallBack)
				{
					CallBack(true, MemoryStream);
				}
			}else
			{
				CallBack(false, MemoryStream);
				return;
			}
		}

		static FbxLayerElementUV* CreateUV(FbxMesh* FbxMeshData, const char* pName)
		{
			// UV
			FbxLayerElementUV* LayerUV = FbxLayerElementUV::Create(FbxMeshData, pName);
			LayerUV->SetMappingMode(FbxLayerElement::eByPolygonVertex);
			LayerUV->SetReferenceMode(FbxLayerElement::eDirect);

			return LayerUV;
		}

		static FbxLayerElementNormal* CreateNormal(FbxMesh* FbxMeshData, const char* pName)
		{
			// Normal
			FbxLayerElementNormal* LayerNormal = FbxLayerElementNormal::Create(FbxMeshData, pName);
			LayerNormal->SetMappingMode(FbxLayerElement::eByPolygonVertex);
			LayerNormal->SetReferenceMode(FbxLayerElement::eDirect);

			return LayerNormal;
		}

		static void ControlPoints(TSharedPtr<FMeshDescription> MeshDescription, FbxMesh* FbxMeshData, FbxNode* MeshNode, FbxScene* Scene)
		{
			UE_LOG(LogTemp, Log, TEXT("Run ControlPoints Function"));

			float Progress = 0;
		
			TSharedPtr<TMap<FVertexID, int32>> VertexIDToFbxIndex = MakeShareable<TMap<FVertexID, int32>>(new TMap<FVertexID, int32>());
			
			TVertexAttributesConstRef<FVector3f> Positions =
			MeshDescription->VertexAttributes().GetAttributesRef<FVector3f>(MeshAttribute::Vertex::Position);
			UE_LOG(LogTemp, Log, TEXT("Grt VertexAttributesConstRef Complete!"));
			
			TMap<FVertexID, FVector3f> PositionCopy;
			UE_LOG(LogTemp, Log, TEXT("VertexID Count: %d"), MeshDescription->Vertices().Num());
			
			int32 ControlPointIndex = 0;
			INT16 Step = 0;
			INT16 OldStep = -1;
			
			for (FVertexID VertexID : MeshDescription->Vertices().GetElementIDs())
			{
				PositionCopy.Add(VertexID, Positions[VertexID]);
				FbxVector4 FbxPos(PositionCopy[VertexID].X, PositionCopy[VertexID].Y, PositionCopy[VertexID].Z);

				FbxMeshData->SetControlPointAt(FbxPos, ControlPointIndex);

				VertexIDToFbxIndex->Add(VertexID, ControlPointIndex);
				
				++ControlPointIndex;

				Progress = static_cast<float>(ControlPointIndex) / MeshDescription->Vertices().Num();
				Step = static_cast<int>(std::floor(Progress * 10.0f));
				if (OldStep != Step)
				{
					ProgressDebug::DebugLog(Progress, "SetControlPoint Progress :");
					OldStep = Step;
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Start ControlPoints FbxScene"));
			// ---- Step 2: Create UVs & Normals Layers ----
			FbxLayer* Layer = FbxMeshData->GetLayer(0);
			if (!Layer)
			{
				FbxMeshData->CreateLayer();
				Layer = FbxMeshData->GetLayer(0);
			}
			UE_LOG(LogTemp, Log, TEXT("GetLayer FbxMeshData Complete!"));

			// UV
			FbxLayerElementUV* LayerUV = CreateUV(FbxMeshData, "UVChannel_0");
			UE_LOG(LogTemp, Log, TEXT("Create UV Complete!"));
	
			// Normal
			FbxLayerElementNormal* LayerNormal = CreateNormal(FbxMeshData, "");
			UE_LOG(LogTemp, Log, TEXT("Create Normal Complete!"));
		
			// ---- Step 3: Convert Triangles to Polygons ----
			ConvertTrianglesToPolygons(MeshDescription, FbxMeshData, Layer, MeshNode, Scene, LayerUV, VertexIDToFbxIndex, LayerNormal);
		}

		static void ConvertTrianglesToPolygons(TSharedPtr<FMeshDescription> MeshDescription, FbxMesh* FbxMeshData, FbxLayer* Layer, FbxNode* MeshNode, FbxScene* Scene, FbxLayerElementUV* LayerUV, TSharedPtr<TMap<FVertexID, int32>> VertexIDToFbxIndex, FbxLayerElementNormal* LayerNormal)
		{
			TVertexInstanceAttributesConstRef<FVector3f> Normals =
				MeshDescription->VertexInstanceAttributes().GetAttributesRef<FVector3f>(MeshAttribute::VertexInstance::Normal);
			TVertexInstanceAttributesConstRef<FVector2f> UVs =
				MeshDescription->VertexInstanceAttributes().GetAttributesRef<FVector2f>(MeshAttribute::VertexInstance::TextureCoordinate);

			float Progress = 0;
			INT16 Step = 0;
			INT16 OldStep = -1;

			UE_LOG(LogTemp, Log, TEXT("VertexID Count: %d"), MeshDescription->Vertices().Num());
			
			for (const FTriangleID TriangleID : MeshDescription->Triangles().GetElementIDs())
			{
				Progress = static_cast<float>(TriangleID) / (MeshDescription->Triangles().Num() - 1.0f);

				Step =  static_cast<int>(std::floor(Progress * 10.0f));
				if (OldStep != Step)
				{
					ProgressDebug::DebugLog(Progress, "Convert Triangles To Polygons Progress :");
					OldStep = Step;
				}
				
				FbxMeshData->BeginPolygon();

				for (int32 Corner = 0; Corner < 3; ++Corner)
				{
					FVertexInstanceID InstanceID = MeshDescription->GetTriangleVertexInstance(TriangleID, Corner);
					FVertexID VertexID = MeshDescription->GetVertexInstanceVertex(InstanceID);

					const int32* FbxIndexPtr = VertexIDToFbxIndex->Find(VertexID);
					if (!FbxIndexPtr)
					{
						UE_LOG(LogTemp, Error, TEXT("VertexID not found in VertexIDToFbxIndex"));
						continue;
					}
					FbxMeshData->AddPolygon(*FbxIndexPtr);

					// UV
					FVector2f UV = UVs[InstanceID];
					LayerUV->GetDirectArray().Add(FbxVector2(UV.X, 1.0f - UV.Y));

					// Normal
					FVector3f Normal = Normals[InstanceID].GetSafeNormal();
					LayerNormal->GetDirectArray().Add(FbxVector4(Normal.X, Normal.Y, Normal.Z));
				}

				FbxMeshData->EndPolygon();
			}

			UE_LOG(LogTemp, Log, TEXT("Start Convert Triangles To Polygons"));
				
			// UV
			Layer->SetUVs(LayerUV, FbxLayerElement::eTextureDiffuse);
			UE_LOG(LogTemp, Log, TEXT("Layer SetUVs Complete!"));

			// Normal
			Layer->SetNormals(LayerNormal);
			UE_LOG(LogTemp, Log, TEXT("Layer SetNormals Complete!"));
			
			// ---- Step 4: Add Mesh to Scene ----
			MeshNode->SetNodeAttribute(FbxMeshData);
			UE_LOG(LogTemp, Log, TEXT("Add Mesh to Scene Complete!"));
				
			Scene->GetRootNode()->AddChild(MeshNode);
			UE_LOG(LogTemp, Log, TEXT("Add Child To RootNode Complete!"));
		}

		static FbxFileTexture* CreateTexture(FbxManager* Manager, const char* pFilename)
		{
			FbxFileTexture* Texture = FbxFileTexture::Create(Manager, pFilename);

			Texture->SetName(pFilename);
			Texture->SetTextureUse(FbxTexture::eStandard);
			Texture->SetMappingType(FbxTexture::eUV);
			Texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
			Texture->SetSwapUV(false);
			Texture->SetTranslation(0.0, 0.0);
			Texture->SetScale(1.0, 1.0);
			Texture->SetRotation(0.0, 0.0);

			return Texture;
		}
		
		struct FbxManagerDeleter
		{
			void operator()(FbxManager* Manager) const
			{
				if (Manager) Manager->Destroy();
			}
		};
	};

	void FExportFBX::Writer(TSharedPtr<FStaticMeshCache> MObject, TFunction<void(bool, TSharedPtr<FInMemoryFBXStream>)> CallBack)
	{
		Async(EAsyncExecution::Thread, [MObject, CallBack](){
			FExportFBXPrivate::ConvertStaticMeshToFbxScene(MObject, [CallBack](bool bIsValid, TSharedPtr<FInMemoryFBXStream> MemoryStream)
			{
				AsyncTask(ENamedThreads::GameThread, [CallBack, bIsValid, MemoryStream]()
				{
					if (CallBack)
					{
						CallBack(bIsValid, MemoryStream);
					}
				});
			});
		});
	}

	void FExportFBX::SaveFBXToFile(TSharedPtr<FInMemoryFBXStream> InMemoryStream, FString SavePath)
	{
		if (!InMemoryStream.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("MemoryStream is invalid"));
			return;
		}

		if (!SavePath.EndsWith(TEXT(".fbx"), ESearchCase::IgnoreCase))
		{
			SavePath += TEXT(".fbx");
		}
	
		TArray<uint8> FBXBytes = InMemoryStream->GetBuffer();
		if (FFileHelper::SaveArrayToFile(FBXBytes, *SavePath))
		{
			UE_LOG(LogTemp, Log, TEXT("FBX saved to: %s"), *SavePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save FBX to: %s"), *SavePath);
		}
	}
}