#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.h"
#include "UnLive2DAnimBase.h"
#include "UnLive2DExpression.generated.h"

UCLASS(Blueprintable, BlueprintType, Category = UnLive2DAnim, hidecategories = Object)
class UNLIVE2DASSET_API UUnLive2DExpression : public UUnLive2DAnimBase
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	bool LoadLive2DExpressionData(const FString& ReadExpressionPath);

	void SetLive2DExpressionData(FUnLiveByteData& Data);

protected:
	virtual bool GetAnimParamterGroup(TWeakObjectPtr<class UUnLive2DRendererComponent>& ObsComp, TArray<struct FUnLive2DParameterData_Expression>& ParameterArr) override;

	virtual void SetAnimParamterValue(FName ParameterStr, float NewParameter) override;

	virtual void SetAnimParamterBlendType(FName ParameterStr, float NewParameter) override;

#endif

public:
	// 淡入时间
	UPROPERTY(Category = Developer, AssetRegistrySearchable, EditAnywhere)
		float FadeInTime;

	// 淡出时间
	UPROPERTY(Category = Developer, AssetRegistrySearchable, EditAnywhere)
		float FadeOutTime;

public:

	const FUnLiveByteData* GetExpressionData() const;

protected:
	// 表情数据
	UPROPERTY()
		FUnLiveByteData ExpressionData;

#if WITH_EDITOR
	TSharedPtr<FJsonObject> ExpressionDataJsonRoot;
#endif
};