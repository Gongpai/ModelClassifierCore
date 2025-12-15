#pragma once

enum MODELCLASSIFIERCORE_API SettingType
{
	Button = 0,
	Slider = 1,
	CheckBox = 2,
	DropDown = 3,
	EditableTextBox = 4,
	FileEditableTextBox = 5,
	ObjectPropertyEntryBox = 6,
	ProgressBar = 7
};

template<typename T>
struct MODELCLASSIFIERCORE_API FSettingData
{
private:
	T Value;
	FString ID = "";

public:
	FSettingData() : Value(MakeShareable<T>()),
		Name(""),
		SettingType(Button),
		GetAction(TDelegate<TSharedPtr<void>(void)>()),
		SetAction(TDelegate<void(TSharedPtr<void>)>()),
		Category("Default") {}
	FSettingData(T InValue) : Value(InValue) {}
	FSettingData(T InValue, FString _Name, SettingType _SettingType, TDelegate<TSharedPtr<void>(void)> _GetAction, TDelegate<void(TSharedPtr<void>)> _SetAction, FString _Category = "Default")
		: Value(InValue),
		Name(_Name),
		SettingType(_SettingType),
		GetAction(_GetAction),
		SetAction(_SetAction),
		Category(_Category){}
	
	bool operator==(const FSettingData& Other) const
	{
		return ID == Other.ID && SettingType == Other.SettingType;
	}
	
	T GetValue() const { return Value; }
	void SetValue(T InValue) { Value = InValue; }

public:
	FString Name;
	SettingType SettingType;
	TDelegate<TSharedPtr<void>(void)> GetAction;
	TDelegate<void(TSharedPtr<void>)> SetAction;
	FString Category = "Default";

	FString& GetID();
	
	FString GenerateAlphaNumeric(int32 Length)
	{
		static const char Alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
	
		FString tmp_s;
	
		for (int i = 0; i < Length; ++i) {
			tmp_s += Alphanum[rand() % (sizeof(Alphanum) - 1)];
		}

		UE_LOG(LogTemp, Log, TEXT("AlphaNumeric: %s"), *tmp_s);
		return tmp_s;
	}
};

template <typename T>
FString& FSettingData<T>::GetID()
{
	if (ID.IsEmpty())
	{
		ID = GenerateAlphaNumeric(10);
	}

	return ID;
}

template<typename T>
FORCEINLINE uint32 GetTypeHash(const FSettingData<T>& SettingData)
{
	uint32 Hash = GetTypeHash(SettingData.Name);
	Hash = HashCombine(Hash, GetTypeHash(static_cast<int32>(SettingData.SettingType)));
	return Hash;
}

struct MODELCLASSIFIERCORE_API FEditableTextBoxData
{
	public:
	FEditableTextBoxData() : Text(""),
	HintText(""){}
	FEditableTextBoxData(FString _Text, FString _HintText) : Text(_Text),
	HintText(_HintText) {}
	FEditableTextBoxData(FString _Text, FString _HintText, FString _File) : Text(_Text),
	HintText(_HintText),
	File(_File){}
	
	FString Text;
	FString HintText;
	FString File;
};

struct MODELCLASSIFIERCORE_API FButtonData
{
public:
	FButtonData(){}
	FButtonData(FString _ButtonLabel) : ButtonLabel(_ButtonLabel) {}
	FString ButtonLabel;
};

struct MODELCLASSIFIERCORE_API FDropDownData
{
public:
	FDropDownData(){}
	FDropDownData(int _SelectedOption) : SelectedOption(_SelectedOption) {}
	
	TArray<TSharedPtr<FString>> Options;
	int SelectedOption;
};

struct MODELCLASSIFIERCORE_API FSliderData
{
public:
	FSliderData(){}
	FSliderData(float _Value) : Value(_Value) {}
	
	float Min;
	float Max;
	float Value;
};

struct MODELCLASSIFIERCORE_API FObjectPropertyEntryBoxData
{
public:
	FObjectPropertyEntryBoxData(){}
	FObjectPropertyEntryBoxData(const UClass* _Class) : Class(_Class) {}
	const UClass* Class;
	FAssetData Asset;
};

struct MODELCLASSIFIERCORE_API FProgressBarData
{
public:
	FProgressBarData(){}
	FProgressBarData(FString _Message, float _Max, float _Value) : Message(_Message), Max(_Max), Value(_Value) {}
	FString Message;
	float Max;
	float Value;
};