#include "FBX/InMemoryFBXStream.h"
#include <fbxsdk.h>

using namespace fbxsdk;
namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FInMemoryFBXStreamImpl : public FbxStream
	{
		FString Name;
		TArray<uint8> Buffer;
		FbxUInt64 Position = 0;
		EState StreamState = eClosed;
		int64 Size;

	public:
		FInMemoryFBXStreamImpl() {}
		virtual ~FInMemoryFBXStreamImpl()
		{
			UE_LOG(LogTemp, Error, TEXT("FInMemoryFBXStreamImpl is destroy!"));
		}
		TArray<uint8> GetBuffer() const { return Buffer; }
		void SetName(const FString& InName) { Name = InName; }
		FString GetName() const { return Name; }

		virtual EState GetState() override { return StreamState; }
		virtual bool Open(void*) override { Position = 0; StreamState = eOpen; Buffer.Reset(); return true; }
		virtual bool Close() override { StreamState = eClosed; return true; }
		virtual bool Flush() override { return true; }

		virtual size_t Write(const void* pData, FbxUInt64 pSize) override
		{
			if (StreamState != eOpen || pSize <= 0 || !pData) return 0;
			if (Position + pSize > Buffer.Num()) {
				Buffer.AddUninitialized(Position + pSize - Buffer.Num());
			}
			FMemory::Memcpy(Buffer.GetData() + Position, pData, pSize);
			Position += pSize;
			return pSize;
		}
		virtual size_t Read(void*, FbxUInt64) const override
		{
			return 0;
		}
		virtual char* ReadString(char* pBuffer, int pMaxSize, bool = false) override {
			if (!pBuffer || pMaxSize <= 0) return nullptr;
			pBuffer[0] = '\0';
			return pBuffer;
		}
		virtual int GetReaderID() const override { return -1; }
		virtual int GetWriterID() const override { return 0; }
		virtual void Seek(const FbxInt64& Offset, const FbxFile::ESeekPos& Pos) override {
			switch (Pos) {
			case FbxFile::eBegin: Position = Offset; break;
			case FbxFile::eCurrent: Position += Offset; break;
			case FbxFile::eEnd: Position = Buffer.Num() + Offset; break;
			}
		}
		virtual int GetError() const override
		{
			return 0;
		}
		virtual void ClearError() override {}
		virtual FbxInt64 GetPosition() const override
		{
			return Position;
		}
		virtual void SetPosition(FbxInt64 pPosition) override
		{
			Position = static_cast<FbxUInt64>(pPosition);
		}
	};

	// -- wrapper functions --
	FInMemoryFBXStream::FInMemoryFBXStream()
	{
		Impl = MakeShareable(new FInMemoryFBXStreamImpl());
		ClassBridge = static_cast<FbxStream*>(Impl.Get());
	}

	bool FInMemoryFBXStream::IsValid()
	{
		return Impl.IsValid();
	}

	TArray<uint8> FInMemoryFBXStream::GetBuffer() const
	{
		return Impl->GetBuffer();
	}

	void FInMemoryFBXStream::SetName(const FString& InName)
	{
		Impl->SetName(InName);
	}

	FString FInMemoryFBXStream::GetName() const
	{
		return Impl->GetName();
	}
}
